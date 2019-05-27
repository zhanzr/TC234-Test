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

#include "enc28j60.h"
#include "ip_arp_udp_tcp.h"
#include "net.h"

#define MESSAGE_Q_NUM   2
QueueHandle_t Message_Queue;

SemaphoreHandle_t BinarySemaphore;

SemaphoreHandle_t CountSemaphore;

SemaphoreHandle_t MutexSemaphore;
/*----------------------------------------------------------*/

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
extern void server_loop(void);

extern const uint8_t g_ip_addr[4];
extern const uint8_t g_mac_addr[6];

const char ON_STR[] = "On";
const char OFF_STR[] = "Off";

const char ON_0_STR[] = "On0";
const char OFF_0_STR[] = "Off0";
const char ON_1_STR[] = "On1";
const char OFF_1_STR[] = "Off1";
const char ON_2_STR[] = "On2";
const char OFF_2_STR[] = "Off2";
const char ON_3_STR[] = "On3";
const char OFF_3_STR[] = "Off3";

int16_t analyse_get_url(char *str) {
	char* p_str = str;

	if (0==strncmp(p_str, ON_0_STR, strlen(ON_0_STR))) {
		return CMD_LD0_ON;
	} else if (0==strncmp(p_str, OFF_0_STR, strlen(OFF_0_STR))) {
		return CMD_LD0_OFF;
	} else if (0==strncmp(p_str, ON_1_STR, strlen(ON_1_STR))) {
		return CMD_LD1_ON;
	} else if (0==strncmp(p_str, OFF_1_STR, strlen(OFF_1_STR))) {
		return CMD_LD1_OFF;
	} else if (0==strncmp(p_str, ON_2_STR, strlen(ON_2_STR))) {
		return CMD_LD2_ON;
	} else if (0==strncmp(p_str, OFF_2_STR, strlen(OFF_2_STR))) {
		return CMD_LD2_OFF;
	} else if (0==strncmp(p_str, ON_3_STR, strlen(ON_3_STR))) {
		return CMD_LD3_ON;
	} else if (0==strncmp(p_str, OFF_3_STR, strlen(OFF_3_STR))) {
		return CMD_LD3_OFF;
	} else {
		return(-1);
	}
}

uint16_t prepare_page(uint8_t *buf) {
	uint16_t plen;
	uint8_t tmp_compose_buf[512];

	plen=fill_tcp_data_p(buf,0,("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));

	plen=fill_tcp_data_p(buf,plen,("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n"));

	sprintf((char*)tmp_compose_buf, "<title>Server[%s]</title>\r\n", __TIME__);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);

	plen=fill_tcp_data_p(buf,plen,("<body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("<center>\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>DieTempSensor: %f 'C\r\n", read_dts_celsius());
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("<p>Server on TC234 Appkit board\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>cpu: %u M, Sys:%u M, STM0.TIM1:%08X, soft_spi:%u, newlib:%s\r\n",
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			MODULE_STM0.TIM1.U,
			SOFT_SPI,
			_NEWLIB_VERSION);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);

	//LED 0 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 0 Status:"));
	if ((LED_ON_STAT == led_stat(0))){
		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
		plen=fill_tcp_data_p(buf,plen,(ON_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	} else {
		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	}

	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	if ((LED_ON_STAT == led_stat(0))){
		plen=fill_tcp_data_p(buf,plen,(OFF_0_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_0_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//LED 1 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 1 Status:"));
	if ((LED_ON_STAT == led_stat(1))){
		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
		plen=fill_tcp_data_p(buf,plen,(ON_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	} else {
		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	}

	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	if ((LED_ON_STAT == led_stat(1))){
		plen=fill_tcp_data_p(buf,plen,(OFF_1_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_1_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//LED 2 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 2 Status:"));
	if ((LED_ON_STAT == led_stat(2))){
		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
		plen=fill_tcp_data_p(buf,plen,(ON_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	} else {
		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	}

	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	if ((LED_ON_STAT == led_stat(2))){
		plen=fill_tcp_data_p(buf,plen,(OFF_2_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_2_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//LED 3 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 3 Status:"));
	if ((LED_ON_STAT == led_stat(3))){
		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
		plen=fill_tcp_data_p(buf,plen,(ON_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	} else {
		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
		plen=fill_tcp_data_p(buf,plen,("</font>"));
	}

	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	if ((LED_ON_STAT == led_stat(3))){
		plen=fill_tcp_data_p(buf,plen,(OFF_3_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_3_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//Task Information
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>TaskList:\r\n"));
	vTaskList(tmp_compose_buf);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("\r\n"));
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>RunTimeStats:\r\n"));
	vTaskGetRunTimeStats(tmp_compose_buf);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("\r\n"));

	//Refresh hypelink
	plen=fill_tcp_data_p(buf,plen,("<p><a href=\""));
	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("\">[Refresh]</a>\r\n<p><a href=\""));

	plen=fill_tcp_data_p(buf,plen,("<hr><p>Board Simple Server\r\n"));

	plen=fill_tcp_data_p(buf,plen,("</center>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("</body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("</html>\r\n"));

	printf("content len:%u\n", plen);

	return plen;
}

void protocol_init(void){
	//using static configuration now
	printf("MAC:%02X,%02X,%02X,%02X,%02X,%02X\n",
			g_mac_addr[0],g_mac_addr[1],g_mac_addr[2],g_mac_addr[3],g_mac_addr[4],g_mac_addr[5]);
	printf("IP:%d.%d.%d.%d\n",
			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
	printf("Port:%d\n",HTTP_PORT);
}

extern 	void ee_emu_test(void);

uint32_t g_a10_val[6];
uint32_t g_a11_val[6];
uint32_t g_pc_val[6];
uint32_t g_psw_val[6];
//uint32_t g_cdc_val[6];
uint32_t g_fcx_val[6];
uint32_t g_lcx_val[6];
uint32_t g_pcxi_val[6];

extern void label_after_call_main(void);
extern void __CSA_SIZE(void);
extern void __CSA_BEGIN(void);
extern void __CSA_END(void);

__attribute__( ( always_inline ) ) static inline uint32_t __get_A10(void) {
	register uint32_t result;

	__asm__ volatile ("mov.d %[d], %%a10" : [d] "=d" (result) : );

	return(result);
}

__attribute__( ( always_inline ) ) static inline uint32_t __get_A11(void) {
	register uint32_t result;

	__asm__ volatile ("mov.d %[d], %%a11" : [d] "=d" (result) : );
	return(result);
}

void test_func_4(void) {
	g_a10_val[5] = __get_A10();
	g_a11_val[5] = __get_A11();
	g_pc_val[5] = _mfcr(CPU_PC);
	g_psw_val[5] = _mfcr(CPU_PSW);
	//g_cdc_val[5] = CPU0_PSW.B.CDC;
	g_fcx_val[5] = _mfcr(CPU_FCX);
	g_lcx_val[5] = _mfcr(CPU_LCX);
	g_pcxi_val[5] = _mfcr(CPU_PCXI);
}

void test_func_3(void) {
	g_a10_val[4] = __get_A10();
	g_a11_val[4] = __get_A11();
	g_pc_val[4] = _mfcr(CPU_PC);
	g_psw_val[4] = _mfcr(CPU_PSW);
	//g_cdc_val[4] = CPU0_PSW.B.CDC;
	g_fcx_val[4] = _mfcr(CPU_FCX);
	g_lcx_val[4] = _mfcr(CPU_LCX);
	g_pcxi_val[4] = _mfcr(CPU_PCXI);

	test_func_4();
}

void test_func_2(void) {
	g_a10_val[3] = __get_A10();
	g_a11_val[3] = __get_A11();
	g_pc_val[3] = _mfcr(CPU_PC);
	g_psw_val[3] = _mfcr(CPU_PSW);
	//g_cdc_val[3] = CPU0_PSW.B.CDC;
	g_fcx_val[3] = _mfcr(CPU_FCX);
	g_lcx_val[3] = _mfcr(CPU_LCX);
	g_pcxi_val[3] = _mfcr(CPU_PCXI);

	test_func_3();
}

void test_func_1(void) {
	g_a10_val[1] = __get_A10();
	g_a11_val[1] = __get_A11();
	g_pc_val[1] = _mfcr(CPU_PC);
	g_psw_val[1] = _mfcr(CPU_PSW);
	//g_cdc_val[1] = CPU0_PSW.B.CDC;
	g_fcx_val[1] = _mfcr(CPU_FCX);
	g_lcx_val[1] = _mfcr(CPU_LCX);
	g_pcxi_val[1] = _mfcr(CPU_PCXI);
}

uint32_t test_func_recursive(uint32_t in) {
	uint32_t fcx = _mfcr(CPU_FCX);
	uint32_t lcx = _mfcr(CPU_LCX);
	uint32_t pcxi = _mfcr(CPU_PCXI);

	uint32_t ret_val;

	printf("\n FCX[%u]->[%08X] %u, LCX[%u]->[%08X] %u, PCXI[%u]->[%08X] %u\n",
			in, portCSA_TO_ADDRESS(fcx),
			((uint32_t)(portCSA_TO_ADDRESS(fcx))-(uint32_t)__CSA_BEGIN)>>6,
			in, portCSA_TO_ADDRESS(lcx),
			((uint32_t)(portCSA_TO_ADDRESS(lcx))-(uint32_t)__CSA_BEGIN)>>6,
			in, portCSA_TO_ADDRESS(pcxi),
			((uint32_t)(portCSA_TO_ADDRESS(pcxi))-(uint32_t)__CSA_BEGIN)>>6);
	flush_stdout();

	if(0==in) {
		ret_val = 1;
	} else {
		ret_val = in*test_func_recursive(in-1);
	}

	return ret_val;
}

int core0_main(int argc, char** argv) {
	prvSetupHardware();
	//SYSTEM_EnaDisCache(1);

	uart_init(mainCOM_TEST_BAUD_RATE);

	config_dts();

	printf("Baudrate:%u prescal:%u\n", mainCOM_TEST_BAUD_RATE, BAUDRATE_PRESCALE);
	flush_stdout();

	printf("ticks per sec:%u, timticks:%u, time scale:%u \n",
			TICKS_PER_SEC, TIMTICKS, TIM_SCALE);
	flush_stdout();

	printf("%s %s %s-%s @%u\n",
			_NEWLIB_VERSION, __func__, __DATE__, __TIME__, GET_CORE_ID());
	flush_stdout();

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

	_syscall(121);
	flush_stdout();

	printf("ticks per sec:%u, timticks:%u, time scale:%u \n",
			TICKS_PER_SEC, TIMTICKS, TIM_SCALE);
	flush_stdout();

	printf("label_after_call_main->%08X\n", (uint32_t)label_after_call_main);
	printf("core0_main->%08X\n", (uint32_t)core0_main);
	printf("CSA_SIZE = %08X(%u)\n", (uint32_t)__CSA_SIZE, ((uint32_t)__CSA_SIZE)>>6);
	printf("__CSA_BEGIN->%08X\n", (uint32_t)__CSA_BEGIN);
	printf("__CSA_END->%08X\n", (uint32_t)__CSA_END);
	flush_stdout();

	//	ee_emu_test();

	interface_init();
	protocol_init();

	//	server_loop();

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

void start_task(void *pvParameters) {
	Message_Queue = xQueueCreate(MESSAGE_Q_NUM, sizeof(uint32_t));

	CountSemaphore = xSemaphoreCreateCounting(2, 2);

	MutexSemaphore = xSemaphoreCreateMutex();

	xTaskCreate((TaskFunction_t )maintaince_task,
			(const char*    )"maintaince_task",
			(uint16_t       )768,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 1,
			(TaskHandle_t*  )&g_task0_handler);

	xTaskCreate((TaskFunction_t )print_task,
			(const char*    )"print_task",
			(uint16_t       )10*1024,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 2,
			(TaskHandle_t*  )&g_info_task_handler);
	vTaskDelete(StartTask_Handler);
}

#define	TEST_LOOP_N	2000000

/*      IMPLICIT REAL*8 (A-H,O-Z); */
#ifndef LOOPS
#define LOOPS (500)
#endif

static  float T,T1,T2,E1[5];
static int J,K,L;
static  float X1,X2,X3,X4;
static  volatile double ptime,time0;

static void PA(float* E);
static void P0(void);
static void P3(float *X, float *Y, float *Z);
static void POUT(int N, int J, int K, float X1, float X2, float X3, float X4);

#pragma section ".ram_code" ax
static inline double dtime (void) {
	return xTaskGetTickCount() / (1000.0);
}

void test_whets (void) {
	int LOOP,I,II,JJ,N1,N2,N3,N4,N5,N6,N7,N8,N9,N10,N11;
	float X,Y,Z;
	volatile double time1;

	printf("%s %p\n", __func__, test_whets);

	printf (" Benchmark #2 -- Single Precision Whetstone (A001)\n");
	printf (" %d LOOPS\n", LOOPS);

	/*      Start benchmark timing at this point. */
	time0 = dtime();
	ptime = time0;

	/*      The actual benchmark starts here. */

	T = .499975;
	T1 = 0.50025;
	T2 = 2.0;

	/*      With loopcount LOOP=10, one million Whetstone instructions */
	/*      will be executed in EACH MAJOR LOOP..A MAJOR LOOP IS EXECUTED */
	/*      'II' TIMES TO INCREASE WALL-CLOCK TIMING ACCURACY. */

	LOOP = LOOPS;
	II   = 1;

	for (JJ=1;JJ<=II;JJ++) { /* DO 500 JJ=1,II; */

		/*      Establish the relative loop counts of each module. */

		N1 = 0;
		N2 = 12 * LOOP;
		N3 = 14 * LOOP;
		N4 = 345 * LOOP;
		N5 = 0;
		N6 = 210 * LOOP;

		N7 = 32 * LOOP;

		N8 = 899 * LOOP;
		N9 = 616 * LOOP;
		N10 = 0;
		N11 = 93 * LOOP;

		/*      Module 1: Simple identifiers */

		X1 = 1.0;
		X2 = -1.0;
		X3 = -1.0;
		X4 = -1.0;

		if (N1!=0) { /* GO TO 35; */
			for(I=1;I<=N1;I++) { /* DO 30 I=1,N1; */
				X1 = (X1 + X2 + X3 - X4)*T;
				X2 = (X1 + X2 - X3 + X4)*T;
				X3 = (X1 - X2 + X3 + X4)*T;
				X4 = (-X1 + X2 + X3 + X4)*T;
				/* 30 */        };
		/* 35 */        };

		if (JJ==II) POUT(N1,N1,N1,X1,X2,X3,X4);

		/*      Module 2: Array elements */

		E1[1] = 1.0;
		E1[2] = -1.0;
		E1[3] = -1.0;
		E1[4] = -1.0;

		if (N2!=0) { /* GO TO 45; */
			for (I=1;I<=N2;I++) { /* DO 40 I=1,N2; */
				E1[1] = (E1[1] + E1[2] + E1[3] - E1[4])*T;
				E1[2] = (E1[1] + E1[2] - E1[3] + E1[4])*T;
				E1[3] = (E1[1] - E1[2] + E1[3] + E1[4])*T;
				E1[4] = (-E1[1] + E1[2] + E1[3] + E1[4])*T;
				/* 40 */        };
		/* 45 */        };

		if (JJ==II) POUT(N2,N3,N2,E1[1],E1[2],E1[3],E1[4]);

		/*      Module 3: Array as parameter */

		if (N3!=0) { /* GO TO 59; */
			for (I=1;I<=N3;I++) { /* DO 50 I=1,N3; */
				PA(E1);
			/* 50 */        };
		/* 59 */        };

		if (JJ==II) POUT(N3,N2,N2,E1[1],E1[2],E1[3],E1[4]);

		/*      Module 4: Conditional jumps */

		J = 1;
		if (N4!=0) { /* GO TO 65; */
			for (I=1;I<=N4;I++) { /* DO 60 I=1,N4; */
				if (J==1) goto L51;
				J = 3;
				goto L52;
				L51:            J = 2;
				L52:            if (J > 2) goto L53;
				J = 1;
				goto L54;
				L53:            J = 0;
				L54:            if (J < 1) goto L55;
				J = 0;
				goto L60;
				L55:            J = 1;
				L60:    ;       };
			/* 65 */        };

		if (JJ==II) POUT(N4,J,J,X1,X2,X3,X4);

		/*      Module 5: Omitted */
		/*      Module 6: Integer arithmetic */

		J = 1;
		K = 2;
		L = 3;

		if (N6!=0) { /* GO TO 75; */
			for (I=1;I<=N6;I++) { /* DO 70 I=1,N6; */
				J = J * (K-J) * (L-K);
				K = L * K - (L-J) * K;
				L = (L - K) * (K + J);
				E1[L-1] = J + K + L;
				E1[K-1] = J * K * L;
				/* 70 */        };
			/* 75 */        };

		if (JJ==II) POUT(N6,J,K,E1[1],E1[2],E1[3],E1[4]);

		/*      Module 7: Trigonometric functions */

		X = 0.5;
		Y = 0.5;

		if (N7!=0) { /* GO TO 85; */
			for (I=1;I<=N7;I++) { /* DO 80 I=1,N7; */
				X=T*atan(T2*sin(X)*cos(X)/(cos(X+Y)+cos(X-Y)-1.0));
				Y=T*atan(T2*sin(Y)*cos(Y)/(cos(X+Y)+cos(X-Y)-1.0));
			/* 80 */        };
		/* 85 */        };

		if (JJ==II) POUT(N7,J,K,X,X,Y,Y);

		/*      Module 8: Procedure calls */

		X = 1.0;
		Y = 1.0;
		Z = 1.0;

		if (N8!=0) { /* GO TO 95; */
			for (I=1;I<=N8;I++) { /* DO 90 I=1,N8; */
				P3(&X,&Y,&Z); /* FORTRAN semantics */
			/* 90 */        };
		/* 95 */        };

		if (JJ==II) POUT(N8,J,K,X,Y,Z,Z);

		/*      Module 9: Array references */

		J = 1;
		K = 2;
		L = 3;
		E1[1] = 1.0;
		E1[2] = 2.0;
		E1[3] = 3.0;

		if (N9!=0) { /* GO TO 105; */
			for (I=1;I<=N9;I++) { /* DO 100  I=1,N9; */
				P0();
			/* 100 */       };
		/* 105 */       };

		if (JJ==II) POUT(N9,J,K,E1[1],E1[2],E1[3],E1[4]);

		/*      Module 10: Integer arithmetic */

		J = 2;
		K = 3;

		if (N10!=0) { /* GO TO 115; */
			for (I=1;I<=N10;I++) { /* DO 110 I=1,N10; */
				J = J + K;
				K = J + K;
				J = K - J;
				K = K - J - J;
				/* 110 */       };
		/* 115 */       };

		if (JJ==II) POUT(N10,J,K,X1,X2,X3,X4);

		/*      Module 11: Standard functions */

		X = 0.75;

		if (N11!=0) { /* GO TO 125; */
			for (I=1;I<=N11;I++) { /* DO 120 I=1,N11; */
				X = sqrt(exp(log(X)/T1));
			/* 120 */       };
		/* 125 */       };

		if (JJ==II) POUT(N11,J,K,X,X,X,X);

		/*      THIS IS THE END OF THE MAJOR LOOP. */

	/* 500 */       };

	/*      Stop benchmark timing at this point. */

	time1 = dtime();

	/*---------------------------------------------------------------- */
	/*      Performance in Whetstone KIP's per second is given by */

	/*      (100*LOOP*II)/TIME */

	/*      where TIME is in seconds. */
	/*-------------------------------------------------------------------- */

	printf("time=%.6f secs,",time1-time0);
	if (time1 <= time0) {
		printf("error: %d\n", __LINE__);
	} else {
		printf (" Single Whetstone MIPS %.3f\n",((100*LOOP*II)/(time1-time0))/1000);
	}
}       /* END; */

static void PA(float* E){ /* SUBROUTINE PA(E); */
	/*      IMPLICIT REAL*8 (A-H,O-Z); */
	/*      DIMENSION E(4); */
	/*      COMMON T,T1,T2,E1(4),J,K,L; */
	int J1;
	J1 = 0;
	L10:    E[1] = (E[1] + E[2] + E[3] - E[4]) * T;
	E[2] = (E[1] + E[2] - E[3] + E[4]) * T;
	E[3] = (E[1] - E[2] + E[3] + E[4]) * T;
	E[4] = (-E[1] + E[2] + E[3] + E[4]) / T2;
	J1 = J1 + 1;
	if ((J1 - 6) < 0) goto L10; /* 10,20,20; */

	/* 20 */        return;
}       /* END; */

static void P0(void){ /* SUOUTINE P0; */
	/*      IMPLICIT REAL*8 (A-H,O-Z); */
	/*      COMMON T,T1,T2,E1(4),J,K,L; */
	E1[J] = E1[K];
	E1[K] = E1[L];
	E1[L] = E1[J];
	return;
}       /* END; */

static void P3(float *X, float *Y, float *Z){ /* SUBROUTINE P3(X,Y,Z); */
	/*      IMPLICIT REAL*8 (A-H,O-Z); */
	/*      COMMON T,T1,T2,E1(4),J,K,L; */
	float Y1;
	X1 = *X;
	Y1 = *Y;
	X1 = T * (X1 + Y1);
	Y1 = T * (X1 + Y1);
	*Z = (X1 + Y1) / T2;
	return;
}       /* END; */

static void POUT(int N, int J, int K, float X1, float X2, float X3, float X4) { /* SUBROUTINE POUT(N,J,K,X1,X2,X3,X4); */
	/*      IMPLICIT REAL*8 (A-H,O-Z); */
	/*      common/ptime/ptime,time0; */
	double time1,dtime();
	time1 = dtime();
#ifdef TRACE
	printf ("%3d%3d %7d%7d%7d%12.4g%12.4g%12.4g%12.4g\n", (int)(time1-time0),(int)(time1-ptime),N,J,K,X1,X2,X3,X4);
#endif
	ptime = time1;
	return;
}       /* END; */
#pragma section

void __attribute__((section(".ram_code"))) test_div_psram_2(void) {
	printf("%s %p\n", __func__, test_div_psram_2);
}

void test_div_flash(void)
{
	printf("%s %p\n", __func__, test_div_flash);

	for(uint32_t i=0; i<TEST_LOOP_N; ++i)
	{
		//Test unsigned integer division
		{
			volatile uint32_t au = 101;
			volatile uint32_t bu = 10;
			volatile uint32_t cu = au/bu;
			volatile uint32_t du = au%bu;

			//		printf("%u %u -> %u %u\n",
			//				au, bu, cu, du);
		}

		//Test signed integer division
		{
			volatile int32_t ai = 101;
			volatile int32_t bi = 10;
			volatile int32_t ci = ai/bi;
			volatile int32_t di = ai%bi;

			//		printf("%i %i -> %i %i\n",
			//				ai, bi, ci, di);
		}

		//Test float division
		{
			volatile float af = 101.0;
			volatile float bf = 10.0;
			volatile float cf = af/bf;
			//		float df = af%bf;

			//		printf("%f %f -> %f\n",
			//				af, bf, cf);
		}
	}
}

void maintaince_task(void *pvParameters) {
	//	char info_buf[512];

	while(1) {
		//		vTaskList(info_buf);
		//		if(NULL != MutexSemaphore) {
		//			if(pdTRUE == xSemaphoreTake(MutexSemaphore, portMAX_DELAY)) {
		//				printf("%s\r\n",info_buf);
		//
		//				vTaskGetRunTimeStats(info_buf);
		//				printf("RunTimeStats Len:%d\r\n", strlen(info_buf));
		//				printf("%s\r\n",info_buf);
		//
		//				printf("Tricore %04X Core:%04X, CPU:%u MHz,Sys:%u MHz,STM:%u MHz,PLL:%u M,Int:%u M,CE:%d\n",
		//						__TRICORE_NAME__,
		//						__TRICORE_CORE__,
		//						SYSTEM_GetCpuClock()/1000000,
		//						SYSTEM_GetSysClock()/1000000,
		//						SYSTEM_GetStmClock()/1000000,
		//						system_GetPllClock()/1000000,
		//						system_GetIntClock()/1000000,
		//						SYSTEM_IsCacheEnabled());
		//				flush_stdout();
		//
		//				xSemaphoreGive(MutexSemaphore);
		//			}
		//		}
		start_dts_measure();

		uint32_t NotifyValue=ulTaskNotifyTake( pdTRUE, /* Clear the notification value on exit. */
				portMAX_DELAY );
		vTaskDelay(40 / portTICK_PERIOD_MS);
	}
}

void print_task(void *pvParameters) {
	char info_buf[512];
	volatile uint8_t net_buf[MAX_FRAMELEN];

	uint16_t payloadlen;
	uint16_t dat_p;
	int16_t cmd16;

	while(true) {
		TickType_t tmp_tick0 = xTaskGetTickCount();
		test_whets();
		TickType_t tmp_tick1 = xTaskGetTickCount();
		test_div_flash();
		TickType_t tmp_tick2 = xTaskGetTickCount();
		test_div_psram_2();
		TickType_t tmp_tick3 = xTaskGetTickCount();

		printf("%u %u %u %u [%u] [%u] [%u]\r\n",
				tmp_tick0, tmp_tick1, tmp_tick2,  tmp_tick3,
				tmp_tick1-tmp_tick0, tmp_tick2-tmp_tick1, tmp_tick3-tmp_tick2);
		flush_stdout();

		printf("DieTempSensor: %f 'C\r\n", read_dts_celsius());
		flush_stdout();
		vTaskDelay(2000 / portTICK_PERIOD_MS);
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

