#ifndef IP_ARP_UDP_TCP_H
#define IP_ARP_UDP_TCP_H

#include <stdint.h>

extern uint8_t eth_type_is_arp_and_my_ip(uint8_t *buf,uint32_t len);
extern uint8_t eth_type_is_ip_and_my_ip(uint8_t *buf,uint32_t len);
extern void make_arp_answer_from_request(uint8_t *buf);
extern void make_echo_reply_from_request(uint8_t *buf,uint32_t len);
extern void make_udp_reply_from_request(uint8_t *buf,char *data,uint32_t datalen,uint32_t port);

extern void make_tcp_synack_from_syn(uint8_t *buf);
extern void init_len_info(uint8_t *buf);
extern uint32_t get_tcp_data_pointer(void);
extern uint32_t fill_tcp_data_p(uint8_t *buf,uint32_t pos, const uint8_t *progmem_s);
extern uint32_t fill_tcp_data(uint8_t *buf,uint32_t pos, const char *s);
extern void make_tcp_ack_from_any(uint8_t *buf);
extern void make_tcp_ack_with_data(uint8_t *buf,uint32_t dlen);

#endif /* IP_ARP_UDP_TCP_H */
