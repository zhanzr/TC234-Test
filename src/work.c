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

#include "tc23xa/IfxStm_reg.h"
#include "tc23xa/IfxStm_bf.h"
#include "tc23xa/IfxCpu_reg.h"
#include "tc23xa/IfxCpu_bf.h"
#include "tc23xa/IfxEth_reg.h"
#include "tc23xa/IfxEth_bf.h"
#include "tc23xa/IfxScu_reg.h"
#include "tc23xa/IfxScu_bf.h"

#include "system_tc2x.h"
#include "interrupts.h"
#include "led.h"
#include "uart_int.h"
#include "cint.h"

#include "asm_prototype.h"

#define SYSTIME_CLOCK	1000	/* timer event rate [Hz] */

volatile uint32_t g_sys_ticks;
volatile bool g_regular_task_flag;

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_sys_ticks;

	if(0==g_sys_ticks%(2*SYSTIME_CLOCK))
	{
		LEDTOGGLE(1);
	}

}

const uint32_t HAL_GetTick(void)
{
	return g_sys_ticks;
}

#define SYSTIME_ISR_PRIO	2

#define STM0_BASE			((Ifx_STM *)&MODULE_STM0)

/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value;

/* pointer to user specified timer callback function */
static TCF user_handler = (TCF)0;

static __inline Ifx_STM *systime_GetStmBase(void)
{
	switch (_mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK)
	{
	case 0 :
	default :
		return STM0_BASE;
		break;
	}
}

/* timer interrupt routine */
static void tick_irq(int reload_value)
{
	Ifx_STM *StmBase = systime_GetStmBase();

	/* set new compare value */
	StmBase->CMP[0].U += (unsigned int)reload_value;
	if (user_handler)
	{
		user_handler();
	}
}

/* Initialise timer at rate <hz> */
void TimerInit(unsigned int hz)
{
	unsigned int CoreID = _mfcr(CPU_CORE_ID) & IFX_CPU_CORE_ID_CORE_ID_MSK;
	Ifx_STM *StmBase = systime_GetStmBase();
	unsigned int frequency = SYSTEM_GetStmClock();
	int irqId;

	reload_value = frequency / hz;

	switch (CoreID)
	{
	default :
	case 0 :
		irqId = SRC_ID_STM0SR0;
		break;
	}

	/* install handler for timer interrupt */
	InterruptInstall(irqId, tick_irq, SYSTIME_ISR_PRIO, (int)reload_value);

	/* reset interrupt flag */
	StmBase->ISCR.U = (IFX_STM_ISCR_CMP0IRR_MSK << IFX_STM_ISCR_CMP0IRR_OFF);
	/* prepare compare register */
	StmBase->CMP[0].U = StmBase->TIM0.U + reload_value;
	StmBase->CMCON.U  = 31;
	StmBase->ICR.B.CMP0EN = 1;
}

/* Install <handler> as timer callback function */
void TimerSetHandler(TCF handler)
{
	user_handler = handler;
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

int main(void)
{
	SYSTEM_Init();
	SYSTEM_EnaDisCache(1);

	/* initialise timer at SYSTIME_CLOCK rate */
	TimerInit(SYSTIME_CLOCK);
	/* add own handler for timer interrupts */
	TimerSetHandler(my_timer_handler);

	_init_uart(BAUDRATE);
	InitLED();

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

	printf("\nTest Fake ETH\n");

	printf("CPUID\t%08X\t:%08X\n\n", CPU_CPU_ID, _mfcr(CPU_CPU_ID));
	printf("CCTRL\t%08X\t:%08X\n\n", CPU_CCTRL, _mfcr(CPU_CCTRL));
	printf("CCNT\t%08X\t:%08X\n\n", CPU_CCNT, _mfcr(CPU_CCNT));
	printf("ICNT\t%08X\t:%08X\n\n", CPU_ICNT, _mfcr(CPU_ICNT));
	printf("M1CNT\t%08X\t:%08X\n\n", CPU_M1CNT, _mfcr(CPU_M1CNT));
	printf("M2CNT\t%08X\t:%08X\n\n", CPU_M2CNT, _mfcr(CPU_M2CNT));
	printf("M3CNT\t%08X\t:%08X\n\n", CPU_M3CNT, _mfcr(CPU_M3CNT));

	//	printf("ETH_ID\t%08X\t:%08X\n\n", &ETH_ID, ETH_ID);
	printf("SCU_ID\t%08X\t:%08X\n\n", &SCU_ID, SCU_ID);
	printf("SCU_MANID\t%08X\t:%08X\n\n", &SCU_MANID, SCU_MANID);
	printf("SCU_CHIPID\t%08X\t:%08X\n\n", &SCU_CHIPID, SCU_CHIPID);
	Ifx_SYSCALL(0);
	Ifx_SYSCALL(1);

	extern void _start(void);
	printf("_start\t:%08X\n\n", (uint32_t)_start);

	extern void __USTACK_BEGIN(void);
	printf("__USTACK_BEGIN\t:%08X\n\n", (uint32_t)__USTACK_BEGIN);
	extern void __USTACK(void);
	printf("__USTACK\t:%08X\n\n", (uint32_t)__USTACK);
	extern void __USTACK_SIZE(void);
	printf("__USTACK_SIZE\t:%08X\n\n", (uint32_t)__USTACK_SIZE);
	extern void __USTACK_END(void);
	printf("__USTACK_END\t:%08X\n\n", (uint32_t)__USTACK_END);

	extern void __ISTACK_BEGIN(void);
	printf("__ISTACK_BEGIN\t:%08X\n\n", (uint32_t)__ISTACK_BEGIN);
	extern void __ISTACK(void);
	printf("__ISTACK\t:%08X\n\n", (uint32_t)__ISTACK);
	extern void __ISTACK_SIZE(void);
	printf("__ISTACK_SIZE\t:%08X\n\n", (uint32_t)__ISTACK_SIZE);
	extern void __ISTACK_END(void);
	printf("__ISTACK_END\t:%08X\n\n", (uint32_t)__ISTACK_END);

	extern void __HEAP_BEGIN(void);
	printf("__HEAP_BEGIN\t:%08X\n\n", (uint32_t)__HEAP_BEGIN);
	extern void __HEAP(void);
	printf("__HEAP\t:%08X\n\n", (uint32_t)__HEAP);
	extern void __HEAP_SIZE(void);
	printf("__HEAP_SIZE\t:%08X\n\n", (uint32_t)__HEAP_SIZE);
	extern void __HEAP_END(void);
	printf("__HEAP_END\t:%08X\n\n", (uint32_t)__HEAP_END);

	extern void __CSA_BEGIN(void);
	printf("__CSA_BEGIN\t:%08X\n\n", (uint32_t)__CSA_BEGIN);
	extern void __CSA(void);
	printf("__CSA\t:%08X\n\n", (uint32_t)__CSA);
	extern void __CSA_SIZE(void);
	printf("__CSA_SIZE\t:%08X\n\n", (uint32_t)__CSA_SIZE);
	extern void __CSA_END(void);
	printf("__CSA_END\t:%08X\n\n", (uint32_t)__CSA_END);

	extern void _SMALL_DATA_(void);
	printf("_SMALL_DATA_\t:%08X\n\n", (uint32_t)_SMALL_DATA_);
	extern void _SMALL_DATA2_(void);
	printf("_SMALL_DATA2_\t:%08X\n\n", (uint32_t)_SMALL_DATA2_);

	extern void first_trap_table(void);
	printf("first_trap_table\t:%08X\n\n", (uint32_t)first_trap_table);

	printf("BIV\t%08X\t:%08X\n\n", CPU_BIV, _mfcr(CPU_BIV));
	extern void TriCore_int_table(void);
	printf("TriCore_int_table\t:%08X\n\n", (uint32_t)TriCore_int_table);

	extern void __interrupt_1(void);
	printf("__interrupt_1\t:%08X\n\n", (uint32_t)__interrupt_1);
	extern void ___interrupt_1(void);
	printf("___interrupt_1\t:%08X\n\n", (uint32_t)___interrupt_1);
	extern void __interrupt_2(void);
	printf("__interrupt_2\t:%08X\n\n", (uint32_t)__interrupt_2);

	extern Hnd_arg Cdisptab[MAX_INTRS];
	printf("Soft Interrupt vector table %08X:%u * %u = %u\n",
			(uint32_t)Cdisptab,
			sizeof(Cdisptab[0]),
			MAX_INTRS,
			sizeof(Cdisptab));

	printf("BTV\t%08X\t:%08X\n\n", CPU_BTV, _mfcr(CPU_BTV));
	extern void TriCore_trap_table(void);
	printf("TriCore_trap_table\t:%08X\n\n", (uint32_t)TriCore_trap_table);

	extern void __trap_0(void);
	printf("__trap_0\t:%08X\n\n", (uint32_t)__trap_0);
	extern void __trap_1(void);
	printf("__trap_1\t:%08X\n\n", (uint32_t)__trap_1);
	extern void __trap_6(void);
	printf("__trap_6\t:%08X\n\n", (uint32_t)__trap_6);
	extern void ___trap_6(void);
	printf("___trap_6\t:%08X\n\n", (uint32_t)___trap_6);

	extern void (*Tdisptab[MAX_TRAPS]) (int tin);
	printf("Soft Trap vector table %08X:%u * %u = %u\n",
			(uint32_t)Tdisptab,
			sizeof(Tdisptab[0]),
			MAX_TRAPS,
			sizeof(Tdisptab));

	g_regular_task_flag = true;
	while(1)
	{
		if(0==g_sys_ticks%(20*SYSTIME_CLOCK))
		{
			g_regular_task_flag = true;
		}

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

			LEDTOGGLE(0);

			printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d, %u\n",
					__TRICORE_NAME__,
					__TRICORE_CORE__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					SYSTEM_GetStmClock()/1000000,
					SYSTEM_IsCacheEnabled(),
					HAL_GetTick());

			printf("CPUID\t%08X\t:%08X\n\n", CPU_CPU_ID, _mfcr(CPU_CPU_ID));
			printf("CCTRL\t%08X\t:%08X\n\n", CPU_CCTRL, _mfcr(CPU_CCTRL));
			printf("CCNT\t%08X\t:%08X\n\n", CPU_CCNT, _mfcr(CPU_CCNT));
			printf("ICNT\t%08X\t:%08X\n\n", CPU_ICNT, _mfcr(CPU_ICNT));
			printf("M1CNT\t%08X\t:%08X\n\n", CPU_M1CNT, _mfcr(CPU_M1CNT));
			printf("M2CNT\t%08X\t:%08X\n\n", CPU_M2CNT, _mfcr(CPU_M2CNT));
			printf("M3CNT\t%08X\t:%08X\n\n", CPU_M3CNT, _mfcr(CPU_M3CNT));
			flush_stdout();
		}
	}

	return EXIT_SUCCESS;
}

