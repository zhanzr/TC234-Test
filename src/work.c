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

	//Test B-C Instruction
	volatile uint32_t a;
	volatile uint32_t b;
	volatile uint32_t c;
	volatile uint32_t d;
	volatile int32_t ai;
	volatile uint32_t res;
	volatile uint64_t a64;
	volatile uint64_t res64;
	volatile float fA;
	volatile float fB;
	volatile float f_res;
	volatile pack64 p_res_64;

#define	TEST_NOP_NUM	1000000

	printf("\nTest DEBUG\n");
	a = g_sys_ticks;
	for(size_t i=0; i<TEST_NOP_NUM; ++i)
	{
		Ifx_Debug();
	}
	b = g_sys_ticks;
	for(size_t i=0; i<TEST_NOP_NUM; ++i)
	{
		Ifx_Nop();
	}
	c = g_sys_ticks;
	printf("DEBUG[%u, %u, %u]\n", a, b, c);
	flush_stdout();

	printf("\nTest DEXTR\n");
	a = 0x11223344;
	b = 0x55667788;
	c = 8;
	res = Ifx_Dextr(a, b, c);
	printf("DEXTR[%08X,%08X, %u] = %08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest DEXTR_I\n");
	a = 0x11223344;
	b = 0x55667788;
	res = Ifx_Dextr_I(a, b);
	printf("DEXTR_I[%08X,%08X, %u] = %08X\n", a, b, 16, res);
	flush_stdout();

	printf("\nTest ENABLE DISABLE 1\n");
	a = g_sys_ticks;
	for(volatile size_t i=0; i<TEST_NOP_NUM; ++i)
	{
		Ifx_Nop();
	}
	b = g_sys_ticks;
	flush_stdout();
	printf("[%u, %u]\n", a, b);
	flush_stdout();

	printf("\nTest ENABLE DISABLE 2\n");
	Ifx_Disable();
	a = g_sys_ticks;
	for(volatile size_t i=0; i<TEST_NOP_NUM; ++i)
	{
		Ifx_Nop();
	}
	b = g_sys_ticks;
	Ifx_Enable();
	Ifx_Dsync();
	printf("[%u, %u]\n", a, b);
	flush_stdout();

	printf("\nTest DSYNC\n");
	Ifx_Dsync();

	printf("\nTest DIV\n");
	ai=-3334;
	b = 3;
	p_res_64.u64 = Ifx_Div(ai, b);
	printf("DIV[%d,%d] = %d,%d\n", ai, b, p_res_64.i32[0], p_res_64.i32[1]);
	flush_stdout();

	printf("\nTest DIV_U\n");
	a = UINT32_MAX/2;
	b = 3;
	p_res_64.u64 = Ifx_Div_U(a, b);
	printf("DIV_U[%u,%u] = %u,%u\n", a, b, p_res_64.u32[0], p_res_64.u32[1]);
	flush_stdout();

	printf("\nTest DIV_F\n");
	fA = M_PI;
	fB = 2;
	f_res = Ifx_Div_F(fA, fB);
	printf("DIV_F[%f, %f] = %f\n", fA, fB, f_res);
	flush_stdout();

	printf("\nTest DVINIT\n");
	ai=-3333;
	b = 3;
	res64 = Ifx_DivInit(ai, b);
	printf("DVINIT[%08X,%08X] = %016llX\n", ai, b, res64);
	flush_stdout();

	printf("\nTest DVSTEP\n");
	a64 = res64;
	b = 3;
	res64 = Ifx_Dvstep(a64, b);
	printf("DVSTEP[%016llX,%08X] = %016llX\n", a64, b, res64);
	flush_stdout();

	printf("\nTest DVADJ\n");
	a64 = res64;
	b = 3;
	res64 = Ifx_Dvadj(a64, b);
	printf("DVADJ[%016llX,%08X] = %016llX\n", a64, b, res64);
	flush_stdout();

	printf("\nTest DVINIT_U\n");
	a = 0x99663312;
	b = 3;
	res64 = Ifx_DivInit_U(a, b);
	printf("DVINIT_U[%08X,%08X] = %016llX\n", a, b, res64);
	flush_stdout();

	printf("\nTest DVSTEP_U\n");
	a64 = res64;
	b = 3;
	res64 = Ifx_Dvstep_U(a64, b);
	printf("DVSTEP_U[%016llX,%08X] = %016llX\n", a64, b, res64);
	flush_stdout();

	printf("\nTest DVADJ\n");
	a64 = res64;
	b = 3;
	res64 = Ifx_Dvadj(a64, b);
	printf("DVADJ[%016llX,%08X] = %016llX\n", a64, b, res64);
	flush_stdout();

	printf("\nTest DVINIT_B\n");
	ai=0xFF8080FF;
	b = 3;
	res64 = Ifx_DivInit_B(a, b);
	printf("DVINIT_B[%08X,%08X] = %016llX\n", a, b, res64);
	flush_stdout();

	printf("\nTest DVINIT_BU\n");
	a = 0x99663312;
	b = 3;
	res64 = Ifx_DivInit_BU(a, b);
	printf("DVINIT_BU[%08X,%08X] = %016llX\n", a, b, res64);
	flush_stdout();

	printf("\nTest DVINIT_H\n");
	ai=0xFFFF8000;
	b = 3;
	res64 = Ifx_DivInit_H(a, b);
	printf("DVINIT_H[%08X,%08X] = %016llX\n", a, b, res64);
	flush_stdout();

	printf("\nTest DVINIT_HU\n");
	a = 0x99663312;
	b = 3;
	res64 = Ifx_DivInit_HU(a, b);
	printf("DVINIT_HU[%08X,%08X] = %016llX\n", a, b, res64);
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

