#include <stdio.h>

#include "net.h"
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"

static uint16_t g_info_hdr_len;
static uint16_t g_info_data_len;

//initial tcp sequence number
static uint8_t g_seq_num = 0xa;

void interface_init(const uint8_t* p_mac_addr) {
	enc28j60Init(p_mac_addr);
}

// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we 
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start
// the calculation.
// len for udp is: 8 + 8 + data length
// len for tcp is: 4+4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
static uint32_t checksum(uint8_t *buf, uint32_t len,uint8_t type){
	// type 0=ip 
	//      1=udp
	//      2=tcp
	uint32_t sum = 0;

	if(type==0){
	        // do not add anything
	}else if(type==1){
		sum+=IP_PROTO_UDP_V; // protocol udp
		// the length here is the length of udp (data+header len)
		// =length given to this function - (IP.scr+IP.dst length)
		sum+=len-8; // = real tcp len
	}else if(type==2){
		sum+=IP_PROTO_TCP_V; 
		// the length here is the length of tcp (data+header len)
		// =length given to this function - (IP.scr+IP.dst length)
		sum+=len-8; // = real tcp len
	}
	// build the sum of 16bit words
	while(len >1){
		sum += 0xFFFF & (*buf<<8|*(buf+1));
		buf+=2;
		len-=2;
	}
	// if there is a byte left then add it (padded with zero)
	if (len){
		sum += (0xFF & *buf)<<8;
	}
	// now calculate the sum over the bytes in the sum
	// until the result is only 16bit long
	while (sum>>16){
		sum = (sum & 0xFFFF)+(sum >> 16);
	}
	// build 1's complement:
	return( (uint32_t) sum ^ 0xFFFF);
}

uint8_t eth_type_is_arp_and_my_ip(uint8_t *buf,uint32_t len){
	if (len<41){
		return(0);
	}

	//Non ARP Packet
	if(buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V || buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V){
		return(0);
	}

	//Not ARP Packet for myself
	for(uint8_t i=0; i<4; ++i){
		if(buf[ETH_ARP_DST_IP_P+i] != g_ip_addr[i]){
			return(0);
		}
	}
	printf("Rxd ARP Req from [%d.%d.%d.%d]\n",
	buf[ETH_ARP_SRC_IP_P],buf[ETH_ARP_SRC_IP_P+1],buf[ETH_ARP_SRC_IP_P+2],buf[ETH_ARP_SRC_IP_P+3]);

	return(1);
}

uint8_t eth_type_is_ip_and_my_ip(uint8_t *buf,uint32_t len){
	if (len<42){
		return(0);
	}

	//Not IP Packet
	if(buf[ETH_TYPE_H_P]!=ETHTYPE_IP_H_V || buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V){
		return(0);
	}

	// must be IP V4 and 20 byte header
	if (buf[IP_HEADER_LEN_VER_P]!=0x45)	{
		return(0);
	}

	//Only Process the IP packet with destination as myself
	for(uint8_t i=0; i<4; ++i){
		if(buf[IP_DST_P+i]!=g_ip_addr[i]){
			return(0);
		}
	}
	return(1);
}

// make a return eth header from a received eth packet
static void make_eth(uint8_t *buf){
	for(uint8_t i=0; i<6; ++i){
		buf[ETH_DST_MAC +i]=buf[ETH_SRC_MAC +i];
		buf[ETH_SRC_MAC +i]=g_mac_addr[i];
	}
}

static void fill_ip_hdr_checksum(uint8_t *buf){
	uint32_t ck;
	// clear the 2 byte checksum
	buf[IP_CHECKSUM_P]=0;
	buf[IP_CHECKSUM_P+1]=0;
	buf[IP_FLAGS_P]=0x40; // don't fragment
	buf[IP_FLAGS_P+1]=0;  // fragement offset
	buf[IP_TTL_P]=64; // ttl

	ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
	buf[IP_CHECKSUM_P]=ck>>8;
	buf[IP_CHECKSUM_P+1]=ck& 0xff;
}

// make a return ip header from a received ip packet
void make_ip(uint8_t *buf){
	for(uint8_t i=0; i<4; ++i){
		buf[IP_DST_P+i]=buf[IP_SRC_P+i];
		buf[IP_SRC_P+i]=g_ip_addr[i];
	}
	fill_ip_hdr_checksum(buf);
}

// make a return tcp header from a received tcp packet
// rel_ack_num is how much we must step the seq number received from the
// other side. We do not send more than 255 bytes of text (=data) in the tcp packet.
// If mss=1 then mss is included in the options list
//
// After calling this function you can fill in the first data byte at TCP_OPTIONS_P+4
// If cp_seq=0 then an initial sequence number is used (should be use in synack)
// otherwise it is copied from the packet we received
void make_tcphead(uint8_t *buf,uint32_t rel_ack_num,uint8_t mss,uint8_t cp_seq){
	uint8_t i=0;
	uint8_t tseq;

	while(i<2){
		buf[TCP_DST_PORT_H_P+i]=buf[TCP_SRC_PORT_H_P+i];
		buf[TCP_SRC_PORT_H_P+i]=0; // clear source port
		i++;
	}
	// set source port  (http):
	buf[TCP_SRC_PORT_L_P] = HTTP_PORT;
	i=4;
	// sequence numbers:
	// add the rel ack num to SEQACK
	while(i>0){
		rel_ack_num=buf[TCP_SEQ_H_P+i-1]+rel_ack_num;
		tseq=buf[TCP_SEQACK_H_P+i-1];
		buf[TCP_SEQACK_H_P+i-1]=0xff&rel_ack_num;
		if (cp_seq){
			// copy the acknum sent to us into the sequence number
			buf[TCP_SEQ_H_P+i-1]=tseq;
		} else {
			buf[TCP_SEQ_H_P+i-1]= 0; // some preset vallue
		}
		rel_ack_num=rel_ack_num>>8;
		i--;
	}

	if (cp_seq==0){
		// put inital seq number
		buf[TCP_SEQ_H_P+0]= 0;
		buf[TCP_SEQ_H_P+1]= 0;
		// we step only the second byte, this allows us to send packts 
		// with 255 bytes or 512 (if we step the initial g_seq_num by 2)
		buf[TCP_SEQ_H_P+2] = g_seq_num;
		buf[TCP_SEQ_H_P+3] = 0;
		// step the inititial seq num by something we will not use
		// during this tcp session:
		g_seq_num += 2;
	}
	// zero the checksum
	buf[TCP_CHECKSUM_H_P]=0;
	buf[TCP_CHECKSUM_L_P]=0;

	// The tcp header length is only a 4 bit field (the upper 4 bits).
	// It is calculated in units of 4 bytes. 
	// E.g 24 bytes: 24/4=6 => 0x60=header len field
	//buf[TCP_HEADER_LEN_P]=(((TCP_HEADER_LEN_PLAIN+4)/4)) <<4; // 0x60
	if (mss){
		// the only option we set is MSS to 1408:
		// 1408 in hex is 0x580
		buf[TCP_OPTIONS_P]=2;
		buf[TCP_OPTIONS_P+1]=4;
		buf[TCP_OPTIONS_P+2]=0x05; 
		buf[TCP_OPTIONS_P+3]=0x80;
		// 24 bytes:
		buf[TCP_HEADER_LEN_P]=0x60;
	} else {
		// no options:
		// 20 bytes:
		buf[TCP_HEADER_LEN_P]=0x50;
	}
}

void make_arp_answer_from_request(uint8_t *buf){
	make_eth(buf); 

	buf[ETH_ARP_OPCODE_H_P]=ETH_ARP_OPCODE_REPLY_H_V;  
	buf[ETH_ARP_OPCODE_L_P]=ETH_ARP_OPCODE_REPLY_L_V;

	for(uint8_t i=0; i<6; ++i){
		buf[ETH_ARP_DST_MAC_P+i]=buf[ETH_ARP_SRC_MAC_P+i];
		buf[ETH_ARP_SRC_MAC_P+i]=g_mac_addr[i];
	}

	for(uint8_t i=0; i<4; ++i){
		buf[ETH_ARP_DST_IP_P+i]=buf[ETH_ARP_SRC_IP_P+i];
		buf[ETH_ARP_SRC_IP_P+i]=g_ip_addr[i];
	}

	printf("Tricore[%d.%d.%d.%d]Send ARP Ans\n",g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);

	enc28j60PacketSend(42,buf); 
}

void make_echo_reply_from_request(uint8_t *buf,uint32_t len) {
	make_eth(buf);
	make_ip(buf);

	buf[ICMP_TYPE_P]=ICMP_TYPE_ECHOREPLY_V;	
	// we changed only the icmp.type field from request(=8) to reply(=0).
	// we can therefore easily correct the checksum:
	if (buf[ICMP_CHECKSUM_P] > (0xff-0x08)){
		buf[ICMP_CHECKSUM_P+1]++;
	}
	buf[ICMP_CHECKSUM_P]+=0x08;

	printf("Tricore[%d.%d.%d.%d]Send ICMP Ans\n",g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);

	enc28j60PacketSend(len,buf);
}

void make_tcp_synack_from_syn(uint8_t *buf){
	uint32_t ck;

	make_eth(buf);
	// total length field in the IP header must be set: 20 bytes IP + 24 bytes (20tcp+4tcp options)
	buf[IP_TOTLEN_H_P]=0;
	buf[IP_TOTLEN_L_P]=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4;

	make_ip(buf);
	buf[TCP_FLAGS_P]=TCP_FLAGS_SYNACK_V;
	make_tcphead(buf,1,1,0);
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + 4 (one option: mss)
	ck=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+4,2);
	buf[TCP_CHECKSUM_H_P]=ck>>8;
	buf[TCP_CHECKSUM_L_P]=ck& 0xff;
	// add 4 for option mss:
	printf("Tricore[%d.%d.%d.%d]Send SYN Ans\n",g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);

	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4+ETH_HEADER_LEN,buf);
}

// get a pointer to the start of tcp data in buf
// Returns 0 if there is no data
// You must call init_len_info once before calling this function
uint32_t get_tcp_data_pointer(void) {
	if (g_info_data_len){
		return((uint32_t)TCP_SRC_PORT_H_P+g_info_hdr_len);
	} else {
		return(0);
	}
}

// do some basic length calculations and store the result in static varibales
void init_len_info(uint8_t *buf) {
	g_info_data_len = (buf[IP_TOTLEN_H_P]<<8)|(buf[IP_TOTLEN_L_P]&0xff);
	g_info_data_len -= IP_HEADER_LEN;
	// generate len in bytes
	g_info_hdr_len = (buf[TCP_HEADER_LEN_P]>>4) * 4;
	g_info_data_len -= g_info_hdr_len;
	if (g_info_data_len <= 0) {
		g_info_data_len = 0;
	}
}

// fill in tcp data at position pos. pos=0 means start of
// tcp data. Returns the position at which the string after
// this string could be filled.
uint32_t fill_tcp_data_p(uint8_t *buf,uint32_t pos, const uint8_t *progmem_s) {
	char c;
	// fill in tcp data at position pos	  jesse
	while ((c = (char)*(progmem_s++)) != 0) {
		buf[TCP_CHECKSUM_L_P+3+pos]=c;
		pos++;
	}
	return(pos);
}

// fill in tcp data at position pos. pos=0 means start of
// tcp data. Returns the position at which the string after
// this string could be filled.
uint32_t fill_tcp_data(uint8_t *buf,uint32_t pos, const char *s){
	// fill in tcp data at position pos
	// with no options the data starts after the checksum + 2 more bytes (urgent ptr)
	while (*s) {
		buf[TCP_CHECKSUM_L_P+3+pos]=*s;
		pos++;
		s++;
	}
	return(pos);
}

// Make just an ack packet with no tcp data inside
// This will modify the eth/ip/tcp header 
void make_tcp_ack_from_any(uint8_t *buf) {
	uint32_t j;
	make_eth(buf);
	// fill the header:
	buf[TCP_FLAGS_P]=TCP_FLAGS_ACK_V;
	if (g_info_data_len==0){
		// if there is no data then we must still acknowledge one packet
		make_tcphead(buf,1,0,1); // no options
	} else {
		make_tcphead(buf,g_info_data_len,0,1); // no options
	}

	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) 
	j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN;
	buf[IP_TOTLEN_H_P]=j>>8;
	buf[IP_TOTLEN_L_P]=j& 0xff;
	make_ip(buf);
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN,2);
	buf[TCP_CHECKSUM_H_P]=j>>8;
	buf[TCP_CHECKSUM_L_P]=j& 0xff;

	printf("Tricore[%d.%d.%d.%d]Send ACK Ans\n",
	g_ip_addr[0],g_ip_addr[1],g_ip_addr[2],g_ip_addr[3]);

	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+ETH_HEADER_LEN,buf);
}

// you must have called init_len_info at some time before calling this function
// dlen is the amount of tcp data (http data) we send in this packet
// You can use this function only immediately after make_tcp_ack_from_any
// This is because this function will NOT modify the eth/ip/tcp header except for
// length and checksum
void make_tcp_ack_with_data(uint8_t *buf,uint32_t dlen) {
	uint32_t j;
	// fill the header:
	// This code requires that we send only one data packet
	// because we keep no state information. We must therefore set
	// the fin here:
	buf[TCP_FLAGS_P]=TCP_FLAGS_ACK_V|TCP_FLAGS_PUSH_V|TCP_FLAGS_FIN_V;

	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) + len of data
	j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen;
	buf[IP_TOTLEN_H_P]=j>>8;
	buf[IP_TOTLEN_L_P]=j& 0xff;
	fill_ip_hdr_checksum(buf);
	// zero the checksum
	buf[TCP_CHECKSUM_H_P]=0;
	buf[TCP_CHECKSUM_L_P]=0;
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+dlen,2);
	buf[TCP_CHECKSUM_H_P]=j>>8;
	buf[TCP_CHECKSUM_L_P]=j& 0xff;
	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen+ETH_HEADER_LEN,buf);
}

