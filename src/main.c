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
#include "timer.h"

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

	_syscall(121);
	flush_stdout();

	g_a10_val[0] = __get_A10();
	g_a11_val[0] = __get_A11();
	g_pc_val[0] = _mfcr(CPU_PC);
	g_psw_val[0] = _mfcr(CPU_PSW);
	//g_cdc_val[0] = CPU0_PSW.B.CDC;
	g_fcx_val[0] = _mfcr(CPU_FCX);
	g_lcx_val[0] = _mfcr(CPU_LCX);
	g_pcxi_val[0] = _mfcr(CPU_PCXI);
	test_func_1();

	g_a10_val[2] = __get_A10();
	g_a11_val[2] = __get_A11();
	g_pc_val[2] = _mfcr(CPU_PC);
	g_psw_val[2] = _mfcr(CPU_PSW);
	//g_cdc_val[2] = CPU0_PSW.B.CDC;
	g_fcx_val[2] = _mfcr(CPU_FCX);
	g_lcx_val[2] = _mfcr(CPU_LCX);
	g_pcxi_val[2] = _mfcr(CPU_PCXI);

	test_func_2();

	printf("test_func_1->%08X\n", (uint32_t)test_func_1);
	printf("test_func_2->%08X\n", (uint32_t)test_func_2);
	printf("test_func_3->%08X\n", (uint32_t)test_func_3);
	printf("test_func_4->%08X\n", (uint32_t)test_func_4);
	printf("label_after_call_main->%08X\n", (uint32_t)label_after_call_main);
	printf("core0_main->%08X\n", (uint32_t)core0_main);
	printf("CSA_SIZE = %08X(%u)\n", (uint32_t)__CSA_SIZE, ((uint32_t)__CSA_SIZE)>>6);
	printf("__CSA_BEGIN->%08X\n", (uint32_t)__CSA_BEGIN);
	printf("__CSA_END->%08X\n", (uint32_t)__CSA_END);
	flush_stdout();

	for(uint32_t i=0; i<6; ++i) {
//		printf("A10[%u] = %08X, A11[%u] = %08X, PC[%u] = %08X\n",
//				i, g_a10_val[i],
//				i, g_a11_val[i],
//				i, g_pc_val[i]);

		printf("\n FCX[%u]->[%08X] %u, LCX[%u]->[%08X] %u, PCXI[%u]->[%08X] %u\n",
				i, portCSA_TO_ADDRESS(g_fcx_val[i]),
				((uint32_t)(portCSA_TO_ADDRESS(g_fcx_val[i]))-(uint32_t)__CSA_BEGIN)>>6,
				i, portCSA_TO_ADDRESS(g_lcx_val[i]),
				((uint32_t)(portCSA_TO_ADDRESS(g_lcx_val[i]))-(uint32_t)__CSA_BEGIN)>>6,
				i, portCSA_TO_ADDRESS(g_pcxi_val[i]),
				((uint32_t)(portCSA_TO_ADDRESS(g_pcxi_val[i]))-(uint32_t)__CSA_BEGIN)>>6);
	}
	flush_stdout();

	uint32_t tmp_ret = test_func_recursive(10);
	printf("recursive(10)=%u\n", tmp_ret);
	flush_stdout();

	tmp_ret = test_func_recursive(62);
	printf("recursive(62)=%u\n", tmp_ret);
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
				portMAX_DELAY );/* Block indefinitely. */
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
		payloadlen = enc28j60PacketReceive(MAX_FRAMELEN, net_buf);

		if(payloadlen==0) {
			vTaskDelay(20 / portTICK_PERIOD_MS);
			continue;
		} else if(eth_type_is_arp_and_my_ip(net_buf,payloadlen)) {
			//Process ARP Request
			make_arp_answer_from_request(net_buf);
			continue;
		} else if(eth_type_is_ip_and_my_ip(net_buf,payloadlen)==0) {
			//Only Process IP Packet destinated at me
			printf("$");
			continue;
		} else if(net_buf[IP_PROTO_P]==IP_PROTO_ICMP_V && net_buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){
			//Process ICMP packet
			printf("Rxd ICMP from [%d.%d.%d.%d]\n",net_buf[ETH_ARP_SRC_IP_P],net_buf[ETH_ARP_SRC_IP_P+1],
					net_buf[ETH_ARP_SRC_IP_P+2],net_buf[ETH_ARP_SRC_IP_P+3]);
			make_echo_reply_from_request(net_buf, payloadlen);
			continue;
		} else if (net_buf[IP_PROTO_P]==IP_PROTO_TCP_V&&net_buf[TCP_DST_PORT_H_P]==0&&net_buf[TCP_DST_PORT_L_P]==HTTP_PORT) {
			//Process TCP packet with HTTP_PORT
			printf("Rxd TCP http pkt\n");
			if (net_buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V) {
				printf("Type SYN\n");
				make_tcp_synack_from_syn(net_buf);
				continue;
			}
			if (net_buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V) {
				printf("Type ACK\n");
				init_len_info(net_buf); // init some data structures
				dat_p=get_tcp_data_pointer();
				if (dat_p==0) {
					if (net_buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V) {
						make_tcp_ack_from_any(net_buf);
					}
					continue;
				}
				// Process Telnet request
				if (strncmp("GET ",(char *)&(net_buf[dat_p]),4)!=0){
					payloadlen=fill_tcp_data_p(net_buf,0,("Tricore\r\n\n\rHTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
					goto SENDTCP;
				}
				//Process HTTP Request
				if (strncmp("/ ",(char *)&(net_buf[dat_p+4]),2)==0) {
					//Update Web Page Content
					payloadlen=prepare_page(net_buf);
					goto SENDTCP;
				}

				//Analysis the command in the URL
				cmd16 = analyse_get_url((char *)&(net_buf[dat_p+5]));
				if (cmd16 < 0) {
					payloadlen=fill_tcp_data_p(net_buf,0,("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
					goto SENDTCP;
				}
				if (CMD_LD0_ON == cmd16)	{
					if(LED_ON_STAT != led_stat(0)) {
						led_on(0);
					} else {
						;
					}
				} else if (CMD_LD0_OFF == cmd16) {
					if(LED_OFF_STAT != led_stat(0)) {
						led_off(0);
					} else {
						;
					}
				} else if (CMD_LD1_ON == cmd16)	{
					if(LED_ON_STAT != led_stat(1)) {
						led_on(1);
					}
				} else if (CMD_LD1_OFF == cmd16) {
					if(LED_OFF_STAT != led_stat(1)) {
						led_off(1);
					}
				} else if (CMD_LD2_ON == cmd16)	{
					if(LED_ON_STAT != led_stat(2)) {
						led_on(2);
					}
				} else if (CMD_LD2_OFF == cmd16) {
					if(LED_OFF_STAT != led_stat(2)) {
						led_off(2);
					}
				} else if (CMD_LD3_ON == cmd16)	{
					if(LED_ON_STAT != led_stat(3)) {
						led_on(3);
					}
				} else if (CMD_LD3_OFF == cmd16) {
					if(LED_OFF_STAT != led_stat(3)) {
						led_off(3);
					}
				}
				//Update Web Page Content
				payloadlen=prepare_page(net_buf);

				SENDTCP:
				// send ack for http get
				make_tcp_ack_from_any(net_buf);
				// send data
				make_tcp_ack_with_data(net_buf,payloadlen);
				continue;
			}
		} else {
			//;
		}
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

