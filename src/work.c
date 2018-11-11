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

#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"

#include "system_tc2x.h"
#include "machine/intrinsics.h"
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

	volatile uint32_t a;
	volatile uint32_t b;
	volatile uint32_t c;
	volatile uint32_t d;
	volatile uint32_t* p_a;
	volatile uint32_t* p_b;
	volatile uint32_t* p_c;
	volatile uint32_t* p_d;
	volatile pack32 packA;
	volatile pack32 packB;
	volatile pack32 packC;
	volatile pack32 packD;
	volatile int32_t ai;
	volatile int32_t bi;
	volatile int32_t ci;
	volatile int32_t di;
	volatile uint32_t res;
	volatile int32_t res_i;
	volatile uint64_t a64;
	volatile uint64_t b64;
	volatile uint64_t res64;
	volatile float fA;
	volatile float fB;
	volatile float fC;
	volatile float f_res;
	volatile pack64 p_res_64;
	volatile pack64 p_a_64;
	volatile pack32 p_b_32;

	printf("\nTest TRAPSV\n");
	Ifx_TRAPSV();
	a = UINT32_MAX;
	b = UINT32_MAX;
	c = a + b;
	flush_stdout();

	printf("\nTest TRAPSV\n");
	Ifx_TRAPV();
	a = UINT32_MAX;
	b = UINT32_MAX;
	c = a + b;
	flush_stdout();

	printf("\nTest UPDFL\n");
	Ifx_UPDFL(a);
	printf("%08X %08X\n", a, _mfcr(CPU_PSW));
	flush_stdout();

	printf("\nTest UTOF\n");
	a = UINT32_MAX/4;
	fA = Ifx_UTOF(a);
	printf("%08X %f %08X\n", a, fA, *(uint32_t*)&fA);
	a = UINT32_MAX/8;
	fA = Ifx_UTOF(a);
	printf("%08X %f %08X\n", a, fA, *(uint32_t*)&fA);
	flush_stdout();

	printf("\nTest XOR\n");
	a = 0x11111111;
	b = 0x88888888;
	c = Ifx_XOR(a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	a = 0;
	b = 0x88888888;
	c = Ifx_XOR(a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XNOR\n");
	a = 0x11111111;
	b = 0x88888888;
	c = Ifx_XNOR(a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	a = 0;
	b = 0x88888888;
	c = Ifx_XNOR(a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR_T\n");
	a = 0x11111111;
	b = 0x88888888;
	c = 0x80000000;
	d = Ifx_XOR_T(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	a = 0;
	b = 0x88888888;
	c = 0x80000000;
	d = Ifx_XOR_T(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XNOR_T\n");
	a = 0x11111111;
	b = 0x88888888;
	c = 0x80000000;
	d = Ifx_XNOR_T(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	a = 0;
	b = 0x88888888;
	d = Ifx_XNOR_T(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR.EQ\n");
	a = 0x11111111;
	b = 0x88888888;
	d = Ifx_XOR_EQ(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	b = a;
	d = Ifx_XOR_EQ(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR.NE\n");
	a = 0x11111111;
	b = 0x88888888;
	d = Ifx_XOR_NE(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	b = a;
	d = Ifx_XOR_NE(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR.GE\n");
	a = 0x11111111;
	b = 0x88888888;
	d = Ifx_XOR_GE(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	b = 0x77777777;
	d = Ifx_XOR_GE(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR.GE.U\n");
	a = 0x11111111;
	b = 0x88888888;
	d = Ifx_XOR_GE_U(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	b = 0x77777777;
	d = Ifx_XOR_GE_U(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR.LT\n");
	a = 0x11111111;
	b = 0x88888888;
	d = Ifx_XOR_LT(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	b = 0x77777777;
	d = Ifx_XOR_LT(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest XOR.LT.U\n");
	a = 0x11111111;
	b = 0x88888888;
	d = Ifx_XOR_LT_U(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	b = 0x77777777;
	d = Ifx_XOR_LT_U(c, a, b);
	printf("%08X %08X %08X %08X\n", a, b, c, d);
	flush_stdout();

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

			//			printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d, %u\n",
			//					__TRICORE_NAME__,
			//					__TRICORE_CORE__,
			//					SYSTEM_GetCpuClock()/1000000,
			//					SYSTEM_GetSysClock()/1000000,
			//					SYSTEM_GetStmClock()/1000000,
			//					SYSTEM_IsCacheEnabled(),
			//					HAL_GetTick());

			flush_stdout();
		}
	}

	return EXIT_SUCCESS;
}

