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

extern volatile uint8_t net_buf[MAX_FRAMELEN];

void server_loop(void) {
	uint16_t plen;
	uint16_t dat_p;
	uint16_t payloadlen=0;
	int16_t cmd16;

	while(true) {
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
