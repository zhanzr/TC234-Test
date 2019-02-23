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

extern const uint8_t g_ip_addr[4];
extern const uint8_t g_mac_addr[6];

static uint8_t net_buf[MAX_FRAMELEN];

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

#define LED_ON_STAT		0
#define LED_OFF_STAT	1

#define CMD_LD0_ON	0x0100
#define CMD_LD0_OFF	0x0000
#define CMD_LD1_ON	0x0101
#define CMD_LD1_OFF	0x0001
#define CMD_LD2_ON	0x0102
#define CMD_LD2_OFF	0x0002
#define CMD_LD3_ON	0x0103
#define CMD_LD3_OFF	0x0003

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
	uint8_t tmp_compose_buf[128];

	plen=fill_tcp_data_p(buf,0,("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nPragma: no-cache\r\n\r\n"));

	plen=fill_tcp_data_p(buf,plen,("<!DOCTYPE html>\r\n<html lang=\"en\">\r\n"));

	sprintf((char*)tmp_compose_buf, "<title>Server[%s]</title>\r\n", __TIME__);
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);		

	plen=fill_tcp_data_p(buf,plen,("<body>\r\n"));
	plen=fill_tcp_data_p(buf,plen,("<center>\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>DieTempSensor: %f 'C\r\n", read_dts_celsius());
	plen=fill_tcp_data(buf,plen,(const char*)tmp_compose_buf);
	plen=fill_tcp_data_p(buf,plen,("<p>Simple Server on TC234 Appkit board\r\n"));

	sprintf((char*)tmp_compose_buf, "<p>cpu: %u M, Sys:%u M, STM0.TIM0:%08X, STM0.TIM1:%08X, soft_spi:%u, newlib:%s\r\n",
			SYSTEM_GetCpuClock()/1000000,
			SYSTEM_GetSysClock()/1000000,
			MODULE_STM0.TIM0.U,
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

void server_loop(void) {
	uint16_t plen;
	uint16_t dat_p;
	uint16_t payloadlen=0;
	int16_t cmd16;

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
				plen=prepare_page(net_buf);

				SENDTCP:
				// send ack for http get
				make_tcp_ack_from_any(net_buf);
				// send data
				make_tcp_ack_with_data(net_buf,plen);
				continue;
			}
		} else {
			//;
		}
	}
}
