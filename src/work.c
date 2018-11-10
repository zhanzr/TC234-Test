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

	printf("\nTest PACK\n");
	p_a_64.u32[0] = (1+(uint32_t)INT32_MAX)/1;
	p_a_64.u32[1] = 1;
	ai = 1;
	f_res = Ifx_PACK(p_a_64.u64, ai);
	printf("PACK[%08X %08X, %i] = %f[%08X]\n", p_a_64.u32[1], p_a_64.u32[0], ai, f_res, *((uint32_t*)&f_res));
	p_a_64.u32[0] = (1+(uint32_t)INT32_MAX)/1;
	p_a_64.u32[1] = 0;
	ai = 1;
	f_res = Ifx_PACK(p_a_64.u64, ai);
	printf("PACK[%08X %08X, %i] = %f[%08X]\n", p_a_64.u32[1], p_a_64.u32[0], ai, f_res, *((uint32_t*)&f_res));

	printf("\nTest UNPACK\n");
	fA = 10000.0;
	p_res_64.u64 = Ifx_UNPACK(fA);
	printf("UNPACK[%f %08X] = %08X %08X\n", fA, *((uint32_t*)&fA), p_res_64.u32[1], p_res_64.u32[0]);
	fA = 1.0;
	p_res_64.u64 = Ifx_UNPACK(fA);
	printf("UNPACK[%f %08X] = %08X %08X\n", fA, *((uint32_t*)&fA), p_res_64.u32[1], p_res_64.u32[0]);
	flush_stdout();

	printf("\nTest PARITY\n");
	a = 0x00010203;
	b = Ifx_PARITY(a);
	printf("PARITY[%08X] = %08X\n", a, b);
	a += 0x01010101;
	b = Ifx_PARITY(a);
	printf("PARITY[%08X] = %08X\n", a, b);
	flush_stdout();

	printf("\nTest Q31TOF\n");
	a = INT32_MAX;
	b = 10;
	f_res = Ifx_Q31TOF(a, b);
	printf("Q31TOF[%08X, %08X] = %f[%08X]\n", a, b, f_res, *((uint32_t*)&f_res));
	a = INT32_MAX/2;
	b = 20;
	f_res = Ifx_Q31TOF(a, b);
	printf("Q31TOF[%08X, %08X] = %f[%08X]\n", a, b, f_res, *((uint32_t*)&f_res));
	flush_stdout();

	printf("\nTest QSEED_F\n");
	fA = 2500.0;
	f_res = Ifx_QSEED_F(fA);
	printf("QSEED_F[%f] = %f\n", fA, f_res);
	fA = 0.0001;
	f_res = Ifx_QSEED_F(fA);
	printf("QSEED_F[%f] = %f\n", fA, f_res);
	flush_stdout();

	printf("\nTest RESTORE\n");
	printf("\nTest RET\n");
	printf("\nTest RFE\n");
	printf("\nTest RFM\n");
	printf("\nTest RSLCX\n");
	printf("\nTest RSTV\n");
	//	Ifx_RESTORE();
	//	Ifx_RET();
	//	Ifx_RFE();
	//	Ifx_RFM();
	//	Ifx_RSLCX();
	//	Ifx_RSTV();
	flush_stdout();

	printf("\nTest RSUB\n");
	ai = 1;
	bi = Ifx_RSUB(ai);
	printf("RSUB[%i] = %i\n", ai, bi);
	ai = INT_MIN;
	bi = Ifx_RSUB(ai);
	printf("RSUB[%i] = %i\n", ai, bi);
	flush_stdout();

	printf("\nTest RSUBS\n");
	ai = 1;
	bi = Ifx_RSUBS(ai);
	printf("RSUBS[%i] = %i\n", ai, bi);
	ai = INT_MIN;
	bi = Ifx_RSUBS(ai);
	printf("RSUBS[%i] = %i\n", ai, bi);
	flush_stdout();

	printf("\nTest RSUBS_U\n");
	a = 1;
	b = Ifx_RSUBS_U(a);
	printf("RSUBS_U[%u] = %u\n", a, b);
	a = INT_MIN;
	b = Ifx_RSUBS_U(a);
	printf("RSUBS_U[%u] = %u\n", a, b);
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

