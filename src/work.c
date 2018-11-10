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

	printf("\nTest SAT.B\n");
	ai = -1000;
	bi = 1000;
	ci = Ifx_SAT_B(ai);
	di = Ifx_SAT_B(bi);
	printf("SAT.B[%i] = %i\n", ai, ci);
	printf("SAT.B[%i] = %i\n", bi, di);
	ai = -128;
	bi = 127;
	ci = Ifx_SAT_B(ai);
	di = Ifx_SAT_B(bi);
	printf("SAT.B[%i] = %i\n", ai, ci);
	printf("SAT.B[%i] = %i\n", bi, di);
	flush_stdout();

	printf("\nTest SAT.H\n");
	ai = -1000;
	bi = 1000;
	ci = Ifx_SAT_H(ai);
	di = Ifx_SAT_H(bi);
	printf("SAT.H[%i] = %i\n", ai, ci);
	printf("SAT.H[%i] = %i\n", bi, di);
	ai = INT16_MIN-1;
	bi = INT16_MAX+1;
	ci = Ifx_SAT_H(ai);
	di = Ifx_SAT_H(bi);
	printf("SAT.H[%i] = %i\n", ai, ci);
	printf("SAT.H[%i] = %i\n", bi, di);
	flush_stdout();

	printf("\nTest SAT.BU\n");
	a = (uint32_t)-1000;
	b = 1000;
	c = Ifx_SAT_BU(a);
	d = Ifx_SAT_BU(b);
	printf("SAT.BU[%u] = %u\n", a, c);
	printf("SAT.BU[%u] = %u\n", b, d);
	a = 0;
	b = UINT8_MAX;
	c = Ifx_SAT_BU(a);
	d = Ifx_SAT_BU(b);
	printf("SAT.BU[%u] = %u\n", a, c);
	printf("SAT.BU[%u] = %u\n", b, d);
	flush_stdout();

	printf("\nTest SAT.HU\n");
	a = (uint32_t)INT16_MIN+1;
	b = INT16_MAX;
	c = Ifx_SAT_HU(a);
	d = Ifx_SAT_HU(b);
	printf("SAT.HU[%u] = %u\n", a, c);
	printf("SAT.HU[%u] = %u\n", b, d);
	a = (uint32_t)INT16_MIN-1;
	b = INT16_MAX+1;
	c = Ifx_SAT_HU(a);
	d = Ifx_SAT_HU(b);
	printf("SAT.HU[%u] = %u\n", a, c);
	printf("SAT.HU[%u] = %u\n", b, d);
	flush_stdout();

	printf("\nTest SEL\n");
	a = 0;
	b = UINT16_MAX;
	c = UINT16_MAX/2;
	d = Ifx_SEL(a, b, c);
	printf("SEL[%u %u %u] = %u\n", a, b, c, d);
	a = 1;
	b = UINT16_MAX/8;
	c = UINT16_MAX/4;
	d = Ifx_SEL(a, b, c);
	printf("SEL[%u %u %u] = %u\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SELN\n");
	a = 0;
	b = UINT16_MAX;
	c = UINT16_MAX/2;
	d = Ifx_SELN(a, b, c);
	printf("SELN[%u %u %u] = %u\n", a, b, c, d);
	a = 1;
	b = UINT16_MAX/8;
	c = UINT16_MAX/4;
	d = Ifx_SELN(a, b, c);
	printf("SELN[%u %u %u] = %u\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH\n");
	a = 0xFF;
	bi = 4;
	c = Ifx_SH(a, bi);
	printf("SH[%08X %i] = %08X\n", a, bi, c);
	a = 0xFF;
	bi = -4;
	c = Ifx_SH(a, bi);
	printf("SH[%08X %i] = %08X\n", a, bi, c);
	flush_stdout();

	printf("\nTest SH_H\n");
	a = 0x001100FF;
	bi = 4;
	c = Ifx_SH(a, bi);
	printf("SH_H[%08X %i] = %08X\n", a, bi, c);
	a = 0x001100FF;
	bi = -4;
	c = Ifx_SH(a, bi);
	printf("SH_H[%08X %i] = %08X\n", a, bi, c);
	flush_stdout();

	printf("\nTest SH.AND.T\n");
	a = 0xFF80;
	b = 1;
	c = 1;
	d = Ifx_SH_AND_T(a, b, c);
	printf("SH.AND.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_AND_T(a, b, c);
	printf("SH.AND.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.ANDN.T\n");
	a = 0xFF80;
	b = 1;
	c = 1;
	d = Ifx_SH_ANDN_T(a, b, c);
	printf("SH.ANDN.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_ANDN_T(a, b, c);
	printf("SH.ANDN.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.NAND.T\n");
	a = 0xFF80;
	b = 1;
	c = 1;
	d = Ifx_SH_NAND_T(a, b, c);
	printf("SH.NAND.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_NAND_T(a, b, c);
	printf("SH.NAND.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.NOR.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_NOR_T(a, b, c);
	printf("SH.NOR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_NOR_T(a, b, c);
	printf("SH.NOR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.OR.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_OR_T(a, b, c);
	printf("SH.OR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_OR_T(a, b, c);
	printf("SH.OR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.ORN.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_ORN_T(a, b, c);
	printf("SH.ORN.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_ORN_T(a, b, c);
	printf("SH.ORN.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.XOR.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_XOR_T(a, b, c);
	printf("SH.XOR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_XOR_T(a, b, c);
	printf("SH.XOR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.XNOR.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_XNOR_T(a, b, c);
	printf("SH.XNOR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_XNOR_T(a, b, c);
	printf("SH.XNOR.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.EQ.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_EQ(a, b, c);
	printf("SH.EQ.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_EQ(a, b, c);
	printf("SH.EQ.T[%08X %08X D0 %08X D0] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.NE.T\n");
	a = 0xFF80;
	b = 0;
	c = 0;
	d = Ifx_SH_NE(a, b, c);
	printf("SH.NE[%08X %08X %08X] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_NE(a, b, c);
	printf("SH.NE[%08X %08X %08X] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.GE\n");
	a = 0xFF80;
	b = 0;
	c = (uint32_t)-1;
	d = Ifx_SH_GE(a, b, c);
	printf("SH.GE[%08X %08X %08X] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_GE(a, b, c);
	printf("SH.GE[%08X %08X %08X] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.GE.U\n");
	a = 0xFF80;
	b = 0;
	c = (uint32_t)-1;
	d = Ifx_SH_GE_U(a, b, c);
	printf("SH.GE.U[%08X %08X %08X] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_GE_U(a, b, c);
	printf("SH.GE.U[%08X %08X %08X] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.LT\n");
	a = 0xFF80;
	b = 0;
	c = (uint32_t)-1;
	d = Ifx_SH_LT(a, b, c);
	printf("SH.LT[%08X %08X %08X] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_LT(a, b, c);
	printf("SH.LT[%08X %08X %08X] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SH.LT.U\n");
	a = 0xFF80;
	b = 0;
	c = (uint32_t)-1;
	d = Ifx_SH_LT_U(a, b, c);
	printf("SH.LT.U[%08X %08X %08X] = %08X\n", a, b, c, d);
	a = 0xFF80;
	b = 1;
	c = 0;
	d = Ifx_SH_LT_U(a, b, c);
	printf("SH.LT.U[%08X %08X %08X] = %08X\n", a, b, c, d);
	flush_stdout();

	printf("\nTest SHA\n");
	ai = 0x80007FFF;
	bi = 4;
	ci = Ifx_SHA(ai, bi);
	printf("SHA[%08X %i] = %08X\n", ai, bi, ci);
	ai = 0x80007FFF;
	bi = -4;
	ci = Ifx_SHA(ai, bi);
	printf("SHA[%08X %i] = %08X\n", ai, bi, ci);
	flush_stdout();

	printf("\nTest SHAS\n");
	ai = 0x80007FFF;
	bi = 4;
	ci = Ifx_SHAS(ai, bi);
	printf("SHAS[%08X %i] = %08X\n", ai, bi, ci);
	ai = 0x80007FFF;
	bi = -4;
	ci = Ifx_SHAS(ai, bi);
	printf("SHAS[%08X %i] = %08X\n", ai, bi, ci);
	flush_stdout();

	printf("\nTest SHA.H\n");
	ai = 0x80007FFF;
	bi = 4;
	ci = Ifx_SHA_H(ai, bi);
	printf("SHA.H[%08X %i] = %08X\n", ai, bi, ci);
	ai = 0x80007FFF;
	bi = -4;
	ci = Ifx_SHA_H(ai, bi);
	printf("SHA.H[%08X %i] = %08X\n", ai, bi, ci);
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

