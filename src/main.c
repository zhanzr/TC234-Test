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

#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "app_ethernet.h"
#include "httpserver-socket.h"

//SemaphoreHandle_t MutexSemaphore;

struct netif gnetif; /* network interface structure */

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

TaskHandle_t g_start_task_handler;
void start_task(void *pvParameters);

//TaskHandle_t g_task0_handler;
//void maintaince_task(void *pvParameters);
//TaskHandle_t g_info_task_handler;
//void print_task(void *pvParameters);

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

void interface_init(void) {
	enc28j60Init();
}

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
//
//uint16_t prepare_page(uint8_t *buf) {
//	uint16_t plen;
//	uint8_t tmp_compose_buf[512];
//
//	plen=fill_tcp_data_p(buf,0,("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));
//
//	plen=fill_tcp_data_p(buf,plen,("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n"));
//
//	sprintf((char*)tmp_compose_buf, "<title>Server[%s]</title>\r\n", __TIME__);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
//
//	plen=fill_tcp_data_p(buf,plen,("<body>\r\n"));
//	plen=fill_tcp_data_p(buf,plen,("<center>\r\n"));
//
//	sprintf((char*)tmp_compose_buf, "<p>DieTempSensor: %f 'C\r\n", read_dts_celsius());
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
//	plen=fill_tcp_data_p(buf,plen,("<p>Server on TC234 Appkit board\r\n"));
//
//	sprintf((char*)tmp_compose_buf, "<p>cpu: %u M, Sys:%u M, STM0.TIM1:%08X, soft_spi:%u, newlib:%s\r\n",
//			SYSTEM_GetCpuClock()/1000000,
//			SYSTEM_GetSysClock()/1000000,
//			MODULE_STM0.TIM1.U,
//			SOFT_SPI,
//			_NEWLIB_VERSION);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
//
//	//LED 0 Control hypelink
//	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 0 Status:"));
//	if ((LED_ON_STAT == led_stat(0))){
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
//		plen=fill_tcp_data_p(buf,plen,(ON_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	} else {
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
//		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	}
//
//	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
//	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
//			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	if ((LED_ON_STAT == led_stat(0))){
//		plen=fill_tcp_data_p(buf,plen,(OFF_0_STR));
//	}else{
//		plen=fill_tcp_data_p(buf,plen,(ON_0_STR));
//	}
//	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));
//
//	//LED 1 Control hypelink
//	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 1 Status:"));
//	if ((LED_ON_STAT == led_stat(1))){
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
//		plen=fill_tcp_data_p(buf,plen,(ON_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	} else {
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
//		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	}
//
//	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
//	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
//			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	if ((LED_ON_STAT == led_stat(1))){
//		plen=fill_tcp_data_p(buf,plen,(OFF_1_STR));
//	}else{
//		plen=fill_tcp_data_p(buf,plen,(ON_1_STR));
//	}
//	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));
//
//	//LED 2 Control hypelink
//	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 2 Status:"));
//	if ((LED_ON_STAT == led_stat(2))){
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
//		plen=fill_tcp_data_p(buf,plen,(ON_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	} else {
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
//		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	}
//
//	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
//	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
//			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	if ((LED_ON_STAT == led_stat(2))){
//		plen=fill_tcp_data_p(buf,plen,(OFF_2_STR));
//	}else{
//		plen=fill_tcp_data_p(buf,plen,(ON_2_STR));
//	}
//	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));
//
//	//LED 3 Control hypelink
//	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 3 Status:"));
//	if ((LED_ON_STAT == led_stat(3))){
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"red\">"));
//		plen=fill_tcp_data_p(buf,plen,(ON_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	} else {
//		plen=fill_tcp_data_p(buf,plen,("<font color=\"green\">"));
//		plen=fill_tcp_data_p(buf,plen,(OFF_STR));
//		plen=fill_tcp_data_p(buf,plen,("</font>"));
//	}
//
//	plen=fill_tcp_data_p(buf,plen,("\t\t<a href=\""));
//	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
//			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	if ((LED_ON_STAT == led_stat(3))){
//		plen=fill_tcp_data_p(buf,plen,(OFF_3_STR));
//	}else{
//		plen=fill_tcp_data_p(buf,plen,(ON_3_STR));
//	}
//	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));
//
//	//Task Information
//	plen=fill_tcp_data_p(buf,plen,("\r\n<p>TaskList:\r\n"));
//	vTaskList(tmp_compose_buf);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	plen=fill_tcp_data_p(buf,plen,("\r\n"));
//	plen=fill_tcp_data_p(buf,plen,("\r\n<p>RunTimeStats:\r\n"));
//	vTaskGetRunTimeStats(tmp_compose_buf);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	plen=fill_tcp_data_p(buf,plen,("\r\n"));
//
//	//Refresh hypelink
//	plen=fill_tcp_data_p(buf,plen,("<p><a href=\""));
//	sprintf((char*)tmp_compose_buf, "http://%u.%u.%u.%u/",
//			g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);
//	plen=fill_tcp_data(buf,plen, (const char*)tmp_compose_buf);
//	plen=fill_tcp_data_p(buf,plen,("\">[Refresh]</a>\r\n<p><a href=\""));
//
//	plen=fill_tcp_data_p(buf,plen,("<hr><p>Board Simple Server\r\n"));
//
//	plen=fill_tcp_data_p(buf,plen,("</center>\r\n"));
//	plen=fill_tcp_data_p(buf,plen,("</body>\r\n"));
//	plen=fill_tcp_data_p(buf,plen,("</html>\r\n"));
//
//	printf("content len:%u\n", plen);
//
//	return plen;
//}

void protocol_init(void){
	//using static configuration now
	printf("MAC:%02X-%02X-%02X-%02X-%02X-%02X\n",
			MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5);
	printf("IP:%d.%d.%d.%d\n",
			IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
	printf("Port:%d\n",HTTP_PORT);
}

int core0_main(int argc, char** argv) {
	prvSetupHardware();

//	SYSTEM_EnaDisCache(1);

	uart_init(mainCOM_TEST_BAUD_RATE);

	//config_dts();

//https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
	uint32_t a = 1;
	uint32_t b = 0;
	uint32_t c = 0;
//	uint32_t c = Ifx_CRC32(b, a);
//	printf("CRC32(%08X, %08X) = %08X\n", a, b, c);
//	flush_stdout();

    __asm__ volatile ("CRC32 %0,%1,%2" : "=d" (c) : "d"(b), "d"(a));
	printf("CRC32(%08X, %08X) = %08X\n", a, b, c);
	flush_stdout();

	a = 0x01020304;
	c = Ifx_CRC32(b, a);
//	printf("CRC32(%08X, %08X) = %08X\n", a, b, c);
//	flush_stdout();
    __asm__ volatile ("CRC32 %0,%1,%2" : "=d" (c) : "d"(b), "d"(a));
	printf("CRC32(%08X, %08X) = %08X\n", a, b, c);
	flush_stdout();

	a = 0x11223344;
	c = Ifx_CRC32(b, a);
    __asm__ volatile ("CRC32 %0,%1,%2" : "=d" (c) : "d"(b), "d"(a));
	printf("CRC32(%08X, %08X) = %08X\n", a, b, c);
	flush_stdout();

	a = 0x61626364;
	c = Ifx_CRC32(b, a);
    __asm__ volatile ("CRC32 %0,%1,%2" : "=d" (c) : "d"(b), "d"(a));
	printf("CRC32(%08X, %08X) = %08X\n", a, b, c);
	flush_stdout();

	uint8_t test_d[]={0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
	printf("%08X %04X\n", test_d, *(uint16_t*)test_d);
	printf("%08X %04X\n", test_d+1, *(uint16_t*)(test_d+1));
	printf("%08X %08X\n", test_d, *(uint32_t*)test_d);
	printf("%08X %08X\n", test_d+1, *(uint32_t*)(test_d+1));
	printf("%08X %08X\n", test_d+2, *(uint32_t*)(test_d+2));
	printf("%08X %08X\n", test_d+3, *(uint32_t*)(test_d+3));
	flush_stdout();

    static unsigned int lock;

    unsigned __builtin_tricore_cmpswapw
    (volatile void *addr, unsigned new_value, unsigned compare_val);
        while( __builtin_tricore_cmpswapw( &lock, 1, 0 ) ) ;

	printf("%08X %08X %08X %08X %08X %08X %08X\n",
			_mfcr( CPU_FCX ),
			_mfcr( CPU_LCX ),
			_mfcr( CPU_PCXI ),
			_mfcr( CPU_ISP ),
			_mfcr( CPU_PC ),
			_mfcr( CPU_DEADD ),
			_mfcr( CPU_DSTR )
	);
	flush_stdout();
	uint32_t* p32 = 0xD0000000 + 184*1024;
	printf("%p %08X\n", p32, *(uint32_t*)(p32));

	printf("%s\n", _NEWLIB_VERSION);

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

	_syscall(101);

	interface_init();
	flush_stdout();

	protocol_init();
	flush_stdout();

	xTaskCreate((TaskFunction_t )start_task,
			(const char*    )"start_task",
			(uint16_t       )512,
			(void*          )NULL,
			(UBaseType_t    )tskIDLE_PRIORITY + 1,
			(TaskHandle_t*  )&g_start_task_handler);

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

extern err_t ethernetif_init(struct netif *netif);
static void Netif_Config(void) {
	ip_addr_t ipaddr;
	ip_addr_t netmask;
	ip_addr_t gw;

	IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
	IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
	IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);

	/* add the network interface */
	netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

	/*  Registers the default network interface. */
	netif_set_default(&gnetif);

	gnetif.flags |= NETIF_FLAG_UP;

	if (netif_is_link_up(&gnetif)) {
		/* When the netif is fully configured this function must be called.*/
		netif_set_up(&gnetif);
	} else {
		/* When the netif link is down this function must be called */
		netif_set_down(&gnetif);
	}
}

void start_task(void *pvParameters) {
//	MutexSemaphore = xSemaphoreCreateMutex();

	/* Create tcp_ip stack thread */
	tcpip_init(NULL, NULL);

	/* Initialize the LwIP stack */
	Netif_Config();

	/* Initialize webserver demo */
	http_server_socket_init();

	/* Notify user about the network interface config */
	//	  User_notification(&gnetif);

	//	xTaskCreate((TaskFunction_t )maintaince_task,
	//			(const char*    )"maintaince_task",
	//			(uint16_t       )768,
	//			(void*          )NULL,
	//			(UBaseType_t    )tskIDLE_PRIORITY + 1,
	//			(TaskHandle_t*  )&g_task0_handler);

	//	xTaskCreate((TaskFunction_t )print_task,
	//			(const char*    )"print_task",
	//			(uint16_t       )10*1024,
	//			(void*          )NULL,
	//			(UBaseType_t    )tskIDLE_PRIORITY + 2,
	//			(TaskHandle_t*  )&g_info_task_handler);

	vTaskDelete(g_start_task_handler);
}

//void maintaince_task(void *pvParameters) {
//	//	char info_buf[512];
//
//	while(1) {
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
		//		start_dts_measure();
		//
		//		uint32_t NotifyValue=ulTaskNotifyTake( pdTRUE, /* Clear the notification value on exit. */
		//						portMAX_DELAY );/* Block indefinitely. */
//		vTaskDelay(40 / portTICK_PERIOD_MS);
//	}
//}

//void print_task(void *pvParameters) {
//	char info_buf[512];
//	volatile uint8_t net_buf[MAX_FRAMELEN];
//
//	uint16_t payloadlen;
//	uint16_t dat_p;
//	int16_t cmd16;
//
//	while(true) {
		//		payloadlen = enc28j60PacketReceive(MAX_FRAMELEN, net_buf);
		//
		//		if(payloadlen==0) {
		//			vTaskDelay(20 / portTICK_PERIOD_MS);
		//			continue;
		//		} else if(eth_type_is_arp_and_my_ip(net_buf,payloadlen)) {
		//			//Process ARP Request
		//			make_arp_answer_from_request(net_buf);
		//			continue;
		//		} else if(eth_type_is_ip_and_my_ip(net_buf,payloadlen)==0) {
		//			//Only Process IP Packet destinated at me
		//			printf("$");
		//			continue;
		//		} else if(net_buf[IP_PROTO_P]==IP_PROTO_ICMP_V && net_buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){
		//			//Process ICMP packet
		//			printf("Rxd ICMP from [%d.%d.%d.%d]\n",net_buf[ETH_ARP_SRC_IP_P],net_buf[ETH_ARP_SRC_IP_P+1],
		//					net_buf[ETH_ARP_SRC_IP_P+2],net_buf[ETH_ARP_SRC_IP_P+3]);
		//			make_echo_reply_from_request(net_buf, payloadlen);
		//			continue;
		//		} else if (net_buf[IP_PROTO_P]==IP_PROTO_TCP_V&&net_buf[TCP_DST_PORT_H_P]==0&&net_buf[TCP_DST_PORT_L_P]==HTTP_PORT) {
		//			//Process TCP packet with HTTP_PORT
		//			printf("Rxd TCP http pkt\n");
		//			if (net_buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V) {
		//				printf("Type SYN\n");
		//				make_tcp_synack_from_syn(net_buf);
		//				continue;
		//			}
		//			if (net_buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V) {
		//				printf("Type ACK\n");
		//				init_len_info(net_buf); // init some data structures
		//				dat_p=get_tcp_data_pointer();
		//				if (dat_p==0) {
		//					if (net_buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V) {
		//						make_tcp_ack_from_any(net_buf);
		//					}
		//					continue;
		//				}
		//				// Process Telnet request
		//				if (strncmp("GET ",(char *)&(net_buf[dat_p]),4)!=0){
		//					payloadlen=fill_tcp_data_p(net_buf,0,("Tricore\r\n\n\rHTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
		//					goto SENDTCP;
		//				}
		//				//Process HTTP Request
		//				if (strncmp("/ ",(char *)&(net_buf[dat_p+4]),2)==0) {
		//					//Update Web Page Content
		//					payloadlen=prepare_page(net_buf);
		//					goto SENDTCP;
		//				}
		//
		//				//Analysis the command in the URL
		//				cmd16 = analyse_get_url((char *)&(net_buf[dat_p+5]));
		//				if (cmd16 < 0) {
		//					payloadlen=fill_tcp_data_p(net_buf,0,("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
		//					goto SENDTCP;
		//				}
		//				if (CMD_LD0_ON == cmd16)	{
		//					if(LED_ON_STAT != led_stat(0)) {
		//						led_on(0);
		//					} else {
		//						;
		//					}
		//				} else if (CMD_LD0_OFF == cmd16) {
		//					if(LED_OFF_STAT != led_stat(0)) {
		//						led_off(0);
		//					} else {
		//						;
		//					}
		//				} else if (CMD_LD1_ON == cmd16)	{
		//					if(LED_ON_STAT != led_stat(1)) {
		//						led_on(1);
		//					}
		//				} else if (CMD_LD1_OFF == cmd16) {
		//					if(LED_OFF_STAT != led_stat(1)) {
		//						led_off(1);
		//					}
		//				} else if (CMD_LD2_ON == cmd16)	{
		//					if(LED_ON_STAT != led_stat(2)) {
		//						led_on(2);
		//					}
		//				} else if (CMD_LD2_OFF == cmd16) {
		//					if(LED_OFF_STAT != led_stat(2)) {
		//						led_off(2);
		//					}
		//				} else if (CMD_LD3_ON == cmd16)	{
		//					if(LED_ON_STAT != led_stat(3)) {
		//						led_on(3);
		//					}
		//				} else if (CMD_LD3_OFF == cmd16) {
		//					if(LED_OFF_STAT != led_stat(3)) {
		//						led_off(3);
		//					}
		//				}
		//				//Update Web Page Content
		//				payloadlen=prepare_page(net_buf);
		//
		//				SENDTCP:
		//				// send ack for http get
		//				make_tcp_ack_from_any(net_buf);
		//				// send data
		//				make_tcp_ack_with_data(net_buf,payloadlen);
		//				continue;
		//			}
		//		} else {
		//			//;
		//		}
//	}
//}

static void prvSetupHardware( void ) {
	system_clk_config_200_100();

	/* activate interrupt system */
	InterruptInit();

	SYSTEM_Init();

	/* Initialize LED outputs. */
	vParTestInitialise();

	spi_init();
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

