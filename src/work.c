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

/* AppKit-TC2X4: P13.0 .. P13.3 --> LED D107 ... D110 */
static Ifx_P * const portLED = (Ifx_P *)&MODULE_P13;

/* OMR is WO ==> don't use load-modify-store access! */
/* set PSx pin */
#define LED_PIN_SET(x)			(1 << (LED_PIN_NR + (x)))
/* set PCLx pin */
#define LED_PIN_RESET(x)		(1 << (LED_PIN_NR + IFX_P_OMR_PCL0_OFF + (x)))

void LEDON(int nr)
{
	if (nr < MAX_LED)
	{
		portLED->OMR.U = LED_PIN_RESET(nr);
	}
}

void LEDOFF(int nr)
{
	if (nr < MAX_LED)
	{
		portLED->OMR.U = LED_PIN_SET(nr);
	}
}

void LEDTOGGLE(int nr)
{
	if (nr < MAX_LED)
	{
		/* set PCLx and PSx pin to 1 ==> toggle pin state */
		portLED->OMR.U = LED_PIN_RESET(nr) | LED_PIN_SET(nr);
	}
}

void InitLED(void)
{
	/* initialise all LEDs (P13.0 .. P13.3) */
	portLED->IOCR0.U = 0x80808080;	/* OUT_PPGPIO */
	/* all LEDs OFF */
	portLED->OMR.U = (MASK_ALL_LEDS << LED_PIN_NR);
}

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_SysTicks;

	if(0==g_SysTicks%(2*SYSTIME_CLOCK))
	{
		LEDTOGGLE(1);
	}
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

	flush_stdout();

	//Test Additon Instruction
	printf("\nTest ADD\n");
	int32_t a = 0x11112222;
	int32_t b = 0x33334444;
	int32_t res = Ifx_Add(a, b);
	printf("ADD[%08X+%08X]=%08X\n", a, b, res);
	flush_stdout();

	printf("\nTest ADDC\n");
	a = 2;
	b = 3;
	res = Ifx_AddC(a,b);
	printf("ADDC[%08X+%08X]=%08X\n", a, b, res);
	flush_stdout();

	printf("\nTest ADD\n");
	a = 33;
	res = Ifx_AddI(a);
	printf("ADD[%i+ %i]=%i\n", a, -10, res);
	flush_stdout();

	printf("\nTest ADDIH\n");
	a = 0x33;
	res = Ifx_AddI_Hi(a);
	printf("ADDIH[%08X+%08X]=%08X\n", a, 4, res);
	flush_stdout();

	printf("\nTest ADDX\n");
	a = 0x11112222;
	b = 0xfff;
	res = Ifx_Addx(a, b);
	printf("ADDX[%08X+%08X]=%08X\n", a, b, res);
	flush_stdout();

	printf("\nTest ADDX Immediate\n");
	a = 0x33;
	res = Ifx_Addx_I(a);
	printf("ADDX[%i+%i]=%i\n", a, -10, res);
	flush_stdout();

	printf("\nTest ADDS\n");
	a = INT32_MAX;
	b = INT32_MAX;
	res = Ifx_Add(a, b);
	printf("ADD[%i+%i]=%i\n", a, b, res);
	res = Ifx_AddS(a, b);
	printf("ADDS[%i+%i]=%i\n", a, b, res);
	flush_stdout();

	printf("\nTest ADDS.U\n");
	uint32_t c = UINT32_MAX;
	uint32_t d = UINT32_MAX;
	uint32_t resU = Ifx_AddS_U(c, d);
	printf("ADDS.U[%u+%u]=%u\n", c, d, resU);
	flush_stdout();

	printf("\nTest ADDS.H\n");
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
	flush_stdout();

	printf("\nTest ADDS.HU\n");
	a_p32.u16[0]=UINT16_MAX;
	a_p32.u16[1]=UINT16_MAX;
	b_p32.u16[0]=UINT16_MAX;
	b_p32.u16[1]=UINT16_MAX;
	res_p32.u32 = Ifx_AddS_HU(a_p32.u32, b_p32.u32);
	printf("ADDS.HU[%u,%u + %u,%u]=%u,%u\n",
			a_p32.u16[0], a_p32.u16[1],
			b_p32.u16[0], b_p32.u16[1],
			res_p32.u16[0], res_p32.u16[1]);
	flush_stdout();

	printf("\nTest ADDA\n");
	uint8_t tmpArr[16];
	uint8_t* pTest = Ifx_AddA(tmpArr, tmpArr);
	printf("ADDA[%08X,%08X]=%08X\n", (uint32_t)tmpArr, (uint32_t)tmpArr, (uint32_t)pTest);
	flush_stdout();

	printf("\nTest ADDA Immediate\n");
	pTest = Ifx_AddA_4(tmpArr);
	printf("ADDA[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);
	flush_stdout();

	printf("\nTest ADDIH.A\n");
	pTest = Ifx_Addih_A(tmpArr);
	printf("ADDIH.A[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);
	flush_stdout();

	printf("\nTest ADDSC.A\n");
	pTest = Ifx_Addsc_A(tmpArr, 4);
	printf("ADDSC.A[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 4, (uint32_t)pTest);
	flush_stdout();

	printf("\nTest ADDSC.AT\n");
	pTest = Ifx_Addsc_AT(tmpArr, 0x12345678);
	printf("ADDSC.AT[%08X,%08X]=%08X\n", (uint32_t)tmpArr, 0x12345678, (uint32_t)pTest);
	flush_stdout();

	printf("\nTest ADD.B\n");
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
	flush_stdout();

	printf("\nTest ADD.F\n");
	flush_stdout();
	float fA = 1.0f;
	float fB = 2.0f;
	float res_f = Ifx_Add_F(fA, fB);
	printf("ADD.F(%f,%f)=%f\n", fA, fB, res_f);
	flush_stdout();

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

