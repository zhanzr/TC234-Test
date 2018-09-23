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
#include "interrupts.h"
#include "led.h"
#include "uart_int.h"

#include "asm_prototype.h"

#include "jansson.h"

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

/* timer reload value (needed for subtick calculation) */
static unsigned int reload_value;

/* pointer to user specified timer callback function */
static TCF user_handler = (TCF)0;

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

void add_2array_to_json( json_t* obj, const char* name, const int*
		marr, size_t dim1, size_t dim2 )
{
	size_t i, j;
	json_t* jarr1 = json_array();

	for( i=0; i<dim1; ++i ) {
		json_t* jarr2 = json_array();

		for( j=0; j<dim2; ++j ) {
			int val = marr[ i*dim2 + j ];
			json_t* jval = json_integer( val );
			json_array_append_new( jarr2, jval );
		}
		json_array_append_new( jarr1, jarr2 );
	}
	json_object_set_new( obj, name, jarr1 );
	return;
}

void test_jansson(void)
{
	json_t* jdata;
	char* s;
//	int arr1[2][3] = { {1,2,3}, {4,5,6} };
	int arr1[2][3];
	int arr2[4][4] = { {1,2,3,4}, {5,6,7,8}, {9,10,11,12}, {13,14,15,16} };

	for(uint32_t i=0; i<2; ++i)
	{
		for(uint32_t j=0; j<3; ++j)
		{
			arr1[i][j] = rand();
		}
	}

	jdata = json_object();

	add_2array_to_json( jdata, "arr1", &arr1[0][0], 2, 3 );
	add_2array_to_json( jdata, "arr2", &arr2[0][0], 4, 4 );

	s = json_dumps( jdata, 0 );

	printf(s);
	flush_stdout();

	free( s );
	json_decref( jdata );
}

int main(void)
{
	SYSTEM_Init();

	SYSTEM_EnaDisCache(1);

	TimerInit(SYSTIME_CLOCK);
	TimerSetHandler(my_timer_handler);

	_init_uart(BAUDRATE);

	InitLED();

	printf("Test Jansson CPU:%u MHz,Sys:%u MHz,STM:%u MHz,CacheEn:%d\n",
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			SYSTEM_IsCacheEnabled());
	flush_stdout();

	printf("\n\n\n\n\n\n\n\n\n\n");
	flush_stdout();

	test_jansson();
	flush_stdout();

	printf("\n\n\n\n\n\n\n\n\n\n");
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

