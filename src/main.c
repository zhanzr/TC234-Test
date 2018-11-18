#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include <machine/intrinsics.h>

#include "bspconfig_tc23x.h"

#include TC_INCLUDE(TCPATH/IfxStm_reg.h)
#include TC_INCLUDE(TCPATH/IfxStm_bf.h)
#include TC_INCLUDE(TCPATH/IfxCpu_reg.h)
#include TC_INCLUDE(TCPATH/IfxCpu_bf.h)
#include TC_INCLUDE(TCPATH/IfxEth_reg.h)
#include TC_INCLUDE(TCPATH/IfxEth_bf.h)
#include TC_INCLUDE(TCPATH/IfxScu_reg.h)
#include TC_INCLUDE(TCPATH/IfxScu_bf.h)
#include TC_INCLUDE(TCPATH/IfxInt_reg.h)
#include TC_INCLUDE(TCPATH/IfxInt_bf.h)
#include TC_INCLUDE(TCPATH/IfxSrc_reg.h)
#include TC_INCLUDE(TCPATH/IfxSrc_bf.h)

#include "core_tc23x.h"
#include "system_tc2x.h"
#include "cint_trap_tc23x.h"
#include "interrupts_tc23x.h"
#include "led.h"
#include "uart_int.h"
#include "asm_prototype.h"
#include "dts.h"
#include "timer.h"
#include "gpsr.h"
#include "scu_eru.h"
#include "data_flash.h"

/* Set mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 1 to create a simple demo.
Set mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 0 to create a much more
comprehensive test application.  See the comments at the top of this file, and
the documentation page on the http://www.FreeRTOS.org web site for more
information. */
#define mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY		1

/*-----------------------------------------------------------*/
void enable_performance_cnt(void)
{
	unlock_wdtcon();
	{
		Ifx_CPU_CCTRL tmpCCTRL;
		tmpCCTRL.U = _mfcr(CPU_CCTRL);

		//Instruction Cache Hit Count
		//		tmpCCTRL.bits.M1 = 2;
		//Data Cache Hit Count.
		tmpCCTRL.B.M1 = 3;
		//Instruction Cache Miss Count

		//		tmpCCTRL.bits.M2 = 1;
		//Data Cache Clean Miss Count
		tmpCCTRL.B.M2 = 3;

		//Data Cache Dirty Miss Count
		tmpCCTRL.B.M3 = 3;

		//Normal Mode
		tmpCCTRL.B.CM = 0;
		//Task Mode
		//		tmpCCTRL.bits.CM = 1;

		tmpCCTRL.B.CE = 1;

		_mtcr(CPU_CCTRL, tmpCCTRL.U);
	}
	lock_wdtcon();
}

int core0_main(int argc, char** argv)
{
	volatile bool g_regular_task_flag;

	system_clk_config_200_100();

	/* activate interrupt system */
	InterruptInit();

	SYSTEM_Init();
	SYSTEM_EnaDisCache(1);

	/* initialise STM CMP 0 at configTICK_RATE_HZ rate */
//	prvSetupTimerInterrupt();
	ConfigureTimeForRunTimeStats();

	uart_init(BAUDRATE);

	led_init();

	config_dts();
//	config_gpsr();
//	config_eru();
//	config_dflash();
//	enable_performance_cnt();

	printf("%s %s\n", _NEWLIB_VERSION, __func__);

	printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,PLL:%u M,Int:%u M,CE:%d\n",
			__TRICORE_NAME__,
			__TRICORE_CORE__,
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			system_GetPllClock()/1000000,
			system_GetIntClock()/1000000,
			SYSTEM_IsCacheEnabled());
	flush_stdout();

	printf("%u %u %u\n", CMP1_MATCH_VAL, configPERIPHERAL_CLOCK_HZ, configRUN_TIME_STATS_RATE_HZ);
	flush_stdout();
	flush_stdout();

	_syscall(3);
	_syscall(4);
	_syscall(255);

	g_regular_task_flag = true;

	uint8_t test_trig_cnt = 0;
	while(1)
	{
#define	DELAY_TICK	(10*configTICK_RATE_HZ)
		if(0==GetFreeRTOSRunTimeTicks()%(DELAY_TICK))
		{
			g_regular_task_flag = true;
		}

		uint32_t test_trigger_cnt = GetFreeRTOSRunTimeTicks()/(DELAY_TICK);

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

			if(0==test_trigger_cnt%4)
			{
				led_on(0);
				led_off(1);
				led_off(2);
			}
			else if(1==test_trigger_cnt%4)
			{
				led_off(0);
				led_on(1);
				led_off(2);
			}
			else if(2==test_trigger_cnt%4)
			{
				led_off(0);
				led_off(1);
				led_on(2);
			}
			else
			{
				led_toggle(3);
			}

			printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,PLL:%u M,Int:%u M,CE:%d\n",
					__TRICORE_NAME__,
					__TRICORE_CORE__,
					SYSTEM_GetCpuClock()/1000000,
					SYSTEM_GetSysClock()/1000000,
					SYSTEM_GetStmClock()/1000000,
					system_GetPllClock()/1000000,
					system_GetIntClock()/1000000,
					SYSTEM_IsCacheEnabled());
			flush_stdout();

			start_dts_measure();

			printf("%u\n", GetFreeRTOSRunTimeTicks());
			flush_stdout();

//			printf("OSCON:%08X PCON0:%08X PCON1:%08X PCON2:%08X CCON0:%08X CCON1:%08X CCON2:%08X CCON3:%08X CCON4:%08X CCON5%08X CCON6%08X CCON9:%08X\n",
//					MODULE_SCU.OSCCON.U,
//					MODULE_SCU.PLLCON0.U,
//					MODULE_SCU.PLLCON1.U,
//					MODULE_SCU.PLLCON2.U,
//					MODULE_SCU.CCUCON0.U,
//					MODULE_SCU.CCUCON1.U,
//					MODULE_SCU.CCUCON2.U,
//					MODULE_SCU.CCUCON3.U,
//					MODULE_SCU.CCUCON4.U,
//					MODULE_SCU.CCUCON5.U,
//					MODULE_SCU.CCUCON6.U,
//					MODULE_SCU.CCUCON9.U
//			);
//			flush_stdout();

			printf("%08X %08X %08X %08X %08X %08X    %08X[%08X] %08X[%08X]\n",
					MODULE_STM0.TIM0.U,
					MODULE_STM0.CMP[0].U,
					MODULE_STM0.CMP[1].U,
					MODULE_STM0.CMCON.U,
					MODULE_STM0.ICR.U,
					MODULE_STM0.ISCR.U,

					MODULE_SRC.STM.STM[0].SR0.U,
					(uint32_t)&MODULE_SRC.STM.STM[0].SR0.U,
					MODULE_SRC.STM.STM[0].SR1.U,
					(uint32_t)&MODULE_SRC.STM.STM[0].SR1.U
			);
			flush_stdout();

			test_trig_cnt = test_trigger_gpsr(test_trig_cnt);
		}

		test_proc_dts();
//		test_proc_eru();
//		test_proc_gpsr();
	}

	return EXIT_SUCCESS;
}


static void prvSetupHardware( void )
{
//	extern void set_cpu_frequency(void);
//
//	/* Set-up the PLL. */
//	set_cpu_frequency();
//
//	/* Initialise LED outputs. */
//	vParTestInitialise();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
#if mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY != 1
	{
		/* vApplicationTickHook() will only be called if configUSE_TICK_HOOK is set
		to 1 in FreeRTOSConfig.h.  It is a hook function that will get called during
		each FreeRTOS tick interrupt.  Note that vApplicationTickHook() is called
		from an interrupt context. */

		/* Call the periodic timer test, which tests the timer API functions that
		can be called from an ISR. */
		vTimerPeriodicISRTests();
	}
#endif /* mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY */
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/
