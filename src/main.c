#include <machine/intrinsics.h>
#include "bspconfig_tc23x.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

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

#define MESSAGE_Q_NUM   2
QueueHandle_t Message_Queue;

SemaphoreHandle_t BinarySemaphore;

SemaphoreHandle_t CountSemaphore;

SemaphoreHandle_t MutexSemaphore;
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
void maintaince_task(void *pvParameters);

TaskHandle_t g_info_task_handler;
void print_task(void *pvParameters);

void test_tlf35584(void) {
	tlf35584_cmd_t tmp_tlf_cmd;
	tlf35584_cmd_t res_tlf_cmd;

	for(uint8_t i=0; i<0x34; ++i) {
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

extern void interface_init(void);
extern void protocol_init(void);
extern void server_loop(void);

int core0_main(int argc, char** argv) {
	prvSetupHardware();

	//	SYSTEM_EnaDisCache(1);

	uart_init(mainCOM_TEST_BAUD_RATE);

	config_dts();

	printf("%s %s\n", _NEWLIB_VERSION, __func__);

	const uint32_t FLASH_SIZE_TABLE_KB[]={256, 512, 1024, 1536, 2048, 2560, 3072, 4096, 5120, 1024*6, 1024*7, 1024*8};
	printf("CHIPID:%X\n"\
			"%c step\n" \
			"AurixGen:%u\n"\
			"EmuDevice?%s\n" \
			"Flash SIZE:%u KB\n"\
			"HSM:%s\n"\
			"SpeedGrade:%X\n",
			MODULE_SCU.CHIPID.B.CHID,
			(MODULE_SCU.CHIPID.B.CHREV/0x10)+'A',
			MODULE_SCU.CHIPID.B.CHTEC,
			(MODULE_SCU.CHIPID.B.EEA==1)?"Yes":"No",
					FLASH_SIZE_TABLE_KB[MODULE_SCU.CHIPID.B.FSIZE%0x0c],
					(MODULE_SCU.CHIPID.B.SEC==1)?"Yes":"No",
							MODULE_SCU.CHIPID.B.SP
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

	_syscall(200);

//	test_tlf35584();

	interface_init();
	protocol_init();

	server_loop();

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
	while(1) {
		_nop();
	}

	return EXIT_SUCCESS;
}

void start_task(void *pvParameters)
{
	Message_Queue = xQueueCreate(MESSAGE_Q_NUM, sizeof(uint32_t));

	CountSemaphore = xSemaphoreCreateCounting(2, 2);

	MutexSemaphore = xSemaphoreCreateMutex();

	xTaskCreate((TaskFunction_t )maintaince_task,
			(const char*    )"maintaince_task",
			(uint16_t       )768,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 2,
			(TaskHandle_t*  )&g_task0_handler);

	xTaskCreate((TaskFunction_t )print_task,
			(const char*    )"print_task",
			(uint16_t       )2048,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 2,
			(TaskHandle_t*  )&g_info_task_handler);
	vTaskDelete(StartTask_Handler);
}

void maintaince_task(void *pvParameters) {
	char info_buf[512];
	EventBits_t EventValue;

	while(1) {
		vTaskList(info_buf);
		if(NULL != MutexSemaphore) {
			if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY)) {

				printf("%s\r\n",info_buf);

				xSemaphoreGive(MutexSemaphore);
			}
		}

		vTaskDelay(4000 / portTICK_PERIOD_MS);
	}
}

void print_task(void *pvParameters) {
	char info_buf[512];

	while(1) {
		vTaskList(info_buf);
		if(NULL != MutexSemaphore) {
			if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY)) {
				printf("%s\r\n",info_buf);

				vTaskGetRunTimeStats(info_buf);
				printf("RunTimeStats Len:%d\r\n", strlen(info_buf));
				printf("%s\r\n",info_buf);

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

		vTaskDelay(4000 / portTICK_PERIOD_MS);
	}
}

static void prvSetupHardware( void ) {
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

void vApplicationTickHook( void ) {
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

void vApplicationIdleHook( void ) {
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

