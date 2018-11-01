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

	printf("\nTest LD.A\n");
	a = 0x10000;
	b = 0x20000;
	p_a = Ifx_LD_A(&a);
	p_b = Ifx_LD_A(&b);
	printf("LD.A[%p] = %p\n", &a, p_a);
	printf("LD.A[%p] = %p\n", &b, p_b);
	flush_stdout();

	printf("\nTest LD.B\n");
	a = 0x12345678;
	b = 0x9ABCDEF0;
	int8_t byte_a = Ifx_LD_B(&a);
	int8_t byte_b = Ifx_LD_B(&b);
	printf("LD.B[%p] = %02X\n", &a, byte_a);
	printf("LD.B[%p] = %02X\n", &b, byte_b);
	flush_stdout();

	printf("\nTest LD.BU\n");
	uint8_t ubyte_a = Ifx_LD_BU(&a);
	uint8_t ubyte_b = Ifx_LD_BU(&b);
	printf("LD.BU[%p] = %02X\n", &a, ubyte_a);
	printf("LD.BU[%p] = %02X\n", &b, ubyte_b);
	flush_stdout();

	printf("\nTest LD.D\n");
	a64 = 0x123456789ABCDEF0;
	res64 = Ifx_LD_D(&a64);
	printf("LD.D[%016llx] = %016llx\n", a64, res64);
	flush_stdout();

	printf("\nTest LD.A\n");
	a64 = 0x123456789ABCDEF0;
	uint64_t* p64 = Ifx_LD_DA(&a64);
	printf("LD.DA[%p] = %016llx\n", &a64, p64);
	flush_stdout();

	printf("\nTest LD.H\n");
	a = 0x12345678;
	b = 0x9ABCDEF0;
	int16_t i16_a = Ifx_LD_H(&a);
	int16_t i16_b = Ifx_LD_H(&b);
	printf("LD.H[%p] = %i\n", &a, i16_a);
	printf("LD.H[%p] = %i\n", &b, i16_b);
	flush_stdout();

	printf("\nTest LD.HU\n");
	uint16_t u16_a = Ifx_LD_HU(&a);
	uint16_t u16_b = Ifx_LD_HU(&b);
	printf("LD.HU[%p] = %u\n", &a, u16_a);
	printf("LD.HU[%p] = %u\n", &b, u16_b);
	flush_stdout();

//	Ifx_LDUCX();
//	Ifx_LDLCX();

	printf("\nTest LD.Q\n");
	a = 0x10000;
	p_a = &a;
	c = Ifx_LD_Q(p_a);
	printf("LD.Q[%p] = %08X\n", &a, c);
	flush_stdout();

	printf("\nTest LD.W\n");
	a = 0x10000;
	b = 0x20000;
	c = Ifx_LD_W(&a);
	d = Ifx_LD_W(&b);
	printf("LD.W[%p] = %08X\n", &a, c);
	printf("LD.W[%p] = %08X\n", &b, d);
	flush_stdout();

//	printf("\nTest LDMST\n");
//	a64 = 0x123456789ABCDEF0;
//	Ifx_LDMST(a64);
//	printf("LDMST[%016llx] = %016llx\n", 0x123456789ABCDEF0, a64);
	flush_stdout();

	printf("\nTest LEA\n");
	p_a = Ifx_LEA();
	printf("LEA[%p] = %p\n", &a, p_a);
	flush_stdout();

	printf("\nTest LOOP\n");
	a = 1;
	b = 10;
	d = Ifx_LOOP(a, b);
	printf("LOOP[%u, %u] = %u\n", a, b, d);
	flush_stdout();

	printf("\nTest LOOPU\n");
	a = 1;
	b = 10;
	c = 20;
	d = Ifx_LOOPU(a, b, c);
	printf("LOOPU[%u, %u, %u] = %u\n", a, b, c, d);
	flush_stdout();

	printf("\nTest LT\n");
	a = 1;
	b = -10;
	d = Ifx_LT(a, b);
	printf("LT[%08X, %08X] = %08X\n", a, b, d);
	flush_stdout();

	printf("\nTest LT.U\n");
	a = 1;
	b = -10;
	d = Ifx_LT_U(a, b);
	printf("LT.U[%08X, %08X] = %08X\n", a, b, d);
	flush_stdout();

	printf("\nTest LT.W\n");
	a = 1;
	b = -10;
	d = Ifx_LT_W(a, b);
	printf("LT.W[%08X, %08X] = %08X\n", a, b, d);
	flush_stdout();

	printf("\nTest LT.WU\n");
	a = 1;
	b = -10;
	d = Ifx_LT_WU(a, b);
	printf("LT.WU[%08X, %08X] = %08X\n", a, b, d);
	flush_stdout();

	printf("\nTest LT.B\n");
	packA.u32 = 0x11AA22BB;
	packB.u32 = 0x339988F0;
	packC = Ifx_LT_B(packA, packB);
	printf("LT.B[%08X, %08X] = %08X\n", packA.u32, packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest LT.BU\n");
	packA.u32 = 0x11AA22BB;
	packB.u32 = 0x339988F0;
	packC = Ifx_LT_BU(packA, packB);
	printf("LT.BU[%08X, %08X] = %08X\n", packA.u32, packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest LT.H\n");
	packA.u32 = 0x1234DEF0;
	packB.u32 = 0x9ABC5678;
	packC = Ifx_LT_H(packA, packB);
	printf("LT.B[%08X, %08X] = %08X\n", packA.u32, packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest LT.BU\n");
	packA.u32 = 0x1234DEF0;
	packB.u32 = 0x9ABC5678;
	packC = Ifx_LT_HU(packA, packB);
	printf("LT.HU[%08X, %08X] = %08X\n", packA.u32, packB.u32, packC.u32);
	flush_stdout();

	printf("\nTest LT.A\n");
	p_a = 0x12345678;
	p_b = 0x9ABCDEF0;
	a = Ifx_LT_A(p_a, p_b);
	printf("LT.A[%08X, %08X] = %08X\n", p_a, p_b, a);
	p_a = &a;
	p_b = &b;
	a = Ifx_LT_A(p_a, p_b);
	printf("LT.A[%08X, %08X] = %08X\n", p_a, p_b, a);
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

