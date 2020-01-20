#include "StdInc.h"

void change_packet_dst_address(unsigned char* target_packet, struct sockaddr_in* address)
{
	struct iphdr* ip_header = target_packet;
	int ip_header_length = 0;
	struct tcphdr* tcp_header = NULL;
	struct tcpcksumhdr tcp_cksum_header = { 0, };

	if (target_packet == NULL)
	{
		return;
	}

	ip_header_length = ip_header->ihl * 4;
	tcp_header = (struct tcphdr*)((char*)ip_header + ip_header_length);
	
	ip_header->daddr = address->sin_addr.s_addr;
	ip_header->check = 0;
	ip_header->check = in_cksum((unsigned short*)ip_header, sizeof(struct iphdr));
	
	tcp_header->dest = address->sin_addr.s_addr;
	tcp_header->check = 0;

	memcpy((char*)(&tcp_cksum_header) + sizeof(struct pseudohdr), tcp_header, sizeof(struct tcphdr));

	tcp_cksum_header.pseudo_header.saddr = tcp_header->source;
	tcp_cksum_header.pseudo_header.daddr = tcp_header->dest;
	tcp_cksum_header.pseudo_header.protocol = ip_header->protocol;
	tcp_cksum_header.pseudo_header.tcplength = ip_header->tot_len - ip_header_length;

	tcp_header->check = in_cksum((unsigned char*)(&tcp_cksum_header.pseudo_header), sizeof(tcp_cksum_header));
}

/*
 * in_cksum --
 * Checksum routine for Internet Protocol
 * family headers (C Version)
*/
unsigned short in_cksum(unsigned short* addr, int len)
{
	register int sum = 0;
	unsigned short answer = 0;
	register unsigned short *w = addr;
	register int nleft = len;
	/*
	* Our algorithm is simple, using a 32 bit accumulator (sum), we add
	* sequential 16 bit words to it, and at the end, fold back all the
	* carry bits from the top 16 bits into the lower 16 bits.
	*/
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	/* mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		*(unsigned char*)(&answer) = *(unsigned char*)w;
		sum += answer;
	}
	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);             /* add carry */
	answer = ~sum;              /* truncate to 16 bits */
	return (answer);
}
