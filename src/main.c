#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

#include <machine/intrinsics.h>

#include "bspconfig_tc23x.h"

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

#include "partest.h"

#define MESSAGE_Q_NUM   2
QueueHandle_t Message_Queue;

SemaphoreHandle_t BinarySemaphore;

SemaphoreHandle_t CountSemaphore;

SemaphoreHandle_t MutexSemaphore;

TimerHandle_t 	AutoReloadTimer_Handle;
TimerHandle_t	OneShotTimer_Handle;

EventGroupHandle_t EventGroupHandler;
#define EVENTBIT_0	(1<<0)
#define EVENTBIT_1	(1<<1)
#define EVENTBIT_2	(1<<2)
#define EVENTBIT_ALL	(EVENTBIT_0|EVENTBIT_1|EVENTBIT_2)
/*----------------------------------------------------------*/

/* Constants for the ComTest tasks. */
#define mainCOM_TEST_BAUD_RATE		( ( uint32_t ) BAUDRATE )

#define mainCOM_TEST_LED			( 4 )
/* Set mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 1 to create a simple demo.
Set mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY to 0 to create a much more
comprehensive test application.  See the comments at the top of this file, and
the documentation page on the http://www.FreeRTOS.org web site for more
information. */
#define mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY		1

/*-----------------------------------------------------------*/

static void prvSetupHardware( void );

/*-----------------------------------------------------------*/

TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

TaskHandle_t g_task0_handler;
void led0_task(void *pvParameters);

TaskHandle_t g_task1_handler;
void led1_task(void *pvParameters);

TaskHandle_t g_info_task_handler;
void print_task(void *pvParameters);

int core0_main(int argc, char** argv)
{
	volatile bool g_regular_task_flag;

	prvSetupHardware();

//	SYSTEM_EnaDisCache(1);

	uart_init(mainCOM_TEST_BAUD_RATE);

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

	extern void stm_wait(uint32_t us);
	for(uint8_t i=1; i!=10; ++i)
	{
		for(uint32_t j=0; j<1000; ++j)
		{
			stm_wait(1000);
		}
		printf("%s STM Delay Test %u\n", _NEWLIB_VERSION, i);
	}
	_syscall(200);

	g_regular_task_flag = true;

	uint8_t test_trig_cnt = 0;
	/* The following function will only create more tasks and timers if
	mainCREATE_SIMPLE_LED_FLASHER_DEMO_ONLY is set to 0 (at the top of this
	file).  See the comments at the top of this file for more information. */
	//	prvOptionallyCreateComprehensveTestApplication();

	xTaskCreate((TaskFunction_t )start_task,
			(const char*    )"start_task",
			(uint16_t       )512,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 1,
			(TaskHandle_t*  )&StartTask_Handler);

	/* Now all the tasks have been started - start the scheduler. */
	vTaskStartScheduler();

	/* If all is well then the following line will never be reached.  If
	execution does reach here, then it is highly probably that the heap size
	is too small for the idle and/or timer tasks to be created within
	vTaskStartScheduler(). */
	while(1)
	{
#define	DELAY_TICK	(10*configTICK_RATE_HZ)
		if(0==GetFreeRTOSRunTimeTicks()%(DELAY_TICK))
		{
			g_regular_task_flag = true;
		}

		if(g_regular_task_flag)
		{
			g_regular_task_flag = false;

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
		}

		test_proc_dts();
//		test_proc_eru();
//		test_proc_gpsr();
	}

	return EXIT_SUCCESS;
}


/*-----------------------------------------------------------*/
void AutoReloadCallback(TimerHandle_t xTimer)
{
	if(NULL != OneShotTimer_Handle)
	{
		xTimerStart(OneShotTimer_Handle,0);
	}

	if(EventGroupHandler!=NULL)
	{
		xEventGroupSetBits(EventGroupHandler,EVENTBIT_1);
	}

	//	xTaskNotify( g_task0_handler, 1, eSetValueWithOverwrite);
	//	xTaskNotify( g_task1_handler, 2, eSetValueWithOverwrite);
//	xTaskNotify( g_info_task_handler, 3, eSetValueWithOverwrite);
	start_dts_measure();
	vParTestToggleLED(2);
}

void OneShotCallback(TimerHandle_t xTimer)
{
	char info_buf[256];

	vParTestToggleLED(0);
	vParTestToggleLED(1);

	if(EventGroupHandler!=NULL)
	{
		xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);
	}

	if(NULL != BinarySemaphore)
	{
		if(pdTRUE == xSemaphoreTake(BinarySemaphore, portMAX_DELAY))
		{
			vTaskList(info_buf);
			if(NULL != MutexSemaphore)
			{
				if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
				{
					//					printf("%s,TaskList Len:%d\r\n",
					//							pcTaskGetName(NULL),
					//							strlen(info_buf));
					//					printf("%s\r\n",info_buf);

					xSemaphoreGive(MutexSemaphore);
				}
			}

			xSemaphoreGive(BinarySemaphore);
		}
	}
}

void start_task(void *pvParameters)
{
	Message_Queue = xQueueCreate(MESSAGE_Q_NUM, sizeof(uint32_t));

	CountSemaphore = xSemaphoreCreateCounting(2, 2);

	MutexSemaphore = xSemaphoreCreateMutex();

	AutoReloadTimer_Handle=xTimerCreate((const char*		)"AT",
			(TickType_t			)1000 / portTICK_PERIOD_MS,
			(UBaseType_t		)pdTRUE,
			(void*				)1,
			(TimerCallbackFunction_t)AutoReloadCallback);

	OneShotTimer_Handle=xTimerCreate((const char*			)"OT",
			(TickType_t			)500 / portTICK_PERIOD_MS,
			(UBaseType_t			)pdFALSE,
			(void*					)2,
			(TimerCallbackFunction_t)OneShotCallback);

	EventGroupHandler = xEventGroupCreate();

	if(NULL != AutoReloadTimer_Handle)
	{
		xTimerStart(AutoReloadTimer_Handle, 0);
	}

	xTaskCreate((TaskFunction_t )led0_task,
			(const char*    )"led0_task",
			(uint16_t       )768,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 2,
			(TaskHandle_t*  )&g_task0_handler);

	xTaskCreate((TaskFunction_t )led1_task,
			(const char*    )"led1_task",
			(uint16_t       )768,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 2,
			(TaskHandle_t*  )&g_task1_handler);

	xTaskCreate((TaskFunction_t )print_task,
			(const char*    )"print_task",
			(uint16_t       )2048,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 2,
			(TaskHandle_t*  )&g_info_task_handler);
	vTaskDelete(StartTask_Handler);
}

void led0_task(void *pvParameters)
{
	char info_buf[512];
	EventBits_t EventValue;

	while(1)
	{
		//    	vParTestToggleLED(0);
		//		vTaskDelay(4000 / portTICK_PERIOD_MS);
		if(EventGroupHandler!=NULL)
		{
			EventValue = xEventGroupGetBits(EventGroupHandler);
			if(NULL != MutexSemaphore)
			{
				if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
				{
					//					printf("<%s,ev:%08X\n",
					//							pcTaskGetName(NULL),
					//							(uint32_t)EventValue);
					xSemaphoreGive(MutexSemaphore);
				}
			}

			EventValue = xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,
					(EventBits_t			)EVENTBIT_0,
					(BaseType_t			)pdTRUE,
					(BaseType_t			)pdTRUE,
					(TickType_t			)portMAX_DELAY);

			EventValue = xEventGroupGetBits(EventGroupHandler);
			if(NULL != MutexSemaphore)
			{
				if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
				{
					//					printf(">%s,ev:%08X\n",
					//							pcTaskGetName(NULL),
					//							(uint32_t)EventValue);
					xSemaphoreGive(MutexSemaphore);
				}
			}
		}

		uint32_t NotifyValue=ulTaskNotifyTake( pdTRUE, /* Clear the notification value on exit. */
				portMAX_DELAY );/* Block indefinitely. */

		vTaskList(info_buf);
		if(NULL != MutexSemaphore)
		{
			if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
			{
				printf("%s,TaskList Len:%d, %08X\r\n",
						pcTaskGetName(NULL),
						strlen(info_buf),
						NotifyValue);
				printf("%s\r\n",info_buf);

				xSemaphoreGive(MutexSemaphore);
			}
		}
	}
}

void led1_task(void *pvParameters)
{
	char info_buf[512];
	EventBits_t EventValue;

	while(1)
	{
		//    	vParTestToggleLED(1);
		//		vTaskDelay(4000 / portTICK_PERIOD_MS);
		if(EventGroupHandler!=NULL)
		{
			EventValue = xEventGroupGetBits(EventGroupHandler);
			if(NULL != MutexSemaphore)
			{
				if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
				{
					//					printf("<%s,ev:%08X\n",
					//							pcTaskGetName(NULL),
					//							(uint32_t)EventValue);
					xSemaphoreGive(MutexSemaphore);
				}
			}

			EventValue = xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,
					(EventBits_t			)EVENTBIT_1,
					(BaseType_t			)pdTRUE,
					(BaseType_t			)pdTRUE,
					(TickType_t			)portMAX_DELAY);

			EventValue = xEventGroupGetBits(EventGroupHandler);
			if(NULL != MutexSemaphore)
			{
				if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
				{
					//					printf(">%s,ev:%08X\n",
					//							pcTaskGetName(NULL),
					//							(uint32_t)EventValue);
					xSemaphoreGive(MutexSemaphore);
				}
			}
		}

		if(NULL != Message_Queue)
		{
			uint32_t tmpTicks = xTaskGetTickCount();
			xQueueSend(Message_Queue, &tmpTicks, 0);
		}

		uint32_t NotifyValue=ulTaskNotifyTake( pdTRUE, /* Clear the notification value on exit. */
				portMAX_DELAY );/* Block indefinitely. */

		vTaskList(info_buf);
		if(NULL != MutexSemaphore)
		{
			if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
			{
				printf("%s,TaskList Len:%d, %08X\r\n",
						pcTaskGetName(NULL),
						strlen(info_buf),
						NotifyValue);
				printf("%s\r\n",info_buf);

				xSemaphoreGive(MutexSemaphore);
			}
		}


	}
}

void print_task(void *pvParameters)
{
	char info_buf[512];

	while(1)
	{
		//		vTaskDelay(4000 / portTICK_PERIOD_MS);
		//		if(NULL != Message_Queue)
		//		{
		//			uint32_t tmpU32;
		//			if(xQueueReceive(Message_Queue, &tmpU32, portMAX_DELAY))
		//			{
		//				mutex_printf("%08X\n", (uint32_t)&Message_Queue);
		//				mutex_printf("CPU:%u Hz %u %u\n",
		//						get_cpu_frequency(),
		//						xTaskGetTickCount(),
		//						tmpU32
		//				);
		//			}
		//		}
		//		else
		//		{
		//			mutex_printf("%08X\n", (uint32_t)&Message_Queue);
		//		    vTaskDelay(1000 / portTICK_PERIOD_MS);
		//		}

		printf("%s %s\n", _NEWLIB_VERSION, __func__);

		float fA = 1.1;
		float fB = 3.3;
		float f_res = Ifx_Add_F(fA, fB);
		printf("%f + %f = %f\n", fA, fB, f_res);
		flush_stdout();

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

		uint32_t NotifyValue = ulTaskNotifyTake( pdTRUE, /* Clear the notification value on exit. */
				portMAX_DELAY );/* Block indefinitely. */
		printf("DTS Triggered %.3f P15.4:%u\n",
				read_dts_celsius(),
				MODULE_P15.IN.B.P4);
		flush_stdout();

		vTaskList(info_buf);
		if(NULL != MutexSemaphore)
		{
			if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY))
			{
				printf("%s,TaskList Len:%d, %08X\r\n",
						pcTaskGetName(NULL),
						strlen(info_buf),
						NotifyValue);
				printf("%s\r\n",info_buf);

				vTaskGetRunTimeStats(info_buf);
				printf("RunTimeStats Len:%d\r\n", strlen(info_buf));
				printf("%s\r\n",info_buf);

				//				uint8_t* buffer;
				//				uint32_t tmpA, tmpB, tmpC, tmpD;
				//				tmpA = xPortGetFreeHeapSize();
				//				buffer = pvPortMalloc(1024);
				//				tmpB = xPortGetFreeHeapSize();
				//				if(buffer!=NULL)
				//				{
				//					vPortFree(buffer);
				//					tmpC = xPortGetFreeHeapSize();
				//					buffer = NULL;
				//				}
				//				printf("FreeMem3:\t %u %u %u\n", tmpA, tmpB, tmpC);
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

				xSemaphoreGive(MutexSemaphore);
			}
		}

		//        if(NULL != CountSemaphore)
		//        {
		//        	if(pdTRUE == xSemaphoreTake(CountSemaphore, portMAX_DELAY))
		//        	{
		//        		vTaskList(info_buf);
		//        		printf("TaskList Len:%d\r\n", strlen(info_buf));
		//        		printf("%s\r\n",info_buf);
		//
		//        		vTaskGetRunTimeStats(info_buf);
		//        		printf("RunTimeStats Len:%d\r\n", strlen(info_buf));
		//        		printf("%s\r\n",info_buf);
		//
		//        		uint32_t tmp_sema_cnt = uxSemaphoreGetCount(CountSemaphore);
		//        		printf("SemaCnt %08X: %d\n", (uint32_t)&CountSemaphore, tmp_sema_cnt);
		//                xSemaphoreGive(CountSemaphore);
		//                tmp_sema_cnt = uxSemaphoreGetCount(CountSemaphore);
		//        		printf("SemaCnt %08X: %d\n", (uint32_t)&CountSemaphore, tmp_sema_cnt);
		//        	}
		//        }
	}
}

static void prvSetupHardware( void )
{
	system_clk_config_200_100();

	/* activate interrupt system */
	InterruptInit();

	SYSTEM_Init();

	/* Initialize LED outputs. */
	vParTestInitialise();
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
