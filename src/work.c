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

	printf("%s CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d\n",
			__TIME__,
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			SYSTEM_IsCacheEnabled());

	flush_stdout();

	volatile uint32_t a;
	volatile uint32_t b;
	volatile uint32_t c;
	volatile uint32_t d;
	volatile pack32 packA;
	volatile pack32 packB;
	volatile pack32 packC;
	volatile pack32 packD;
	volatile int32_t ai;
	volatile int32_t bi;
	volatile uint32_t res;
	volatile int32_t res_i;
	volatile uint64_t a64;
	volatile uint64_t res64;
	volatile float fA;
	volatile float fB;
	volatile float f_res;
	volatile pack64 p_res_64;
	volatile pack64 p_a_64;
	volatile pack32 p_b_32;

	printf("\nTest J\n");
	a = 1000;
	res = Ifx_J(a);
	printf("J[%u] = %u\n", a, res);
	flush_stdout();

	printf("\nTest JA\n");
	a = 1000;
	res = Ifx_JA(a);
	printf("JA[%u] = %u\n", a, res);
	flush_stdout();

	printf("\nTest INS.T\n");
	a = 0xFFFFFFFF;
	b = 0x11223344;
	res = Ifx_InsT(a, b);
	printf("INS.T[%08X, %u, %08X, %u] = %08X\n", a, 5, b, 7, res);
	a = 0;
	res = Ifx_InsT(a, b);
	printf("INS.T[%08X, %u, %08X, %u] = %08X\n", a, 5, b, 7, res);
	flush_stdout();

	printf("\nTest INSN.T\n");
	a = 0xFFFFFFFF;
	b = 0x11223344;
	res = Ifx_Ins_nT(a, b);
	printf("INSN.T[%08X, %u, %08X, %u] = %08X\n", a, 5, b, 7, res);
	a = 0;
	res = Ifx_Ins_nT(a, b);
	printf("INSN.T[%08X, %u, %08X, %u] = %08X\n", a, 5, b, 7, res);
	flush_stdout();

	printf("\nTest INSERT\n");
	a = 0;
	b = 0xFF;
	res = Ifx_Insert(a, b, 8);
	printf("INSERT[%08X, %08X, %u] = %08X\n", a, b, 8, res);
	res = Ifx_Insert(a, b, 16);
	printf("INSERT[%08X, %08X, %u] = %08X\n", a, b, 16, res);
	flush_stdout();

	printf("\nTest ISYNC\n");
	Ifx_Isync();
	flush_stdout();

	printf("\nTest IXMAX\n");
	p_res_64.u16[0] = 0;
	p_res_64.u16[1] = 0;
	p_res_64.u16[2] = 0;
	p_res_64.u16[3] = 0;
	p_a_64.u16[0] = 0;
	p_a_64.u16[1] = 0;
	p_a_64.u16[2] = 0;
	p_a_64.u16[3] = 0;
	p_b_32.i16[0] = 1000;
	p_b_32.i16[1] = -1;
	printf("IXMAX[%016llX,%i, %i] = %i, %i, %i, %i\n",
			p_a_64.u64, p_b_32.i16[0], p_b_32.i16[1],
			p_res_64.i16[0], p_res_64.i16[1], p_res_64.i16[2], p_res_64.i16[3]);
	p_res_64.u64 = Ifx_Ixmax(p_a_64.u64, p_b_32.u32);
	printf("IXMAX[%016llX,%i, %i] = %i, %i, %i, %i\n",
			p_a_64.u64, p_b_32.i16[0], p_b_32.i16[1],
			p_res_64.i16[0], p_res_64.i16[1], p_res_64.i16[2], p_res_64.i16[3]);
	flush_stdout();

	printf("\nTest IXMIN\n");
	p_res_64.u16[0] = 0;
	p_res_64.u16[1] = 0;
	p_res_64.u16[2] = 0;
	p_res_64.u16[3] = 0;
	p_a_64.u16[0] = 0;
	p_a_64.u16[1] = 0;
	p_a_64.u16[2] = 0;
	p_a_64.u16[3] = 0;
	p_b_32.i16[0] = 1000;
	p_b_32.i16[1] = -1;
	printf("IXMIN[%016llX,%i, %i] = %i, %i, %i, %i\n",
			p_a_64.u64, p_b_32.i16[0], p_b_32.i16[1],
			p_res_64.i16[0], p_res_64.i16[1], p_res_64.i16[2], p_res_64.i16[3]);
	p_res_64.u64 = Ifx_Ixmin(p_a_64.u64, p_b_32.u32);
	printf("IXMIN[%016llX,%i, %i] = %i, %i, %i, %i\n",
			p_a_64.u64, p_b_32.i16[0], p_b_32.i16[1],
			p_res_64.i16[0], p_res_64.i16[1], p_res_64.i16[2], p_res_64.i16[3]);
	flush_stdout();

	printf("\nTest IXMAX_U\n");
	p_res_64.u16[0] = 0;
	p_res_64.u16[1] = 0;
	p_res_64.u16[2] = 0;
	p_res_64.u16[3] = 0;
	p_a_64.u16[0] = 0;
	p_a_64.u16[1] = 0;
	p_a_64.u16[2] = 0;
	p_a_64.u16[3] = 0;
	p_b_32.i16[0] = 1000;
	p_b_32.u16[1] = (uint16_t)-1;
	printf("IXMAX_U[%016llX,%u, %u] = %u, %u, %u, %u\n",
			p_a_64.u64, p_b_32.u16[0], p_b_32.u16[1],
			p_res_64.u16[0], p_res_64.u16[1], p_res_64.u16[2], p_res_64.u16[3]);
	p_res_64.u64 = Ifx_Ixmax_U(p_a_64.u64, p_b_32.u32);
	printf("IXMAX_U[%016llX,%u, %u] = %u, %u, %u, %u\n",
			p_a_64.u64, p_b_32.u16[0], p_b_32.u16[1],
			p_res_64.u16[0], p_res_64.u16[1], p_res_64.u16[2], p_res_64.u16[3]);
	flush_stdout();

	printf("\nTest IXMIN_U\n");
	p_res_64.u16[0] = 0;
	p_res_64.u16[1] = 0;
	p_res_64.u16[2] = 0;
	p_res_64.u16[3] = 0;
	p_a_64.u16[0] = 0;
	p_a_64.u16[1] = 0;
	p_a_64.u16[2] = 0;
	p_a_64.u16[3] = 0;
	p_b_32.u16[0] = 1000;
	p_b_32.u16[1] = (uint16_t)-1;
	printf("IXMIN_U[%016llX,%u, %u] = %u, %u, %u, %u\n",
			p_a_64.u64, p_b_32.u16[0], p_b_32.u16[1],
			p_res_64.u16[0], p_res_64.u16[1], p_res_64.u16[2], p_res_64.u16[3]);
	p_res_64.u64 = Ifx_Ixmin_U(p_a_64.u64, p_b_32.u32);
	printf("IXMIN_U[%016llX,%u, %u] = %u, %u, %u, %u\n",
			p_a_64.u64, p_b_32.u16[0], p_b_32.u16[1],
			p_res_64.u16[0], p_res_64.u16[1], p_res_64.u16[2], p_res_64.u16[3]);
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

			printf("%s CPU:%u MHz,Sys:%u MHz, %u, CacheEn:%d\n",
					__TIME__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					HAL_GetTick(),
					SYSTEM_IsCacheEnabled());

			__asm__ volatile ("nop" ::: "memory");
			__asm volatile ("" : : : "memory");
			/* wait until sending has finished */
			while (_uart_sending())
				;
		}
	}

	return EXIT_SUCCESS;
}

