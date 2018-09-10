#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "led.h"
#include "uart_int.h"

#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"

#include "system_tc2x.h"
#include "machine/intrinsics.h"
#include "interrupts.h"

#include "asm_prototype.h"

#define BAUDRATE	115200

#define SYSTIME_CLOCK	1000	/* timer event rate [Hz] */

volatile uint32_t g_SysTicks;
volatile bool g_regular_task_flag;

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_SysTicks;
}

const uint32_t HAL_GetTick(void)
{
	return g_SysTicks;
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

	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	//Test Additon Instruction
	printf("\nTest ADD ADDC ADDI ADDIH\n");

	int32_t a = 0x11112222;
	int32_t b = 0x33334444;
	int32_t res = Ifx_Add(a, b);
	printf("ADD[%08X+%08X]=%08X\n", a, b, res);

	a = INT32_MAX;
	b = 1;
	res = Ifx_Add(a,b);
	a = 1;
	b = 1;
	res = Ifx_AddC(a,b);
	printf("ADDC[%08X+%08X]=%08X\n", a, b, res);

	a = 33;
	res = Ifx_AddI(a);
	printf("ADD[%i+%i]=%i\n", a, -10, res);

	a = 0x33;
	res = Ifx_AddI_Hi(a);
	printf("ADDIH[%08X+%08X]=%08X\n", a, b, res);

	 a = 0x11112222;
	 b = 0xFFFFFFFF;
	 res = Ifx_Addx(a, b);
	printf("ADDX[%08X+%08X]=%08X\n", a, b, res);

	a = 0x33;
	res = Ifx_Addx_I(a);
	printf("ADDX[%i+%i]=%i\n", a, -10, res);

	a = INT32_MAX;
	b = INT32_MAX;
	res = Ifx_Add(a, b);
	printf("ADD[%i+%i]=%i\n", a, b, res);
	res = Ifx_AddS(a, b);
	printf("ADDS[%i+%i]=%i\n", a, b, res);

	uint32_t c = UINT32_MAX;
	uint32_t d = UINT32_MAX;
	res = Ifx_Add(a, b);
	printf("ADD[%08X+%08X]=%08X\n", a, b, res);
	uint32_t resU = Ifx_AddS_U(c, c);
	printf("ADDS.U[%u+%u]=%u\n", c, d, resU);

	pack32 a_p32;
	pack32 b_p32;
	pack32 res_p32;
	a_p32.i16[0]=INT16_MAX;
	a_p32.i16[1]=INT16_MAX;
	b_p32.i16[0]=INT16_MAX;
	b_p32.i16[1]=INT16_MAX;
	res_p32.i32 = Ifx_Add_H(a_p32.i32, b_p32.i32);
	printf("ADD.H[%i,%i + %i,%i]=%i,%i\n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	res_p32.i32 = Ifx_AddS_H(a_p32.i32, b_p32.i32);
	printf("ADDS.H[%i,%i + %i,%i]=%i,%i\n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);

	a_p32.u16[0]=UINT16_MAX;
	a_p32.u16[1]=UINT16_MAX;
	b_p32.u16[0]=UINT16_MAX;
	b_p32.u16[1]=UINT16_MAX;
	res_p32.i32 = Ifx_Add_H(a_p32.i32, b_p32.i32);
	printf("ADD.H[%i,%i + %i,%i]=%i,%i\n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	res_p32.i32 = Ifx_AddS_HU(a_p32.i32, b_p32.i32);
	printf("ADDS.HU[%i,%i + %i,%i]=%i,%i\n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);

	uint8_t tmpArr[16];
	printf("%08X\n", (uint32_t)tmpArr);
	uint8_t* pTest = Ifx_AddA(tmpArr, tmpArr);
	printf("ADDA[%08X,%08X]=%08X\n", (uint32_t)tmpArr, (uint32_t)tmpArr, (uint32_t)pTest);

	pTest = Ifx_AddA_4(tmpArr);
	printf("ADDA[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);

	pTest = Ifx_Addih_A(tmpArr);
	printf("ADDA[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);

	pTest = Ifx_Addsc_A(tmpArr, 4);
	printf("ADDSC.A[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);

	pTest = Ifx_Addsc_AT(tmpArr, 4);
	printf("ADDSC.AT[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);

	float fA = 1.123456789;
	float fB = 0.987654321;
	float res_f = Ifx_Add_F(fA, fB);
	printf("ADD.F(%f,%f)=%f\n", fA, fB, res_f);

	a_p32.i8[0]=INT8_MAX/3;
	a_p32.i8[1]=INT8_MAX/4;
	a_p32.i8[2]=INT8_MAX/5;
	a_p32.i8[3]=INT8_MAX/6;
	b_p32.i8[0]=INT8_MAX/3;
	b_p32.i8[1]=INT8_MAX/4;
	b_p32.i8[2]=INT8_MAX/5;
	b_p32.i8[3]=INT8_MAX/6;
	res_p32.i32 = Ifx_Add_B(a_p32.i32, b_p32.i32);
	printf("ADD.B[%i,%i,%i,%i + %i,%i,%i,%i]=%i,%i,%i,%i\n",
			a_p32.i8[0], a_p32.i8[1],a_p32.i8[2], a_p32.i8[3],
			b_p32.i8[0], b_p32.i8[1],b_p32.i8[2], b_p32.i8[3],
			res_p32.i8[0], res_p32.i8[1],res_p32.i8[2], res_p32.i8[3]);

	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABS.B\n");
	a_p32.i8[0] = INT8_MIN/2;
	a_p32.i8[1] = INT8_MIN/3;
	a_p32.i8[2] = INT8_MIN/4;
	a_p32.i8[3] = INT8_MIN/5;
	res_p32.u32 = Ifx_Abs_B(a_p32.i32);
	printf("Abs_B[%i %i %i %i]=%i %i %i %i\n",
			a_p32.i8[0], a_p32.i8[1], a_p32.i8[2], a_p32.i8[3],
			res_p32.i8[0], res_p32.i8[1], res_p32.i8[2], res_p32.i8[3]);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABS.H\n");
	a_p32.i16[0] = INT16_MIN/2;
	a_p32.i16[1] = INT16_MIN/3;
	res_p32.u32 = Ifx_Abs_H(a_p32.i32);
	printf("Abs_H[%i %i]=%i %i \n",
			a_p32.i16[0], a_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSS\n");
	a_p32.i32 = INT32_MIN;
	res_p32.u32 = Ifx_Abs(a_p32.i32);
	printf("Abs[%i]=%i\n", a_p32.i32, res_p32.i32);
	res_p32.u32 = Ifx_AbsS(a_p32.i32);
	printf("AbsS[%i]=%i\n", a_p32.i32, res_p32.i32);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSS.H\n");
	a_p32.i16[0] = INT16_MIN;
	a_p32.i16[1] = INT16_MIN-1;
	res_p32.u32 = Ifx_Abs_H(a_p32.i32);
	printf("Abs_H[%i %i]=%i %i \n",
			a_p32.i16[0], a_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	res_p32.u32 = Ifx_AbsS_H(a_p32.i32);
	printf("AbsS_H[%i %i]=%i %i \n",
			a_p32.i16[0], a_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSDIF\n");
	a_p32.i32 = INT32_MIN/2;
	b_p32.i32 = INT32_MAX/2;
	res_p32.u32 = Ifx_Absdif(a_p32.i32, b_p32.i32);
	printf("Absdif[%i,%i]=%i\n", a_p32.i32, b_p32.i32, res_p32.i32);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSDIF.B\n");
	a_p32.i8[0] = INT8_MIN/2;
	a_p32.i8[1] = INT8_MIN/3;
	a_p32.i8[2] = INT8_MIN/4;
	a_p32.i8[3] = INT8_MIN/5;
	b_p32.i8[0] = INT8_MAX/2;
	b_p32.i8[1] = INT8_MAX/3;
	b_p32.i8[2] = INT8_MAX/4;
	b_p32.i8[3] = INT8_MAX/5;
	res_p32.u32 = Ifx_Absdif_B(a_p32.i32, b_p32.i32);
	printf("Absdif_B[%i %i %i %i\t%i %i %i %i]=%i %i %i %i\n",
			a_p32.i8[0], a_p32.i8[1], a_p32.i8[2], a_p32.i8[3],
			b_p32.i8[0], b_p32.i8[1], b_p32.i8[2], b_p32.i8[3],
			res_p32.i8[0], res_p32.i8[1], res_p32.i8[2], res_p32.i8[3]);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSDIF.H\n");
	a_p32.i16[0] = INT16_MIN/2;
	a_p32.i16[1] = INT16_MIN/3;
	b_p32.i16[0] = INT16_MAX/2;
	b_p32.i16[1] = INT16_MAX/3;
	res_p32.u32 = Ifx_Absdif_H(a_p32.i32, b_p32.i32);
	printf("Absdif_H[%i %i\t%i %i]=%i %i \n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSDIFS\n");
	a_p32.i32 = INT32_MIN;
	b_p32.i32 = INT32_MAX;
	res_p32.u32 = Ifx_Absdif(a_p32.i32, b_p32.i32);
	printf("Absdif[%i,%i]=%i\n", a_p32.i32, b_p32.i32, res_p32.i32);
	res_p32.u32 = Ifx_AbsdifS(a_p32.i32, b_p32.i32);
	printf("AbsdifS[%i,%i]=%i\n", a_p32.i32, b_p32.i32, res_p32.i32);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	printf("\nTest ABSDIFS.H\n");
	a_p32.i16[0] = INT16_MIN;
	a_p32.i16[1] = INT16_MIN+1;
	b_p32.i16[0] = INT16_MAX;
	b_p32.i16[1] = INT16_MAX-1;
	res_p32.u32 = Ifx_Absdif_H(a_p32.i32, b_p32.i32);
	printf("Absdif_H[%i %i\t%i %i]=%i %i \n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	res_p32.u32 = Ifx_AbsdifS_H(a_p32.i32, b_p32.i32);
	printf("AbsdifS_H[%i %i\t%i %i]=%i %i \n",
			a_p32.i16[0], a_p32.i16[1],
			b_p32.i16[0], b_p32.i16[1],
			res_p32.i16[0], res_p32.i16[1]);
	__asm__ volatile ("nop" ::: "memory");
	__asm volatile ("" : : : "memory");
	/* wait until sending has finished */
	while (_uart_sending())
		;

	g_regular_task_flag = true;
	while(1)
	{
		if(0==g_SysTicks%(20*SYSTIME_CLOCK))
		{
			g_regular_task_flag = true;
		}

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

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

