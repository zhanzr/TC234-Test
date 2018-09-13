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

void flush_stdout(void)
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

	//Test AND Instruction
	printf("\nTest AND\n");
	uint32_t a = 0x1111FFFF;
	uint32_t b = 0x33333333;
	uint32_t c = 0xFFFF0001;
	uint32_t res = Ifx_And(a, b);
	printf("AND[%08X,%08X]=%08X\n", a, b, res);
	flush_stdout();
	b = 0;
	res = Ifx_And(a, b);
	printf("AND[%08X,%08X]=%08X\n", a, b, res);
	flush_stdout();

	printf("\nTest ANDI\n");
	res = Ifx_AndI(a);
	printf("AND[%08X,%03X]=%08X\n", a, 0x0f, res);
	flush_stdout();

	printf("\nTest ANDN\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_Andn(a, b);
	printf("ANDN[%08X,%08X]=%08X\n", a, b, res);
	flush_stdout();
	b = 0;
	res = Ifx_Andn(a, b);
	printf("ANDN[%08X,%08X]=%08X\n", a, b, res);
	flush_stdout();

	printf("\nTest ANDI\n");
	res = Ifx_AndnI(a);
	printf("ANDN[%08X,%03X]=%08X\n", a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.EQ\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_And_EQ(a, b, c);
	printf("AND.EQ[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();
	b = a;
	res = Ifx_And_EQ(a, b, c);
	printf("AND.EQ[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();

	printf("\nTest AND.EQ.I\n");
	res = Ifx_AndI_EQ(a, c);
	printf("ANDN[%08X, %08X,%03X]=%08X\n", c, a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.NE\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_And_NE(a, b, c);
	printf("AND.NE[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();
	b = a;
	res = Ifx_And_NE(a, b, c);
	printf("AND.NE[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();

	printf("\nTest AND.NE.I\n");
	res = Ifx_AndI_NE(a, c);
	printf("ANDN[%08X, %08X,%03X]=%08X\n", c, a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.GE\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_And_GE(a, b, c);
	printf("AND.GE[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();
	b = a;
	res = Ifx_And_GE(a, b, c);
	printf("AND.GE[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();

	printf("\nTest AND.GE.I\n");
	res = Ifx_AndI_GE(a, c);
	printf("AND.GE[%08X, %08X,%03X]=%08X\n", c, a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.GE.U\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_And_GE_U(a, b, c);
	printf("AND.GE.U[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();
	b = a;
	res = Ifx_And_GE_U(a, b, c);
	printf("AND.GE.U[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();

	printf("\nTest AND.GE.U.I\n");
	res = Ifx_AndI_GE_U(a, c);
	printf("AND.GE.U[%08X, %08X,%03X]=%08X\n", c, a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.LT\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_And_LT(a, b, c);
	printf("AND.LT[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();
	b = a;
	res = Ifx_And_LT(a, b, c);
	printf("AND.LT[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();

	printf("\nTest AND.LT.I\n");
	res = Ifx_AndI_LT(a, c);
	printf("AND.LT[%08X, %08X,%03X]=%08X\n", c, a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.LT.U\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_And_LT_U(a, b, c);
	printf("AND.LT.U[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();
	b = a;
	res = Ifx_And_LT_U(a, b, c);
	printf("AND.LT.U[%08X, %08X,%08X]=%08X\n", c, a, b, res);
	flush_stdout();

	printf("\nTest AND.LT.U.I\n");
	res = Ifx_AndI_LT_U(a, c);
	printf("ADD.LT.U[%08X, %08X,%03X]=%08X\n", c, a, 0x0f, res);
	flush_stdout();

	printf("\nTest AND.T\n");
	res = Ifx_And_T(a, b);
	printf("AND.T[%08X(%d), %08X(%d)]=%08X\n", a,  0, b, 0, res);
	flush_stdout();

	printf("\nTest ANDN.T\n");
	res = Ifx_Andn_T(a, b);
	printf("ANDN.T[%08X(%d), %08X(%d)]=%08X\n", a,  0, b, 0, res);
	flush_stdout();

	printf("\nTest ANDAND.T\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_AndAnd_T(a, b, c);
	printf("ANDAND.T[%08X, %08X(%d),%08X(%d)]=%08X\n", c, a,1, b,1, res);
	flush_stdout();

	printf("\nTest ANDANDN.T\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_AndAndn_T(a, b, c);
	printf("ANDANDN.T[%08X, %08X(%d),%08X(%d)]=%08X\n", c, a,1, b,1, res);
	flush_stdout();

	printf("\nTest ANDOR.T\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_AndOr_T(a, b, c);
	printf("ANDOR.T[%08X, %08X(%d),%08X(%d)]=%08X\n", c, a,1, b,1, res);
	flush_stdout();

	printf("\nTest ANDNOR.T\n");
	a = 0x1111FFFF;
	b = 0x33333333;
	res = Ifx_AndNor_T(a, b, c);
	printf("ANDNOR.T[%08X, %08X(%d),%08X(%d)]=%08X\n", c, a,1, b,1, res);
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

