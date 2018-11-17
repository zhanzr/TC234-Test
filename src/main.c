#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include <machine/intrinsics.h>
#include <machine/wdtcon.h>

#include "system_tc2x.h"
#include "interrupts.h"
#include "led.h"
#include "uart_int.h"
#include "cint.h"
#include "asm_prototype.h"
#include "bspconfig.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)
#include TC_INCLUDE(TCPATH/IfxEth_reg.h)
#include TC_INCLUDE(TCPATH/IfxEth_bf.h)
#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)
#include TC_INCLUDE(TCPATH/IfxInt_reg.h)
#include TC_INCLUDE(TCPATH/IfxInt_bf.h)
#include TC_INCLUDE(TCPATH/IfxSrc_reg.h)
#include TC_INCLUDE(TCPATH/IfxSrc_bf.h)

#define SYS_TICK_HZ	1000
#define STM_CMP0_ISR_PRIO	10
#define STM_CMP1_ISR_PRIO	11
#define GPSR0_ISR_PRIO	12
#define GPSR1_ISR_PRIO	13
#define GPSR2_ISR_PRIO	14
#define GPSR3_ISR_PRIO	15
#define DTS_ISR_PRIO	16

#define ERS0_ISR_PRIO	17
#define ERS1_ISR_PRIO	18
#define ERS2_ISR_PRIO	19
#define ERS3_ISR_PRIO	20

volatile uint32_t g_sys_ticks;
volatile uint32_t g_cmp1_ticks;

volatile bool g_gpsr_flag[4];
volatile bool g_dts_flag;
volatile bool g_ers_flag[4];

static void gpsr0_isr(uint32_t var)
{
	SRC_GPSR00.B.SRR = 0;
	g_gpsr_flag[0] = true;
}

static void gpsr1_isr(uint32_t var)
{
	SRC_GPSR01.B.SRR = 0;
	g_gpsr_flag[1] = true;
}

static void gpsr2_isr(uint32_t var)
{
	SRC_GPSR02.B.SRR = 0;
	g_gpsr_flag[2] = true;
}

static void gpsr3_isr(uint32_t var)
{
	SRC_GPSR03.B.SRR = 0;
	g_gpsr_flag[3] = true;
}

static void ers0_isr(uint32_t var)
{
	MODULE_SCU.FMR.B.FC0 = 1;
	g_ers_flag[0] = true;
}

static void ers1_isr(uint32_t var)
{
	MODULE_SCU.FMR.B.FC1 = 1;
	g_ers_flag[1] = true;
}

static void ers2_isr(uint32_t var)
{
	MODULE_SCU.FMR.B.FC2 = 1;
	g_ers_flag[2] = true;
}

static void ers3_isr(uint32_t var)
{
	MODULE_SCU.FMR.B.FC3 = 1;
	g_ers_flag[3] = true;
}

const uint32_t HAL_GetTick(void)
{
	return g_sys_ticks;
}

const uint32_t HAL_GetRunTimeTick(void)
{
	return g_cmp1_ticks;
}

/* timer interrupt routine */
static void stm_cmp0_isr(uint32_t reload_value)
{
	MODULE_STM0.ISCR.B.CMP1IRR = 1;
	MODULE_STM0.CMP[0].U += (uint32_t)reload_value;

	++g_sys_ticks;
}

static void stm_cmp1_isr(uint32_t reload_value)
{
	MODULE_STM0.ISCR.B.CMP0IRR = 1;
	MODULE_STM0.CMP[1].U += (uint32_t)reload_value;

	++g_cmp1_ticks;
}

/* Initialise timer at rate <hz> */
void stm_init(uint8_t ch, uint32_t hz)
{
	uint32_t freq_stm = SYSTEM_GetStmClock();

	uint32_t reload_value = freq_stm / hz;

	if(0==ch)
	{
		InterruptInstall(SRC_ID_STM0SR0, stm_cmp0_isr, STM_CMP0_ISR_PRIO, reload_value);

		/* Determine how many bits are used without changing other bits in the CMCON register. */
		MODULE_STM0.CMCON.B.MSIZE0 &= ~0x1fUL;
		MODULE_STM0.CMCON.B.MSIZE0 |= (0x1f - __builtin_clz( reload_value));

		/* reset interrupt flag */
		MODULE_STM0.ISCR.B.CMP0IRR = 1;
		/* prepare compare register */
		MODULE_STM0.CMP[0].U = MODULE_STM0.TIM0.U + reload_value;

		MODULE_STM0.ICR.B.CMP0OS = 0;
		MODULE_STM0.ICR.B.CMP0EN = 1;
	}
	else
	{
		InterruptInstall(SRC_ID_STM0SR1, stm_cmp1_isr, STM_CMP1_ISR_PRIO, reload_value);

		/* Determine how many bits are used without changing other bits in the CMCON register. */
		MODULE_STM0.CMCON.B.MSIZE1 &= ~0x1fUL;
		MODULE_STM0.CMCON.B.MSIZE1 |= (0x1f - __builtin_clz( reload_value));

		/* reset interrupt flag */
		MODULE_STM0.ISCR.B.CMP1IRR = 1;
		/* prepare compare register */
		MODULE_STM0.CMP[1].U = MODULE_STM0.TIM0.U + reload_value;

		MODULE_STM0.ICR.B.CMP1OS = 1;
		MODULE_STM0.ICR.B.CMP1EN = 1;
	}
}

static inline void flush_stdout(void)
{
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
	{
		;
	}
}

static void prvTrapYield( int iTrapIdentification )
{
	switch( iTrapIdentification )
	{
	default:
		printf("Syscall Tin:%d\n", iTrapIdentification);
		break;
	}
}

void enable_performance_cnt(void)
{
	unlock_wdtcon();
	{
		Ifx_CPU_CCTRL tmpCCTRL;
		tmpCCTRL.U = _mfcr(CPU_CCTRL);

		//Instruction Cache Hit Count
		//		tmpCCTRL.bits.M1 = 2;
		//Data Cache Hit Count.
		tmpCCTRL.B.M1 = 3;
		//Instruction Cache Miss Count

		//		tmpCCTRL.bits.M2 = 1;
		//Data Cache Clean Miss Count
		tmpCCTRL.B.M2 = 3;

		//Data Cache Dirty Miss Count
		tmpCCTRL.B.M3 = 3;

		//Normal Mode
		tmpCCTRL.B.CM = 0;
		//Task Mode
		//		tmpCCTRL.bits.CM = 1;

		tmpCCTRL.B.CE = 1;

		_mtcr(CPU_CCTRL, tmpCCTRL.U);
	}
	lock_wdtcon();
}

typedef struct _Hnd_arg
{
	void (*hnd_handler)(int);
	int hnd_arg;
} Hnd_arg;

void config_gpsr(void)
{
	SRC_GPSR00.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR00, gpsr0_isr, GPSR0_ISR_PRIO, 0);

	SRC_GPSR01.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR01, gpsr1_isr, GPSR1_ISR_PRIO, 1);

	SRC_GPSR02.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR02, gpsr2_isr, GPSR2_ISR_PRIO, 2);

	SRC_GPSR03.B.SRR = 0;
	InterruptInstall(SRC_ID_GPSR03, gpsr3_isr, GPSR3_ISR_PRIO, 3);
}

bool IfxDts_isReady(void)
{
    return SCU_DTSSTAT.B.RDY == 1 ? true : false;
}

static void dts_isr(uint32_t para)
{
	g_dts_flag = true;
}

static void config_dts(void)
{
//	printf("SCU_DTSCON\t%08X\t:%08X\n", &SCU_DTSCON, SCU_DTSCON);
//	printf("SCU_DTSSTAT\t%08X\t:%08X\n", &SCU_DTSSTAT, SCU_DTSSTAT);
//	printf("SCU_DTSLIM\t%08X\t:%08X\n", &SCU_DTSLIM, SCU_DTSLIM);
//	flush_stdout();

	SCU_DTSCON.B.PWD = 0;
	SCU_DTSCON.B.START = 1;

	if(IfxDts_isReady())
	{
	    /* one dummy read to a SCU register which ensures that the BUSY flag is synchronized
	     * into the status register before IfxDts_Dts_isBusy() is called */
		_nop();
	}

	InterruptInstall(SRC_ID_SCUDTS, dts_isr, DTS_ISR_PRIO, 0);
}

static inline int16_t read_dts(void)
{
	return SCU_DTSSTAT.B.RESULT;
}

static inline float read_dts_celsius(void)
{
	int16_t raw = read_dts();
    return (raw - 607)/2.13;
}

static inline void start_dts_measure(void)
{
	SCU_DTSCON.B.START = 1;
}

static void config_eru(void)
{
	//ERS 0, 2, 3, 6, 7
//	ERS0:
//		P15.4		->X102.P33
//
//	ERS1:
//		P14.3
//
//	ERS2:
//		P10.2
//		P02.1		->X103.P14
//		P00.4		->X103.P26
//
//	ERS3:
//		P10.3
//		P14.1
//		P02.0		->X103.P13
//
//	ERS4:
//		P33.7
//		P15.5
//
//	ERS5:
//		P15.8
//
//	ERS6:
//		P20.0
//		P33.11		-> X102.P26
//		P11.10
//
//	ERS7:
//		P20.9		-> X102.P37
//		P15.1

	//ERS Channel 0
	MODULE_P15.IOCR4.B.PC4 = IN_PULLUP;

	MODULE_SCU.EICR[0].B.EXIS0 = 0;
	MODULE_SCU.EICR[0].B.FEN0 = 1;
	MODULE_SCU.EICR[0].B.REN0 = 0;
	MODULE_SCU.EICR[0].B.LDEN0 = 0;
	MODULE_SCU.EICR[0].B.EIEN0 = 1;
	MODULE_SCU.EICR[0].B.INP0 = 0;

	MODULE_SCU.IGCR[0].B.IGP0 = 2;
	MODULE_SCU.IGCR[0].B.GEEN0 = 1;

	//ERS2 Channel 1
	MODULE_P02.IOCR0.B.PC1 = IN_PULLUP;

	MODULE_SCU.EICR[1].B.EXIS0 = 1;
	MODULE_SCU.EICR[1].B.FEN0 = 1;
	MODULE_SCU.EICR[1].B.REN0 = 0;
	MODULE_SCU.EICR[1].B.LDEN0 = 0;
	MODULE_SCU.EICR[1].B.EIEN0 = 1;
	MODULE_SCU.EICR[1].B.INP0 = 0;

	MODULE_SCU.IGCR[1].B.IGP0 = 2;
	MODULE_SCU.IGCR[1].B.GEEN0 = 1;

	//ERS3 Channel 2
	MODULE_P02.IOCR0.B.PC0 = IN_PULLUP;

	MODULE_SCU.EICR[1].B.EXIS1 = 2;
	MODULE_SCU.EICR[1].B.FEN1 = 1;
	MODULE_SCU.EICR[1].B.REN1 = 0;
	MODULE_SCU.EICR[1].B.LDEN1 = 0;
	MODULE_SCU.EICR[1].B.EIEN1 = 1;
	MODULE_SCU.EICR[1].B.INP1 = 0;

	MODULE_SCU.IGCR[1].B.IGP1 = 2;
	MODULE_SCU.IGCR[1].B.GEEN1 = 1;

	//ERS6 Channel 2
	MODULE_P33.IOCR8.B.PC11 = IN_PULLUP;

	MODULE_SCU.EICR[3].B.EXIS0 = 2;
	MODULE_SCU.EICR[3].B.FEN0 = 1;
	MODULE_SCU.EICR[3].B.REN0 = 0;
	MODULE_SCU.EICR[3].B.LDEN0 = 0;
	MODULE_SCU.EICR[3].B.EIEN0 = 1;
	MODULE_SCU.EICR[3].B.INP0 = 0;

	MODULE_SCU.IGCR[3].B.IGP0 = 2;
	MODULE_SCU.IGCR[3].B.GEEN0 = 1;

	//ERS7 Channel 0
	MODULE_P20.IOCR8.B.PC9 = IN_PULLUP;

	MODULE_SCU.EICR[3].B.EXIS1 = 0;
	MODULE_SCU.EICR[3].B.FEN1 = 1;
	MODULE_SCU.EICR[3].B.REN1 = 0;
	MODULE_SCU.EICR[3].B.LDEN1 = 0;
	MODULE_SCU.EICR[3].B.EIEN1 = 1;
	MODULE_SCU.EICR[3].B.INP1 = 0;

	MODULE_SCU.IGCR[3].B.IGP1 = 2;
	MODULE_SCU.IGCR[3].B.GEEN1 = 1;

	InterruptInstall(SRC_ID_SCUERU0, ers0_isr, ERS0_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_SCUERU1, ers1_isr, ERS1_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_SCUERU2, ers2_isr, ERS2_ISR_PRIO, 0);
	InterruptInstall(SRC_ID_SCUERU3, ers3_isr, ERS3_ISR_PRIO, 0);
}

int main(void)
{
	volatile bool g_regular_task_flag;

	SYSTEM_Init();
	SYSTEM_EnaDisCache(1);

	/* initialise STM CMP 0 at SYS_TICK_HZ rate */
	stm_init(0, SYS_TICK_HZ);
	stm_init(1, 10*SYS_TICK_HZ);

//	config_gpsr();
	config_dts();
	config_eru();

	_init_uart(BAUDRATE);
	led_init();

	enable_performance_cnt();

	printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d\n",
			__TRICORE_NAME__,
			__TRICORE_CORE__,
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			SYSTEM_IsCacheEnabled());
	flush_stdout();

	/* Install the Syscall Handler for yield calls. */
	extern void (*Tdisptab[MAX_TRAPS])(int tin);
	Tdisptab[6] = prvTrapYield;

	printf("\nTest ERU\n");

	printf("CPUID\t%08X\t:%08X\n", CPU_CPU_ID, _mfcr(CPU_CPU_ID));
//	printf("CCTRL\t%08X\t:%08X\n", CPU_CCTRL, _mfcr(CPU_CCTRL));
//	printf("CCNT\t%08X\t:%08X\n", CPU_CCNT, _mfcr(CPU_CCNT));
//	printf("ICNT\t%08X\t:%08X\n", CPU_ICNT, _mfcr(CPU_ICNT));
//	printf("M1CNT\t%08X\t:%08X\n", CPU_M1CNT, _mfcr(CPU_M1CNT));
//	printf("M2CNT\t%08X\t:%08X\n", CPU_M2CNT, _mfcr(CPU_M2CNT));
//	printf("M3CNT\t%08X\t:%08X\n", CPU_M3CNT, _mfcr(CPU_M3CNT));

	//	printf("ETH_ID\t%08X\t:%08X\n", &ETH_ID, ETH_ID);
	printf("SCU_ID\t%08X\t:%08X\n", &SCU_ID, SCU_ID);
	printf("SCU_MANID\t%08X\t:%08X\n", &SCU_MANID, SCU_MANID);
	printf("SCU_CHIPID\t%08X\t:%08X\n", &SCU_CHIPID, SCU_CHIPID);
	Ifx_SYSCALL(0);
	Ifx_SYSCALL(1);

	printf("INT_ID\t%08X\t:%08X\n", &INT_ID, INT_ID);
	printf("INT_SRB0\t%08X\t:%08X\n", &INT_SRB0, INT_SRB0);
	printf("SRC_GPSR00\t%08X\t:%08X\n", &SRC_GPSR00, SRC_GPSR00);
	printf("SRC_GPSR01\t%08X\t:%08X\n", &SRC_GPSR01, SRC_GPSR01);
	printf("SRC_GPSR02\t%08X\t:%08X\n", &SRC_GPSR02, SRC_GPSR02);
	printf("SRC_GPSR03\t%08X\t:%08X\n", &SRC_GPSR03, SRC_GPSR03);
	flush_stdout();

	extern void _start(void);
	printf("_start\t:%08X\n", (uint32_t)_start);

	extern void __USTACK_BEGIN(void);
	printf("__USTACK_BEGIN\t:%08X\n", (uint32_t)__USTACK_BEGIN);
	extern void __USTACK(void);
	printf("__USTACK\t:%08X\n", (uint32_t)__USTACK);
	extern void __USTACK_SIZE(void);
	printf("__USTACK_SIZE\t:%08X\n", (uint32_t)__USTACK_SIZE);
	extern void __USTACK_END(void);
	printf("__USTACK_END\t:%08X\n", (uint32_t)__USTACK_END);
	flush_stdout();

	extern void __ISTACK_BEGIN(void);
	printf("__ISTACK_BEGIN\t:%08X\n", (uint32_t)__ISTACK_BEGIN);
	extern void __ISTACK(void);
	printf("__ISTACK\t:%08X\n", (uint32_t)__ISTACK);
	extern void __ISTACK_SIZE(void);
	printf("__ISTACK_SIZE\t:%08X\n", (uint32_t)__ISTACK_SIZE);
	extern void __ISTACK_END(void);
	printf("__ISTACK_END\t:%08X\n", (uint32_t)__ISTACK_END);
	flush_stdout();

	extern void __HEAP_BEGIN(void);
	printf("__HEAP_BEGIN\t:%08X\n", (uint32_t)__HEAP_BEGIN);
	extern void __HEAP(void);
	printf("__HEAP\t:%08X\n", (uint32_t)__HEAP);
	extern void __HEAP_SIZE(void);
	printf("__HEAP_SIZE\t:%08X\n", (uint32_t)__HEAP_SIZE);
	extern void __HEAP_END(void);
	printf("__HEAP_END\t:%08X\n", (uint32_t)__HEAP_END);
	flush_stdout();

	extern void __CSA_BEGIN(void);
	printf("__CSA_BEGIN\t:%08X\n", (uint32_t)__CSA_BEGIN);
	extern void __CSA(void);
	printf("__CSA\t:%08X\n", (uint32_t)__CSA);
	extern void __CSA_SIZE(void);
	printf("__CSA_SIZE\t:%08X\n", (uint32_t)__CSA_SIZE);
	extern void __CSA_END(void);
	printf("__CSA_END\t:%08X\n", (uint32_t)__CSA_END);
	flush_stdout();

	extern void _SMALL_DATA_(void);
	printf("_SMALL_DATA_\t:%08X\n", (uint32_t)_SMALL_DATA_);
	extern void _SMALL_DATA2_(void);
	printf("_SMALL_DATA2_\t:%08X\n", (uint32_t)_SMALL_DATA2_);

	extern void first_trap_table(void);
	printf("first_trap_table\t:%08X\n", (uint32_t)first_trap_table);

	printf("BIV\t%08X\t:%08X\n", CPU_BIV, _mfcr(CPU_BIV));
	extern void TriCore_int_table(void);
	printf("TriCore_int_table\t:%08X\n", (uint32_t)TriCore_int_table);
	flush_stdout();

	extern void __interrupt_1(void);
	printf("__interrupt_1\t:%08X\n", (uint32_t)__interrupt_1);
	extern void ___interrupt_1(void);
	printf("___interrupt_1\t:%08X\n", (uint32_t)___interrupt_1);
	extern void __interrupt_2(void);
	printf("__interrupt_2\t:%08X\n", (uint32_t)__interrupt_2);
	flush_stdout();

	extern Hnd_arg Cdisptab[MAX_INTRS];
	printf("Soft Interrupt vector table %08X:%u * %u = %u\n",
			(uint32_t)Cdisptab,
			sizeof(Cdisptab[0]),
			MAX_INTRS,
			sizeof(Cdisptab));
	flush_stdout();

	printf("BTV\t%08X\t:%08X\n", CPU_BTV, _mfcr(CPU_BTV));
	extern void TriCore_trap_table(void);
	printf("TriCore_trap_table\t:%08X\n", (uint32_t)TriCore_trap_table);

	extern void __trap_0(void);
	printf("__trap_0\t:%08X\n", (uint32_t)__trap_0);
	extern void __trap_1(void);
	printf("__trap_1\t:%08X\n", (uint32_t)__trap_1);
	extern void __trap_6(void);
	printf("__trap_6\t:%08X\n", (uint32_t)__trap_6);
	extern void ___trap_6(void);
	printf("___trap_6\t:%08X\n", (uint32_t)___trap_6);
	flush_stdout();

	extern void (*Tdisptab[MAX_TRAPS]) (int tin);
	printf("Soft Trap vector table %08X:%u * %u = %u\n",
			(uint32_t)Tdisptab,
			sizeof(Tdisptab[0]),
			MAX_TRAPS,
			sizeof(Tdisptab));
	flush_stdout();

	g_regular_task_flag = true;

	uint8_t test_trig_cnt = 0;
	while(1)
	{
		if(0==g_sys_ticks%(SYS_TICK_HZ))
		{
			g_regular_task_flag = true;
		}

		uint32_t test_trigger_cnt = g_sys_ticks/(SYS_TICK_HZ);

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

			if(0==test_trigger_cnt%4)
			{
				led_on(0);
				led_off(1);
				led_off(2);
			}
			else if(1==test_trigger_cnt%4)
			{
				led_off(0);
				led_on(1);
				led_off(2);
			}
			else if(2==test_trigger_cnt%4)
			{
				led_off(0);
				led_off(1);
				led_on(2);
			}
			else
			{
				led_toggle(3);
			}

			printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d\n",
					__TRICORE_NAME__,
					__TRICORE_CORE__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					SYSTEM_GetStmClock()/1000000,
					SYSTEM_IsCacheEnabled());
			flush_stdout();

			start_dts_measure();

//			printf("EIFR:%08X P15_IN:%08X\n",
//					MODULE_SCU.EIFR.U, MODULE_P15.IN.U);
//			flush_stdout();

//			printf("SCU_DTSCON\t%08X\t:%08X\n", &SCU_DTSCON, SCU_DTSCON);
//			printf("SCU_DTSSTAT\t%08X\t:%08X\n", &SCU_DTSSTAT, SCU_DTSSTAT);
//			printf("SCU_DTSLIM\t%08X\t:%08X\n", &SCU_DTSLIM, SCU_DTSLIM);
//			flush_stdout();

//			printf("STMID:%08X\n",
//					MODULE_STM0.ID.U);
//
//			printf("%u\n", HAL_GetTick());
//			printf("%u\n", HAL_GetRunTimeTick());
//			flush_stdout();
//
//			printf("%08X %08X %08X %08X %08X %08X\n",
//					MODULE_STM0.TIM0.U,
//					MODULE_STM0.TIM1.U,
//					MODULE_STM0.TIM2.U,
//					MODULE_STM0.TIM3.U,
//					MODULE_STM0.TIM4.U,
//					MODULE_STM0.TIM5.U
//			);
//			flush_stdout();

//			switch(test_trig_cnt)
//			{
//			case 0:
//				INT_SRB0.B.TRIG0 = 1;
////				SRC_GPSR00.B.SETR = 1;
//				break;
//
//			case 1:
//				INT_SRB0.B.TRIG1 = 1;
////				SRC_GPSR01.B.SETR = 1;
//				break;
//
//			case 2:
////				INT_SRB0.B.TRIG2 = 1;
//				SRC_GPSR02.B.SETR = 1;
//				break;
//
//			case 3:
////				INT_SRB0.B.TRIG3 = 1;
//				SRC_GPSR03.B.SETR = 1;
//				break;
//
//			default:
//				break;
//			}
//			test_trig_cnt = (test_trig_cnt+1)%4;

		}

		if(g_dts_flag)
		{
			g_dts_flag = false;
			printf("DTS Triggered %.3f P15.4:%u\n",
					read_dts_celsius(),
					MODULE_P15.IN.B.P4);
			flush_stdout();
		}

		if(g_ers_flag[0])
		{
			g_ers_flag[0] = false;
//			printf("External Interrupt Channel 0 Triggered\n");
			flush_stdout();
		}

		if(g_ers_flag[1])
		{
			g_ers_flag[1] = false;
//			printf("External Interrupt Channel 1 Triggered\n");
			flush_stdout();
		}

		if(g_ers_flag[2])
		{
			g_ers_flag[2] = false;
//			printf("External Interrupt Channel 2 Triggered\n");
			flush_stdout();
		}

		if(g_ers_flag[3])
		{
			g_ers_flag[3] = false;
//			printf("External Interrupt Channel 3 Triggered\n");
			flush_stdout();
		}

//		if(g_gpsr_flag[0])
//		{
//			g_gpsr_flag[0] = false;
//			printf("GPSR 0 Triggered\n");
//			flush_stdout();
//		}
//
//		if(g_gpsr_flag[1])
//		{
//			g_gpsr_flag[1] = false;
//			printf("GPSR 1 Triggered\n");
//			flush_stdout();
//		}
//
//		if(g_gpsr_flag[2])
//		{
//			g_gpsr_flag[2] = false;
//			printf("GPSR 2 Triggered\n");
//			flush_stdout();
//		}
//
//		if(g_gpsr_flag[3])
//		{
//			g_gpsr_flag[3] = false;
//			printf("GPSR 3 Triggered\n");
//			flush_stdout();
//		}


	}

	return EXIT_SUCCESS;
}

