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
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t res;
	uint64_t res64;

//	printf("\nTest BISR RSLCX\n");
//	Ifx_Bisr(4);
//	Ifx_Rslcx();
//	printf("BISR RSLCX should be combined to avoid CSA leak\n");
//	flush_stdout();

	printf("\nTest BMERGE\n");
	a = 0x000000FF;
	b = 0x0000FF00;
	res = Ifx_Bmerge(a, b);
	printf("BMERGE[%08X, %08X]=%08X\n", a, b, res);
	flush_stdout();

	printf("\nTest BSPLIT\n");
	a = 0xA5A5C9C9;
	res64 = Ifx_Bsplit(a);
	printf("BSPLIT[%08X]=%016llX\n", a, res64);
	flush_stdout();

	printf("\nTest CACHEA.I CACHEA.W CACHEA.WI CACHEI.I CACHEI.W CACHEI.WI\n");
	Ifx_Cachea_I();
	Ifx_Cachea_W();
	Ifx_Cachea_WI();
	Ifx_Cachei_I();
	Ifx_Cachei_W();
	Ifx_Cachei_WI();
	printf("Cache operation end\n");
	flush_stdout();

	printf("\nTest CADD\n");
	a = 0x11111111;
	b = 0x22222222;
	c = 0;
	res =  Ifx_CADD(a, b, c);
	printf("CADD[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	c = 1;
	res =  Ifx_CADD(a, b, c);
	printf("CADD[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CADDI\n");
	a = 0x11111111;
	c = 0;
	res =  Ifx_CADD_I(a, c);
	printf("CADDI[%08X, %08X, %08X]=%08X\n", a, 126, c, res);
	c = 1;
	res =  Ifx_CADD_I(a, c);
	printf("CADDI[%08X, %08X, %08X]=%08X\n", a, 126, c, res);
	flush_stdout();

	printf("\nTest CADDI 16bit\n");
	a = 0x11111111;
	c = 0;
	res =  Ifx_CADD_I(a, c);
	printf("CADDI_16[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	c = 1;
	res =  Ifx_CADD_I16(a, c);
	printf("CADDI_16[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	flush_stdout();

	printf("\nTest CADDN\n");
	a = 0x11111111;
	b = 0x22222222;
	c = 0;
	res =  Ifx_CADDN(a, b, c);
	printf("CADDN[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	c = 1;
	res =  Ifx_CADDN(a, b, c);
	printf("CADDN[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CADDN_I\n");
	a = 0x11111111;
	c = 0;
	res =  Ifx_CADDN_I(a, c);
	printf("CADDN_I[%08X, %08X, %08X]=%08X\n", a, 126, c, res);
	c = 1;
	res =  Ifx_CADDN_I(a, c);
	printf("CADDN_I[%08X, %08X, %08X]=%08X\n", a, 126, c, res);
	flush_stdout();

	printf("\nTest CADDNI 16bit\n");
	a = 0x11111111;
	c = 0;
	res =  Ifx_CADDN_I16(a, c);
	printf("CADDNI_16[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	c = 1;
	res =  Ifx_CADDN_I16(a, c);
	printf("CADDNI_16[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	flush_stdout();

	printf("\nTest CALL\n");
	a = 0x11111111;
	b = 0x22222222;
	c = 0;
	res =  Ifx_Call(a, b, c);
	printf("CADD[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CALL_A\n");
	c = 1;
	res =  Ifx_Call_A(a, b, c);
	printf("CADD[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CALL_I\n");
	c = 0;
	res =  Ifx_Call_I(Ifx_CADD, a, b, c);
	printf("CADD[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CLO(Count Leading Ones)\n");
	a = 0xC0010000;
	res =  Ifx_Clo(a);
	printf("CLO[%08X]=%08X\n", a, res);
	flush_stdout();

	printf("\nTest CLO.H(Count Leading Ones in Packed Half-words)\n");
	a = 0xC0019003;
	res =  Ifx_Clo_H(a);
	printf("CLO.H[%08X]=%08X\n", a, res);
	flush_stdout();

	printf("\nTest CLS(Count Leading Signs)\n");
	a = 0xC0010000;
	res =  Ifx_Cls(a);
	printf("CLS[%08X]=%08X\n", a, res);
	flush_stdout();

	printf("\nTest CLS.H(Count Leading Signs in Packed Half-words)\n");
	a = 0xC0013003;
	res =  Ifx_Cls_H(a);
	printf("CLS.H[%08X]=%08X\n", a, res);
	flush_stdout();

	printf("\nTest CLZ(Count Leading Zeros)\n");
	a = 0x00010000;
	res =  Ifx_Clz(a);
	printf("CLO[%08X]=%08X\n", a, res);
	flush_stdout();

	printf("\nTest CLO.H(Count Leading Zeros in Packed Half-words)\n");
	a = 0x00030008;
	res =  Ifx_Clz_H(a);
	printf("CLO.H[%08X]=%08X\n", a, res);
	flush_stdout();

	printf("\nTest CMOV\n");
	a = 0x11111111;
	b = 0x22222222;
	c = 0;
	res =  Ifx_Cmov(a, b, c);
	printf("CMOV[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	c = 1;
	res =  Ifx_Cmov(a, b, c);
	printf("CMOV[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CMOV_I\n");
	a = 0x11111111;
	c = 0;
	res =  Ifx_Cmov_I(a, c);
	printf("CMOV_I[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	c = 1;
	res =  Ifx_Cmov_I(a, c);
	printf("CMOV_I[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	flush_stdout();

	printf("\nTest CMOVN\n");
	a = 0x11111111;
	b = 0x22222222;
	c = 0;
	res =  Ifx_Cmovn(a, b, c);
	printf("CMOVN[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	c = 1;
	res =  Ifx_Cmovn(a, b, c);
	printf("CMOVN[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CMOVN_I\n");
	a = 0x11111111;
	c = 0;
	res =  Ifx_Cmovn_I(a, c);
	printf("CMOVN_I[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	c = 1;
	res =  Ifx_Cmovn_I(a, c);
	printf("CMOVN_I[%08X, %08X, %08X]=%08X\n", a, 6, c, res);
	flush_stdout();

//	printf("\nTest CMP.F\n");
//	float fA = 1.1;
//	float fB = 2.2;
//	res =  Ifx_Cmp_F(fA, fB);
//	printf("CMP.F[%f, %f]=%08X\n", fA, fB, res);
//	flush_stdout();

	printf("\nTest CSUB\n");
	a = 0x22222222;
	b = 0x11111110;
	c = 0;
	res =  Ifx_Csub(a, b, c);
	printf("CSUB[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	c = 1;
	res =  Ifx_Csub(a, b, c);
	printf("CSUB[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	flush_stdout();

	printf("\nTest CSUBN\n");
	a = 0x22222222;
	b = 0x11111110;
	c = 0;
	res =  Ifx_Csubn(a, b, c);
	printf("CSUBN[%08X, %08X, %08X]=%08X\n", a, b, c, res);
	c = 1;
	res =  Ifx_Csubn(a, b, c);
	printf("CSUBN[%08X, %08X, %08X]=%08X\n", a, b, c, res);
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

