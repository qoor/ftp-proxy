#ifndef __PACKET_H__
#define __PACKET_H__

struct packet
{
	struct iphdr* ip_header;
	struct tcphdr* tcp_header;
	unsigned char* body;
	size_t body_size;
};

struct pseudo_header
{
	u_int32_t saddr;		/* Source IP */
    u_int32_t daddr;		/* Destination IP */
    u_int8_t useless;		/* Unused */
    u_int8_t protocol;		/* Protocol type */
    u_int16_t tcplength;	/* TCP Header size */
};

void change_packet_dst_address(struct packet* target_packet, struct sockaddr_in* address);
unsigned short in_cksum(unsigned short* addr, int len);

#endif
