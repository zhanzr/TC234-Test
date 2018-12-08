#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

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
#include TC_INCLUDE(TCPATH/IfxQspi_reg.h)
#include TC_INCLUDE(TCPATH/IfxQspi_bf.h)
#include TC_INCLUDE(TCPATH/IfxEth_reg.h)
#include TC_INCLUDE(TCPATH/IfxEth_bf.h)

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

#include "md5.h"

const uint32_t test_content[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

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

void test_tlf35584(void)
{
	tlf35584_cmd_t tmp_tlf_cmd;
	tlf35584_cmd_t res_tlf_cmd;

	for(uint8_t i=0; i<0x34; ++i)
	{
		tmp_tlf_cmd.B.cmd = 0;
		tmp_tlf_cmd.B.addr = i;
		tmp_tlf_cmd.B.data = 0;
		tmp_tlf_cmd.B.parity = 0;
		pack32 z_parity;
		z_parity.u32 = Ifx_PARITY((uint32_t)tmp_tlf_cmd.U);
		tmp_tlf_cmd.B.parity = z_parity.u8[0] ^ z_parity.u8[1];

		QSPI2_DATAENTRY0.B.E = (uint32_t)tmp_tlf_cmd.U;
		/* wait until transfer is complete */
		while (!QSPI2_STATUS.B.TXF)
			;
		/* clear TX flag */
		QSPI2_FLAGSCLEAR.B.TXC = 1;
		/* wait for receive is finished */
		while (!QSPI2_STATUS.B.RXF)
			;
		/* clear RX flag */
		QSPI2_FLAGSCLEAR.B.RXC = 1;

		//read
		res_tlf_cmd.U = QSPI2_RXEXIT.U;
		printf("%04X [%02X]=%02X\n",
				res_tlf_cmd.U,
				tmp_tlf_cmd.B.addr,
				res_tlf_cmd.B.data
		);
		flush_stdout();
	}

	{
		tmp_tlf_cmd.B.cmd = 0;
		tmp_tlf_cmd.B.addr = 0x3f;
		tmp_tlf_cmd.B.data = 0;
		tmp_tlf_cmd.B.parity = 0;
		pack32 z_parity;
		z_parity.u32 = Ifx_PARITY((uint32_t)tmp_tlf_cmd.U);
		tmp_tlf_cmd.B.parity = z_parity.u8[0] ^ z_parity.u8[1];

		QSPI2_DATAENTRY0.B.E = (uint32_t)tmp_tlf_cmd.U;
		/* wait until transfer is complete */
		while (!QSPI2_STATUS.B.TXF)
			;
		/* clear TX flag */
		QSPI2_FLAGSCLEAR.B.TXC = 1;
		/* wait for receive is finished */
		while (!QSPI2_STATUS.B.RXF)
			;
		/* clear RX flag */
		QSPI2_FLAGSCLEAR.B.RXC = 1;
		//read
		res_tlf_cmd.U = QSPI2_RXEXIT.U;
		printf("%04X [%02X]=%02X\n",
				res_tlf_cmd.U,
				tmp_tlf_cmd.B.addr,
				res_tlf_cmd.B.data
		);
		flush_stdout();
	}
}

int core0_main(int argc, char** argv)
{
	prvSetupHardware();

	//	SYSTEM_EnaDisCache(1);

	uart_init(mainCOM_TEST_BAUD_RATE);

	config_dts();
	//	config_gpsr();
	//	config_eru();
	//	config_dflash();
	//	enable_performance_cnt();

	printf("%s %s\n", _NEWLIB_VERSION, __func__);

	const uint32_t FLASH_SIZE_TABLE_KB[]={256, 512, 1024, 1536, 2048, 2560, 3072, 4096, 5120, 1024*6, 1024*7, 1024*8};
	printf("CHIPID:%X\n"\
			"%c step\n" \
			"AurixGen:%u\n"\
			"EmuDevice?%s\n" \
			"Flash SIZE:%u KB\n"\
			"HSM:%s\n"\
			"SpeedGrade:%X\n"\
			"FlashVer:%X\n",
			MODULE_SCU.CHIPID.B.CHID,
			(MODULE_SCU.CHIPID.B.CHREV/0x10)+'A',
			MODULE_SCU.CHIPID.B.CHTEC,
			(MODULE_SCU.CHIPID.B.EEA==1)?"Yes":"No",
					FLASH_SIZE_TABLE_KB[MODULE_SCU.CHIPID.B.FSIZE%0x0c],
					(MODULE_SCU.CHIPID.B.SEC==1)?"Yes":"No",
							MODULE_SCU.CHIPID.B.SP,
							MODULE_SCU.CHIPID.B.UCODE
	);
	flush_stdout();

	printf("DEPT:%s MANUF:%s\n",
			(0==MODULE_SCU.MANID.B.DEPT)?"ATV Microcontroller department":"Unknown",
					(0xc1==MODULE_SCU.MANID.B.MANUF)?"Infineon Technologies":"Unknown"
	);
	flush_stdout();

	printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,PLL:%u M,CE:%d\n",
			__TRICORE_NAME__,
			__TRICORE_CORE__,
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			SYSTEM_GetStmClock()/1000000,
			system_GetPllClock()/1000000,
			SYSTEM_IsCacheEnabled());
	flush_stdout();

	Ifx_TestLED(3);

	extern void stm_wait(uint32_t us);
	for(uint8_t i=0; i<4; ++i)
	{
		for(uint32_t j=0; j<1000; ++j)
		{
			stm_wait(500);
		}
		printf("%s STM Test Delay %u\n", _NEWLIB_VERSION, i);
	}
	_syscall(200);

	test_tlf35584();

	//	test_dma_crc();

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
		_nop();
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
//		test_tlf35584();

		const uint32_t FLASH_SIZE_TABLE_KB[]={256, 512, 1024, 1536, 2048, 2560, 3072, 4096, 5120, 1024*6, 1024*7, 1024*8};
		printf("CHIPID:%X\n"\
				"%c step\n" \
				"AurixGen:%u\n"\
				"EmuDevice?%s\n" \
				"Flash SIZE:%u KB\n"\
				"HSM:%s\n"\
				"SpeedGrade:%X\n"\
				"FlashVer:%X\n",
				MODULE_SCU.CHIPID.B.CHID,
				(MODULE_SCU.CHIPID.B.CHREV/0x10)+'A',
				MODULE_SCU.CHIPID.B.CHTEC,
				(MODULE_SCU.CHIPID.B.EEA==1)?"Yes":"No",
						FLASH_SIZE_TABLE_KB[MODULE_SCU.CHIPID.B.FSIZE%0x0c],
						(MODULE_SCU.CHIPID.B.SEC==1)?"Yes":"No",
								MODULE_SCU.CHIPID.B.SP,
								MODULE_SCU.CHIPID.B.UCODE
		);
		flush_stdout();

		printf("DEPT:%s MANUF:%s\n",
				(0==MODULE_SCU.MANID.B.DEPT)?"ATV Microcontroller department":"Unknown",
						(0xc1==MODULE_SCU.MANID.B.MANUF)?"Infineon Technologies":"Unknown"
		);
		flush_stdout();

		Ifx_CPU_DCON2 tmp_dcon2;
		tmp_dcon2.U = _mfcr( CPU_DCON2 );
		printf("Data Cache Size:%u KB\n"\
				"Data Scratch Size:%u KB\n",
				tmp_dcon2.B.DCACHE_SZE,
				tmp_dcon2.B.DSCRATCH_SZE);
		flush_stdout();

		SYSTEM_EnaDisCache(1);

//		tmp_dcon2.U = _mfcr( CPU_DCON2 );
//		printf("Data Cache Size:%u KB\n"\
//				"Data Scratch Size:%u KB\n",
//				tmp_dcon2.B.DCACHE_SZE,
//				tmp_dcon2.B.DSCRATCH_SZE);
//		flush_stdout();
//
//		SYSTEM_EnaDisCache(0);

		float fA = M_PI;
		float fB = M_E;
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

		//Test The MDT digest
		uint8_t md5_result[16]={0};
		md5((uint8_t*)test_content, sizeof(test_content), md5_result);
		printf("\nThe Result:\n");
		for(uint32_t i=0; i<sizeof(md5_result); ++i)
		{
			printf("%02x", md5_result[i]);
		}
		printf("\n");
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
