#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
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

void TestFunc(void)
{
	printf("%s_%d\n", __func__, __LINE__);
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
	volatile uint64_t res64;
	volatile float fA;
	volatile float fB;
	volatile float fC;
	volatile float f_res;
	volatile pack64 p_res_64;
	volatile pack64 p_a_64;
	volatile pack32 p_b_32;

	printf("\nTest MSUB\n");
	ai = 0x100;
	bi = 0x200;
	ci = 0x300;
	di = Ifx_MSUB(ai, bi, ci);
	printf("MSUB[%i-%i*%i] = %i\n", ai, bi, ci, di);
	flush_stdout();

	printf("\nTest MSUBS\n");
	ai = 0x10000;
	bi = 0x20000;
	ci = 0x30000;
	di = Ifx_MSUBS(ai, bi, ci);
	printf("MSUBS[%i-%i*%i] = %i\n", ai, bi, ci, di);
	flush_stdout();

	printf("\nTest MSUB_U\n");
	a64 = 0x10000000;
	b = 0x200;
	c = 0x300;
	res64 = Ifx_MSUB_U(a64, b, c);
	printf("MSUB_U[%llu-%u*%u] = %llu\n", a64, b, c, res64);
	flush_stdout();

	printf("\nTest MSUBS_U\n");
	a64 = 0;
	b = 0x20000;
	c = 0x30000;
	res64 = Ifx_MSUBS_U(a64, b, c);
	printf("MSUBS_U[%llu-%u*%u] = %llu\n", a64, b, c, res64);
	flush_stdout();

	printf("\nTest MSUB_F\n");
	fA = 1.1;
	fB = 2.2;
	fC = 3.3;
	f_res = Ifx_MSUB_F(fA, fB, fC);
	printf("MSUB_F[%f-%f*%f] = %f\n", fA, fB, fC, f_res);
	flush_stdout();

	printf("\nTest NAND\n");
	a = 0xFFFFFFFF;
	b = 0x22222222;
	c = Ifx_NAND(a, b);
	printf("NAND[%08X, %08X] = %08X\n", a, b, c);
	b = 0x11111111;
	c = Ifx_NAND(a, b);
	printf("NAND[%08X, %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest NAND_T\n");
	a = 0xFFFFFFFF;
	b = 0x22222222;
	c = Ifx_NAND_T(a, b);
	printf("NAND_T[%08X, %08X, 0] = %08X\n", a, b, c);
	b = 0x11111111;
	c = Ifx_NAND_T(a, b);
	printf("NAND_T[%08X, %08X, 0] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest NE\n");
	a = 0x12345678;
	b = 0x55AACCDD;
	c = Ifx_NE(a, b);
	printf("NE[%08X, %08X] = %08X\n", a, b, c);
	b = a;
	c = Ifx_NE(a, b);
	printf("NE[%08X, %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest NE_A\n");
	p_a = &a;
	p_b = &b;
	c = Ifx_NE_A(p_a, p_b);
	printf("NE_A[%08X, %08X] = %08X\n", p_a, p_b, c);
	p_b = p_a;
	c = Ifx_NE_A(p_a, p_b);
	printf("NE_A[%08X, %08X] = %08X\n", p_a, p_b, c);
	flush_stdout();

	printf("\nTest NEZ_A\n");
	p_a = &a;
	c = Ifx_NEZ_A(p_a);
	printf("NEZ_A[%08X] = %08X\n", p_a, c);
	p_a = 0;
	c = Ifx_NEZ_A(p_a);
	printf("NEZ_A[%08X] = %08X\n", p_a, c);
	flush_stdout();

	printf("\nTest NOP\n");
	Ifx_NOP();

	printf("\nTest NOR\n");
	a = 0xAAAA5555;
	b = 0x22222222;
	c = Ifx_NOR(a, b);
	printf("NOR[%08X, %08X] = %08X\n", a, b, c);
	b = 0x11111111;
	c = Ifx_NOR(a, b);
	printf("NOR[%08X, %08X] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest NOR_T\n");
	a = 0xAAAA5555;
	b = 0x22222222;
	c = Ifx_NOR_T(a, b);
	printf("NOR_T[%08X, %08X, 0] = %08X\n", a, b, c);
	b = 0x11111111;
	c = Ifx_NOR_T(a, b);
	printf("NOR_T[%08X, %08X, 0] = %08X\n", a, b, c);
	flush_stdout();

	printf("\nTest NOT\n");
	a = 0xAAAA5555;
	c = Ifx_NOT(a);
	printf("NOT[%08X] = %08X\n", a, c);
	a = 0x11111111;
	c = Ifx_NOT(a);
	printf("NOT[%08X] = %08X\n", a, c);
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

			printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d, %u\n",
					__TRICORE_NAME__,
					__TRICORE_CORE__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					SYSTEM_GetStmClock()/1000000,
					SYSTEM_IsCacheEnabled(),
					HAL_GetTick());

			flush_stdout();
		}
	}

	return EXIT_SUCCESS;
}

