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

	/* initialise timer at SYSTIME_CLOCK rate */
	TimerInit(SYSTIME_CLOCK);
	/* add own handler for timer interrupts */
	TimerSetHandler(my_timer_handler);

	_init_uart(BAUDRATE);
	InitLED();

	printf("Tricore %04X Core:%04X CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d\n",
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

	printf("\nTest MFCR\n");
	b = _mfcr(CPU_CORE_ID);
	printf("MFCR[%X] = %08X\n", CPU_CORE_ID, b);
	flush_stdout();

	printf("\nTest MTCR\n");
	SYSTEM_EnaDisCache(0);
	printf("PCON0 = %08X\n", _mfcr(CPU_PCON0));
	printf("DCON0 = %08X\n", _mfcr(CPU_DCON0));
	flush_stdout();
	SYSTEM_EnaDisCache(1);
	printf("PCON0 = %08X\n", _mfcr(CPU_PCON0));
	printf("DCON0 = %08X\n", _mfcr(CPU_DCON0));
	flush_stdout();

	printf("\nTest MOV\n");
	a = Ifx_MOV(0x123);
	b = Ifx_MOV(0x456);
	printf("MOV[%X] = %08X\n", 0x123, a);
	printf("MOV[%X] = %08X\n", 0x456, b);
	flush_stdout();

	printf("\nTest MOVU\n");
	a = Ifx_MOV_U();
	printf("MOVU[%X] = %08X\n", 0x1234, a);
	flush_stdout();

	printf("\nTest MOVU\n");
	a = Ifx_MOVH();
	printf("MOVH[%X] = %08X\n", 0x1234, a);
	flush_stdout();

	printf("\nTest MOVA\n");
	a = 0x12345678;
	p_a = Ifx_MOV_A(a);
	printf("MOVA[%X] = %p\n", a, p_a);
	flush_stdout();

	printf("\nTest MOVH_A\n");
	p_a = Ifx_MOVH_A();
	printf("MOVA[%X] = %p\n", 0x1234,  p_a);
	flush_stdout();

	printf("\nTest MOV_AA\n");
	p_a = Ifx_MOV_AA(&a);
	printf("MOVA[%X] = %p\n", &a, p_a);
	flush_stdout();

	printf("\nTest MOVD\n");
	b = Ifx_MOV_D(&a);
	printf("MOVD[%X] = %08X\n", a,  b);
	flush_stdout();

	printf("\nTest MAX\n");
	ai = 0x12345678;
	bi = 0x89ABCDEF;
	ci = Ifx_MAX(ai, bi);
	printf("MAX[%i %i] = %i\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MAXU\n");
	a = 0x12345678;
	b = 0x89ABCDEF;
	c = Ifx_MAX_U(a, b);
	printf("MAX[%u %u] = %u\n", a,  b, c);
	flush_stdout();

	printf("\nTest MAX_B\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MAX_B(packA, packB);
	printf("MAXB[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MAX_BU\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MAX_BU(packA, packB);
	printf("MAXBU[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MAX_H\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MAX_H(packA, packB);
	printf("MAXH[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MAX_HU\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MAX_HU(packA, packB);
	printf("MAXHU[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MIN\n");
	ai = 0x12345678;
	bi = 0x89ABCDEF;
	ci = Ifx_MIN(ai, bi);
	printf("MIN[%i %i] = %i\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MINU\n");
	a = 0x12345678;
	b = 0x89ABCDEF;
	c = Ifx_MIN_U(a, b);
	printf("MIN[%u %u] = %u\n", a,  b, c);
	flush_stdout();

	printf("\nTest MIN_B\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MIN_B(packA, packB);
	printf("MINB[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MIN_BU\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MIN_BU(packA, packB);
	printf("MINBU[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MIN_H\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MIN_H(packA, packB);
	printf("MINH[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MIN_HU\n");
	packA.i32 = 0x119922AA;
	packB.i32 = 0x33778866;
	packC = Ifx_MIN_HU(packA, packB);
	printf("MINHU[%08X %08X] = %08X\n", packA.u32,  packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest MUL\n");
	ai = 0x11112222;
	bi = 2;
	ci = Ifx_MUL(ai, bi);
	printf("MUL[%i %i] = %i\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MULS\n");
	ai = 0x12345678;
	bi = 0x89ABCDEF;
	ci = Ifx_MULS(ai, bi);
	printf("MULS[%i %i] = %i\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MUL_U\n");
	a = 0x12345678;
	b = 0x89ABCDEF;
	res64 = Ifx_MUL_U(a, b);
	printf("MUL[%u %u] = %016llu\n", a,  b, res64);
	flush_stdout();

	printf("\nTest MULS_U\n");
	a = 0x12345678;
	b = 0x89ABCDEF;
	c = Ifx_MULS_U(a, b);
	printf("MULS[%u %u] = %u\n", a,  b, c);
	flush_stdout();

	printf("\nTest MUL_F\n");
	fA = 1.1;
	fB = 9.9;
	f_res = Ifx_MUL_F(fA, fB);
	printf("MUL[%f %f] = %f\n", fA,  fB, f_res);
	flush_stdout();

	//Fixed Point DSP
	printf("\nTest MUL.Q\n");
	ai = 0x12345678;
	bi = 0x89ABCDEF;
	ci = Ifx_MUL_Q(ai, bi);
	printf("MUL.Q[%08X %08X] = %08X\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MULR.H\n");
	ai = 0x12345678;
	bi = 0x89ABCDEF;
	ci = Ifx_MULR_H(ai, bi);
	printf("MULR.H[%08X %08X] = %08X\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MULR.Q\n");
	ai = 0x12345678;
	bi = 0x89ABCDEF;
	ci = Ifx_MULR_Q(ai, bi);
	printf("MULR.Q[%08X %08X] = %08X\n", ai,  bi, ci);
	flush_stdout();

	printf("\nTest MUL.H\n");
	a = 0x12345678;
	b = 0x89ABCDEF;
	res64 = Ifx_MUL_H(a, b);
	printf("MUL.H[%08X %08X] = %016llX\n", a,  b, res64);
	flush_stdout();

	printf("\nTest MULM.H\n");
	a = 0x12345678;
	b = 0x89ABCDEF;
	res64 = Ifx_MULM_H(a, b);
	printf("MULM.H[%08X %08X] = %016llX\n", a,  b, res64);
	flush_stdout();

	//Deprecated in TC2!
//	printf("\nTest MULMS.H\n");
//	a = 0x12345678;
//	b = 0x89ABCDEF;
//	res64 = Ifx_MULMS_H(a, b);
//	printf("MULMS.H[%08X %08X] = %016llX\n", a,  b, res64);
//	flush_stdout();

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

			printf("Tricore %04X Core:%04X CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d, %u\n",
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

