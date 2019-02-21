#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "enc28j60.h"
#include "ip_arp_udp_tcp.h"
#include "net.h"
#include "timer.h"

extern volatile uint16_t g_adc_buf[2];

extern const uint8_t g_ip_addr[4];
extern const uint8_t g_mac_addr[6];

#if(0 != UDP_TEST_SUPPORT)
#define TEST_UDP_PORT	((uint16_t)1200)
static uint8_t udp_buf[MAX_FRAMELEN];
#else
#warning udp test disabled
#endif //#if(0 != UDP_TEST_SUPPORT)

static uint8_t net_buf[MAX_FRAMELEN];

int32_t g_tmpK;

const char ON_STR[] = "On";
const char OFF_STR[] = "Off";

const char ON_05_STR[] = "On05";
const char OFF_05_STR[] = "Off05";
const char ON_06_STR[] = "On06";
const char OFF_06_STR[] = "Off06";
const char ON_07_STR[] = "On07";
const char OFF_07_STR[] = "Off07";
const char ON_14_STR[] = "On14";
const char OFF_14_STR[] = "Off14";
const char ON_15_STR[] = "On15";
const char OFF_15_STR[] = "Off15";

#define LED_ON_STAT		0
#define LED_OFF_STAT	1

void LD_05_ON(void){
	led_on(0);
}

void LD_05_OFF(void){
	led_off(0);
}

uint32_t LD_05_Stat(void) {
	return led_stat(0);
}

void LD_06_ON(void){
	led_on(1);
}

void LD_06_OFF(void){
	led_off(1);
}

uint32_t LD_06_Stat(void) {
	return led_stat(1);
}

//Note LD_07 is positive control logic, other LDs are negative control logic
void LD_07_ON(void){
	led_on(2);
}

void LD_07_OFF(void){
	led_off(2);
}

uint32_t LD_07_Stat(void) {
	return led_stat(2);
}

void LD_14_ON(void){
	led_on(3);
}

void LD_14_OFF(void){
	led_off(3);
}

uint32_t LD_14_Stat(void) {
	return led_stat(3);
}

int16_t analyse_get_url(char *str) {
	char* p_str = str;

	if (0==strncmp(p_str, ON_05_STR, strlen(ON_05_STR))) {
		return 0x0105;
	} else if (0==strncmp(p_str, OFF_05_STR, strlen(OFF_05_STR))) {
		return 0x0005;
	} else if (0==strncmp(p_str, ON_06_STR, strlen(ON_06_STR))) {
		return 0x0106;
	} else if (0==strncmp(p_str, OFF_06_STR, strlen(OFF_06_STR))) {
		return 0x0006;
	} else if (0==strncmp(p_str, ON_07_STR, strlen(ON_07_STR))) {
		return 0x0107;
	} else if (0==strncmp(p_str, OFF_07_STR, strlen(OFF_07_STR))) {
		return 0x0007;
	} else if (0==strncmp(p_str, ON_14_STR, strlen(ON_14_STR))) {
		return 0x0114;
	} else if (0==strncmp(p_str, OFF_14_STR, strlen(OFF_14_STR))) {
		return 0x0014;
	} else if (0==strncmp(p_str, ON_15_STR, strlen(ON_15_STR))) {
		return 0x0115;
	} else if (0==strncmp(p_str, OFF_15_STR, strlen(OFF_15_STR))) {
		return 0x0015;
	}else {
		return(-1);
	}
}

uint16_t prepare_page(uint8_t *buf) {
	uint16_t plen;
	uint8_t tmp_compose_buf[128];

	plen=fill_tcp_data_p(buf,0,("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));

	plen=fill_tcp_data_p(buf,plen,("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n"));

	sprintf((char*)tmp_compose_buf, "<title>Server[%s]</title>\r\n", __TIME__);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		

	plen=fill_tcp_data_p(buf,plen,("<body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("<center>\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>DieTempSensor: %f 'C\r\n", read_dts_celsius());
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("<p>Provided by Cortex M board\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>cpu: %u M, Sys:%u M, tick0:%08X, tick1:%08X, soft_spi:%u, newlib_v:%s\r\n",
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			MODULE_STM0.TIM0.U,
			MODULE_STM0.TIM1.U,
			SOFT_SPI,
			_NEWLIB_VERSION);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		
//
//	sprintf((char*)tmp_compose_buf,
//			"<p>MAC Rev: 0x%02X VS:%08X VE:%08X VL:%08X\r\n",
//			enc28j60getrev(),
//			(uint32_t)VeneerStart,
//			(uint32_t)VeneerEnd,
//			(uint32_t)VeneerSize);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);

//	sprintf((char*)tmp_compose_buf,
//			"<p>rcs:%08X rce:%08X rcl:%08X\r\n",
//			(uint32_t)__ram_code_start,
//			(uint32_t)__ram_code_end,
//			(uint32_t)__ram_code_size);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);

//	sprintf((char*)tmp_compose_buf,
//			"<p>hbs:%08X hbe:%08X hbl:%08X\r\n",
//			(uint32_t)Heap_Bank1_Start,
//			(uint32_t)Heap_Bank1_End,
//			(uint32_t)Heap_Bank1_Size);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
//
//	sprintf((char*)tmp_compose_buf,
//			"<p>isp:%08X ss:%08X\r\n",
//			(uint32_t)__initial_sp,
//			(uint32_t)stack_size);
//	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);

	//LED 0.5 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 05 Status:"));
	if ((LED_ON_STAT == LD_05_Stat())){
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
	if ((LED_ON_STAT == LD_05_Stat())){
		plen=fill_tcp_data_p(buf,plen,(OFF_05_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_05_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//LED 0.6 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 06 Status:"));
	if ((LED_ON_STAT == LD_06_Stat())){
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
	if ((LED_ON_STAT == LD_06_Stat())){
		plen=fill_tcp_data_p(buf,plen,(OFF_06_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_06_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//LED 0.7 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 07 Status:"));
	if ((LED_ON_STAT == LD_07_Stat())){
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
	if ((LED_ON_STAT == LD_07_Stat())){
		plen=fill_tcp_data_p(buf,plen,(OFF_07_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_07_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

	//LED 1.4 Control hypelink
	plen=fill_tcp_data_p(buf,plen,("\r\n<p>LED 14 Status:"));
	if ((LED_ON_STAT == LD_14_Stat())){
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
	if ((LED_ON_STAT == LD_14_Stat())){
		plen=fill_tcp_data_p(buf,plen,(OFF_14_STR));
	}else{
		plen=fill_tcp_data_p(buf,plen,(ON_14_STR));
	}
	plen=fill_tcp_data_p(buf,plen,("\">Toggle</a>"));

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

	printf("web page len:%u\n", plen);

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

void server_loop(void) {
	uint16_t plen;
	uint16_t dat_p;
	uint16_t payloadlen=0;
	int16_t cmd16;

	//init the ethernet/ip layer:
	while(true){
		plen = enc28j60PacketReceive(MAX_FRAMELEN, net_buf);

		if(plen==0) {
			continue; 
		} else if(eth_type_is_arp_and_my_ip(net_buf,plen)) {
			//Process ARP Request
			make_arp_answer_from_request(net_buf);
			continue;
		} else if(eth_type_is_ip_and_my_ip(net_buf,plen)==0) {
			//Only Process IP Packet destinated at me
			printf("$");
			continue;
		} else if(net_buf[IP_PROTO_P]==IP_PROTO_ICMP_V && net_buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V){
			//Process ICMP packet
			printf("Rxd ICMP from [%d.%d.%d.%d]\n",net_buf[ETH_ARP_SRC_IP_P],net_buf[ETH_ARP_SRC_IP_P+1],
					net_buf[ETH_ARP_SRC_IP_P+2],net_buf[ETH_ARP_SRC_IP_P+3]);
			make_echo_reply_from_request(net_buf, plen);
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
					plen=fill_tcp_data_p(net_buf,0,("Tricore\r\n\n\rHTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
					goto SENDTCP;
				}
				//Process HTTP Request
				if (strncmp("/ ",(char *)&(net_buf[dat_p+4]),2)==0) {
					//Update Web Page Content
					plen=prepare_page(net_buf);
					goto SENDTCP;
				}

				//Analysis the command in the URL
				cmd16 = analyse_get_url((char *)&(net_buf[dat_p+5]));
				if (cmd16 < 0) {
					plen=fill_tcp_data_p(net_buf,0,("HTTP/1.0 401 Unauthorized\r\nContent-Type: text/html\r\n\r\n<h1>401 Unauthorized</h1>"));
					goto SENDTCP;
				}
				if (0x0105 == cmd16)	{
					if(LED_ON_STAT != LD_05_Stat()) {
						LD_05_ON();
						printf(" O05 ");
					} else {
						printf(" N05 ");
					}
				} else if (0x0005 == cmd16) {
					if(LED_OFF_STAT != LD_05_Stat()) {
						LD_05_OFF();
						printf(" F05 ");
					} else {
						printf(" G05 ");
					}
				} else if (0x0106 == cmd16)	{
					if(LED_ON_STAT != LD_06_Stat()) {
						LD_06_ON();
					}
				} else if (0x0006 == cmd16) {
					if(LED_OFF_STAT != LD_06_Stat()) {
						LD_06_OFF();
					}
				} else if (0x0107 == cmd16)	{
					if(LED_ON_STAT != LD_07_Stat()) {
						LD_07_ON();
					}
				} else if (0x0007 == cmd16) {
					if(LED_OFF_STAT != LD_07_Stat()) {
						LD_07_OFF();
					}
				} else if (0x0114 == cmd16)	{
					if(LED_ON_STAT != LD_14_Stat()) {
						LD_14_ON();
					}
				} else if (0x0014 == cmd16) {
					if(LED_OFF_STAT != LD_14_Stat()) {
						LD_14_OFF();
					}
				}
				//Update Web Page Content
				plen=prepare_page(net_buf);

				SENDTCP:
				// send ack for http get
				make_tcp_ack_from_any(net_buf);
				// send data
				make_tcp_ack_with_data(net_buf,plen);
				continue;
			}
		}
#if(0 != UDP_TEST_SUPPORT)
		else if ( (net_buf[IP_PROTO_P]==IP_PROTO_UDP_V)&&
				(net_buf[UDP_DST_PORT_H_P]==(TEST_UDP_PORT>>8))&&
				(net_buf[UDP_DST_PORT_L_P]==(uint8_t)TEST_UDP_PORT) ) {
			//Process UDP Packet with port TEST_UDP_PORT
			payloadlen=	net_buf[UDP_LEN_H_P];
			payloadlen = payloadlen<<8;
			payloadlen = (payloadlen+net_buf[UDP_LEN_L_P])-UDP_HEADER_LEN;

			for(uint16_t i=0; i<payloadlen; i++){
				udp_buf[i]=net_buf[UDP_DATA_P+i];
			}

			make_udp_reply_from_request(net_buf,udp_buf, payloadlen, TEST_UDP_PORT);
		}
#endif //#if(0 != UDP_TEST_SUPPORT)
		else {
			//;
		}
	}
}
