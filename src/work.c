#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "led.h"
#include "uart_int.h"

#include "tc27xc/IfxStm_reg.h"
#include "tc27xc/IfxStm_bf.h"
#include "tc27xc/IfxCpu_reg.h"
#include "tc27xc/IfxCpu_bf.h"

#include "system_tc2x.h"
#include "machine/intrinsics.h"

#define SPRINTF		sprintf
#define VSPRINTF	vsprintf

#include "interrupts.h"

#define BUFSIZE		512

#define BAUDRATE	115200

#define SYSTIME_CLOCK	1000	/* timer event rate [Hz] */

volatile uint32_t g_SysTicks;
volatile uint8_t test_flag = 0;

/* timer callback handler */
static void my_timer_handler(void)
{
	++g_SysTicks;
	if(0==g_SysTicks%(20*SYSTIME_CLOCK))
	{
		test_flag = 1;
	}
}

const uint32_t HAL_GetTick(void)
{
	return g_SysTicks;
}

static void my_puts(const char *str)
{
	char buffer[BUFSIZE];

	SPRINTF(buffer, "%s\r\n", str);
	_uart_puts(buffer);
}

static void my_printf(const char *fmt, ...)
{
	char buffer[BUFSIZE];
	va_list ap;

	va_start(ap, fmt);
	VSPRINTF(buffer, fmt, ap);
	va_end(ap);

	_uart_puts(buffer);
}

#define SYSTIME_ISR_PRIO	2

#define STM0_BASE			((Ifx_STM *)&MODULE_STM0)

/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value = 0;

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


inline int32_t __add(int32_t a, int32_t b)
{
	int32_t res;
	__asm volatile ("add %0, %1, %2": "=d" (res) : "d" (a), "d" (b));
	return res;
}

inline int32_t __sub(int32_t a, int32_t b)
{
	int32_t res;
	__asm volatile ("sub %0, %1, %2": "=d" (res) : "d" (a), "d" (b));
	return res;
}

inline int32_t __adds(int32_t a, int32_t b)
{
	int32_t res;
	__asm volatile ("adds %0, %1, %2": "=d" (res) : "d" (a), "d" (b));
	return res;
}

inline int32_t __subs(int32_t a, int32_t b)
{
	int32_t res;
	__asm volatile ("subs %0, %1, %2": "=d" (res) : "d" (a), "d" (b));
	return res;
}

/** multiplication signed without saturation
 */
inline uint32_t __mul(int32_t a, int32_t b)
{
	int32_t res;
	__asm volatile ("mul %0, %1, %2": "=d" (res) : "d" (a), "d" (b));
	return res;
}

/** multiplication signed with saturation
 */
inline int32_t __muls(int32_t a, int32_t b)
{
	int32_t res;
	__asm volatile ("muls %0, %1, %2": "=d" (res) : "d" (a), "d" (b));
	return res;
}

uint32_t IfxCpu_getRandomValue(uint32_t *seed)
{
	/*************************************************************************
	 * the choice of a and m is important for a long period of the LCG
	 * with a =  279470273 and
	 *       m = 4294967291
	 * a maximum period of 2^32-5 is given
	 * values for a:
	 * 0x5EB0A82F = 1588635695
	 * 0x48E7211F = 1223106847
	 * 0x10a860c1 =  279470273
	 ***************************************************************************/
	uint32_t x = *seed;

	/* a seed of 0 is not allowed, and therefore will be changed to a valid value */
	if (x == 0)
	{
		x = 42;
	}

	uint32_t a = 0x10a860c1;  // 279470273
	uint32_t m = 0xfffffffb;  // 4294967291
	uint32_t result;

	/* *INDENT-OFF* */
#ifdef __GNUC__
	__asm("      mul.u     %%e14,%1,%2       # d15 = Eh; d14 = El    \n"
			"        mov       %%d12,%%d14       #   e12 = El            \n"
			"        mov       %%d13, 0          #                       \n"
			"        madd.u    %%e14,%%e12,%%d15, 5 # e14 = El + 5 * d15    \n"
			" cmp_m: jge.u     %%d14,%3,sub_m    #                       \n"
			"        jz        %%d15,done        #                       \n"
			" sub_m: subx      %%d14,%%d14,%3    #  e12=e12-m            \n"
			"        subc      %%d15,%%d15,%%d13 # d13=d13-0             \n"
			"        loopu     cmp_m             #                       \n"
			" done:  mov       %0,%%d14          #                       \n"
			: "=d"(result) : "d"(a), "d"(x), "d"(m) : "d12","d13","d14","d15");
#endif
	/* *INDENT-ON* */
	* seed = result; // to simplify seed passing

	return result;
}

#define	TEST_N	10

int32_t testRand[TEST_N];

int16_t Ifx_AbsQ15(int16_t X);

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

	printf("%u %u %u %u %u\n",
			SYSTEM_GetCpuClock(),
			SYSTEM_GetSysClock(),
			SYSTEM_GetStmClock(),
			SYSTEM_GetCanClock(),
			SYSTEM_IsCacheEnabled());

	test_flag = 1;
	while(1)
	{
		if(0!=test_flag)
		{
			test_flag = 0;

			for(uint32_t i=0; i<TEST_N; ++i)
			{
				printf("%08X ", rand());
				int32_t testSeed = i;
				printf("%08X ", IfxCpu_getRandomValue(&testSeed));
				printf("%d %d\n", 0-i, Ifx_AbsQ15(0-i));
			}
			printf("\n");

			//Test Add
			printf("\nTest Add\n");
			{
				int32_t t1 = INT32_MAX;
				int32_t t2 = 3;
				int32_t res_a = __add(t1, t2);
				int32_t res_as = __adds(t1, t2);
				printf("[ADD] %i + %i = %i\n", t1, t2, res_a);
				printf("[ADDS] %i + %i = %i\n", t1, t2, res_as);
			}

			//Test Sub
			printf("\nTest Substract\n");
			{
				int32_t t1 = INT32_MIN;
				int32_t t2 = 3;
				int32_t res_s = __sub(t1, t2);
				int32_t res_ss = __subs(t1, t2);
				printf("[SUB] %i - %i = %i\n", t1, t2, res_s);
				printf("[SUBS] %i - %i = %i\n", t1, t2, res_ss);
			}

			//Test Multiplication
			printf("\nTest Multiplication\n");
			{
				int32_t t1 = INT32_MAX/2;
				int32_t t2 = 3;
				int32_t res_m = __mul(t1, t2);
				int32_t res_ms = __muls(t1, t2);
				printf("[MUL] %i * %i = %i\n", t1, t2, res_m);
				printf("[MULS] %i * %i = %i\n", t1, t2, res_ms);
			}
		}
	}

	return EXIT_SUCCESS;
}

