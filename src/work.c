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

	printf("\nTest ST.A\n");
	p_b = 0x12345678;
	Ifx_ST_A(&a, p_b);
	printf("ST.A[%08X] = %08X\t%08X\n", &a, a, p_b);
	flush_stdout();

	printf("\nTest ST.B\n");
	b = 0x12345678;
	Ifx_ST_B(&a, b);
	printf("ST.B[%08X] = %08X\t%08X\n", &a, a, b);
	flush_stdout();

	printf("\nTest ST.H\n");
	b = 0x12345678;
	Ifx_ST_H(&a, b);
	printf("ST.H[%08X] = %08X\t%08X\n", &a, a, b);
	flush_stdout();

	printf("\nTest ST.W\n");
	b = 0x12345678;
	Ifx_ST_W(&a, b);
	printf("ST.W[%08X] = %08X\t%08X\n", &a, a, b);
	flush_stdout();

	printf("\nTest ST.Q\n");
	b = 0x12345678;
	Ifx_ST_Q(&a, b);
	printf("[%08X] = %08X\t%08X\n", &a, a, b);
	flush_stdout();

	printf("\nTest ST.D\n");
	b64 = 0x9ABCDEF012345678;
	Ifx_ST_W(&a64, b64);
	printf("[%08X] = %016llX\t%0016llX\n", &a64, a64, b64);
	flush_stdout();

	printf("\nTest ST.DA\n");
	b64 = 0x123456789ABCDEF0;
	Ifx_ST_W(&a, &b64);
	printf("[%08X %08X] = %08X %08X\t%0016llX\n", &a, &b, a, b, b64);
	flush_stdout();

#define TEST_ADDR	0xD0020000
	printf("\nTest ST.T\n");
	Ifx_ST_T();
	printf("[%08X] = %08X\n", TEST_ADDR, *((uint32_t*)TEST_ADDR));
	flush_stdout();

	uint32_t tmpCSA[16] = {0};
	printf("\nTest STLCX\n");
//	Ifx_STLCX(tmpCSA);
//	for(uint32_t i=0; i<16; ++i)
//	{
//		printf("%08X ", tmpCSA[i]);
//	}
//	printf("\r\n");
	flush_stdout();

	printf("\nTest STUCX\n");
//	Ifx_STUCX(tmpCSA);
//	for(uint32_t i=0; i<16; ++i)
//	{
//		printf("%08X ", tmpCSA[i]);
//	}
//	printf("\r\n");
	flush_stdout();

	printf("\nTest SVLCX\n");
	//Ifx_SVLCX();
	flush_stdout();

	printf("\nTest SUB\n");
	ai = 1;
	bi = 10;
	ci = Ifx_SUB(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	ai = INT32_MIN;
	bi = 10;
	ci = Ifx_SUB(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	flush_stdout();

	printf("\nTest SUBS\n");
	ai = 1;
	bi = 10;
	ci = Ifx_SUBS(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	ai = INT32_MIN;
	bi = 10;
	ci = Ifx_SUBS(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	flush_stdout();

	printf("\nTest SUBS.U\n");
	a = 0;
	b = UINT32_MAX;
	c = Ifx_SUBS_U(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	a = 0x12345678;
	b = 0x11111111;
	c = Ifx_SUBS_U(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest SUB.B\n");
	a = 0;
	b = UINT32_MAX;
	c = Ifx_SUB_B(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	a = 0x12345678;
	b = 0x11111111;
	c = Ifx_SUB_B(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest SUB.H\n");
	a = 0;
	b = UINT32_MAX;
	c = Ifx_SUB_H(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	a = 0x12345678;
	b = 0x11111111;
	c = Ifx_SUB_H(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest SUBS.H\n");
	a = 0;
	b = UINT32_MAX;
	c = Ifx_SUBS_H(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	a = 0x12345678;
	b = 0x11111111;
	c = Ifx_SUBS_H(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest SUBS.HU\n");
	a = 0;
	b = UINT32_MAX;
	c = Ifx_SUBS_HU(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	a = 0x12345678;
	b = 0x11111111;
	c = Ifx_SUBS_HU(a, b);
	printf("[%08X - %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest SUB.A\n");
	p_a = Ifx_SUB_A(&a, &b);
	printf("[%08X - %08X] = %08X\n", (uint32_t)&a, (uint32_t)&b, (uint32_t)p_a);
	p_a = Ifx_SUB_A(&ai, &bi);
	printf("[%08X - %08X] = %08X\n", (uint32_t)&ai, (uint32_t)&bi, (uint32_t)p_a);
	flush_stdout();

	printf("\nTest SUBC\n");
	ai = 1;
	bi = 10;
	ci = Ifx_SUBC(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	ai = 101;
	bi = 10;
	ci = Ifx_SUBC(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	flush_stdout();

	printf("\nTest SUBX\n");
	ai = 1;
	bi = 10;
	ci = Ifx_SUBX(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	ai = 101;
	bi = 10;
	ci = Ifx_SUBX(ai, bi);
	printf("[%i - %i] = %i\n", ai, bi, ci);
	flush_stdout();

	printf("\nTest SUBF\n");
	fA = 1.1;
	fB = 2.2;
	fC = Ifx_SUB_F(fA, fB);
	printf("[%f - %f] = %f\n", fA, fB, fC);
	fA = 3.3;
	fB = 9.9;
	fC = Ifx_SUB_F(fA, fB);
	printf("[%f - %f] = %f\n", fA, fB, fC);
	flush_stdout();

	printf("\nTest SWAP.W\n");
	a = 1;
	b = 2;
	Ifx_SWAP_W(&a, b);
	printf("[%08X %08X] = %08X %08X\n", (uint32_t)&a, (uint32_t)&b, a, b);
	a = INT32_MAX;
	b = UINT32_MAX;
	Ifx_SWAP_W(&a, b);
	printf("[%08X %08X] = %08X %08X\n", (uint32_t)&a, (uint32_t)&b, a, b);
	flush_stdout();

	printf("\nTest SYSCALL\n");
	Ifx_SYSCALL(0);
	Ifx_SYSCALL(1);
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

