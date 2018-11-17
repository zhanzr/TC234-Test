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

#include "bspconfig_tc23x.h"

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

#include "core_tc23x.h"
#include "system_tc2x.h"
#include "cint_trap_tc23x.h"
#include "interrupts_tc23x.h"
#include "led.h"
#include "uart_int.h"
#include "asm_prototype.h"
#include "dts.h"
#include "timer.h"
#include "gpsr.h"
#include "scu_eru.h"
#include "data_flash.h"

static void prvTrapYield( int tin )
{
	switch( tin )
	{
	default:
		printf("Trap6 Tin:%d\n", tin);
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


int main(void)
{
	volatile bool g_regular_task_flag;

	SYSTEM_Init();
	SYSTEM_EnaDisCache(1);

	/* initialise STM CMP 0 at SYS_TICK_HZ rate */
	stm_init(0, SYS_TICK_HZ);
	stm_init(1, 10*SYS_TICK_HZ);

	_init_uart(BAUDRATE);
	led_init();

	config_dts();
//	config_gpsr();
//	config_eru();
	config_dflash();

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

	printf("\nTest data flash\n");
	flush_stdout();


	extern void __PMI_PSPR_BEGIN(void);
	extern void __PMI_PSPR_SIZE(void);
	printf("PSRR:%08X %08X\n", (uint32_t)__PMI_PSPR_BEGIN, (uint32_t)__PMI_PSPR_SIZE);
	for(uint32_t i=0; i<0x20/4; ++i)
	{
		printf("%08X ", *((uint32_t*)((uint32_t)__PMI_PSPR_BEGIN+i*4)));
	}
	printf("\n");
	flush_stdout();

	//Read UCB
//#define	UCB_ADDR	(0xAF100000)
//#define	UCB_SIZE	(0x4000)
//	printf("UCB:%08X %08X\n", (uint32_t)UCB_ADDR, (uint32_t)UCB_SIZE);
//	for(uint32_t i=0; i<UCB_SIZE/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)((uint32_t)UCB_ADDR+i*4)));
//	}
//	printf("\n");
//	flush_stdout();

//	printf("BMHD0:%08X\n", BMHD_ADDR_0);
//	flush_stdout();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_0+i*4)));
//		flush_stdout();
//	}

//	unlock_wdtcon();
//	uint32_t tmpU32 = *((uint32_t*)(BMHD_ADDR_1));
//	lock_wdtcon();
//	printf("%08X\n", tmpU32);
//	flush_stdout();
//
//	printf("BMHD1:%08X\n", BMHD_ADDR_1);
//	flush_stdout();
//
//	unlock_wdtcon();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_1+i*4)));
//	}
//	lock_wdtcon();
//	flush_stdout();

//	printf("BMHD2:%08X\n", BMHD_ADDR_2);
//	flush_stdout();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_2+i*4)));
//		flush_stdout();
//	}
//
//	printf("BMHD3:%08X\n", BMHD_ADDR_3);
//	flush_stdout();
//	for(uint32_t i=0; i<0x20/4; ++i)
//	{
//		printf("%08X ", *((uint32_t*)(BMHD_ADDR_3+i*4)));
//		flush_stdout();
//	}

//	unlock_wdtcon();
//	printf("BMHD1, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
//			(uint32_t)P_BMHD_1->startAddress,
//			P_BMHD_1->bmIndex,
//			P_BMHD_1->bmhdID,
//			P_BMHD_1->chkStart,
//			P_BMHD_1->chkEnd,
//			P_BMHD_1->crcRange,
//			P_BMHD_1->invCrcRange,
//			P_BMHD_1->crcHead,
//			P_BMHD_1->invCrcHead);
//	flush_stdout();
//	lock_wdtcon();

//	printf("BMHD2, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
//			(uint32_t)P_BMHD_2->startAddress,
//			P_BMHD_2->bmIndex,
//			P_BMHD_2->bmhdID,
//			P_BMHD_2->chkStart,
//			P_BMHD_2->chkEnd,
//			P_BMHD_2->crcRange,
//			P_BMHD_2->invCrcRange,
//			P_BMHD_2->crcHead,
//			P_BMHD_2->invCrcHead);
//	flush_stdout();
//
//	printf("P_BMHD_3, STADABM:%08X, BMI:%04X, BMHDID:%04X, CHKSTART:%08X, CHKEND:%08X, CRCRANGE:%08X, nCRCRANGE:%08X, CRCHEAD:%08X, nCRCHEAD:%08X\n",
//			(uint32_t)P_BMHD_3->startAddress,
//			P_BMHD_3->bmIndex,
//			P_BMHD_3->bmhdID,
//			P_BMHD_3->chkStart,
//			P_BMHD_3->chkEnd,
//			P_BMHD_3->crcRange,
//			P_BMHD_3->invCrcRange,
//			P_BMHD_3->crcHead,
//			P_BMHD_3->invCrcHead);
//	flush_stdout();

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

	_syscall(0);
	_syscall(1);

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
#define	DELAY_TICK	(10*SYS_TICK_HZ)
		if(0==HAL_GetTick()%(DELAY_TICK))
		{
			g_regular_task_flag = true;
		}

		uint32_t test_trigger_cnt = HAL_GetTick()/(DELAY_TICK);

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

			printf("%u\n", HAL_GetTick());
			printf("%u\n", HAL_GetRunTimeTick());
			flush_stdout();

			//			printf("%08X %08X %08X %08X %08X %08X\n",
			//					MODULE_STM0.TIM0.U,
			//					MODULE_STM0.TIM1.U,
			//					MODULE_STM0.TIM2.U,
			//					MODULE_STM0.TIM3.U,
			//					MODULE_STM0.TIM4.U,
			//					MODULE_STM0.TIM5.U
			//			);
			//			flush_stdout();

			test_trig_cnt = test_trigger_gpsr(test_trig_cnt);

		}

		test_proc_dts();

		test_proc_eru();

//		test_proc_gpsr();

	}

	return EXIT_SUCCESS;
}

