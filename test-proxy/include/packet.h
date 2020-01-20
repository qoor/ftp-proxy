#ifndef __PACKET_H__
#define __PACKET_H__

/* TCP Pseudo header */
struct pseudohdr
{
	u_int32_t saddr;		/* Source IP */
    u_int32_t daddr;		/* Destination IP */
    u_int8_t useless;		/* Unused */
    u_int8_t protocol;		/* Protocol type */
    u_int16_t tcplength;	/* TCP Header size */
};

/* TCP Header information use to calculate checksum */
struct tcpcksumhdr
{
	struct pseudohdr pseudo_header;
	struct tcphdr tcp_header;
};

void change_packet_dst_address(unsigned char* target_packet, struct sockaddr_in* address);
unsigned short in_cksum(unsigned short* addr, int len);

#endif
