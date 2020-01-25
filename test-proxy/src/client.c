#include "client.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> /* uintx_t */

void polling_client()
{
	/*Declare create_epoll related variables*/
	int epoll_file_descriptor = 0;
	/*Create_raw_socket related variable declaration*/
	int raw_socket_file_descriptor = 0;
	int client_addr_len = 0;
	int sock_opt = 1;
	struct sockaddr_in client_addr = { 0, };
	char rbuff[BUFSIZ] = { 0, };
	/*declaration of variables associated with listen_epoll*/
	struct epoll_event event = { 0, };
	struct epoll_event events[MAX_CLIENT_EVENTS] = { {0,}, };
	int event_ready = 0;
	int i = 0;
	/* declaration of packet-related variables */
	int ip_header_length = 0;
	struct iphdr* ip_header = NULL;
	struct tcphdr* tcp_header = NULL;


	epoll_file_descriptor = epoll_create(MAX_CLIENT_EVENTS); /* Creating an EPOLL object */
	if (epoll_file_descriptor == -1)
	{
		fprintf(stderr, "EPOLL CREATE ERROR");
	}

	raw_socket_file_descriptor = socket(PF_INET, SOCK_RAW, IPPROTO_TCP); /* Create RAW Sockets */
	if (raw_socket_file_descriptor < 0)
	{
		fprintf(stderr, "RAW SOCKET CREATE ERROR");
	}
	

	/*
	Set the source IP address to be used for datagrams that will be sent to the raw socket when the RAW socket is created and bind is called.
	(Only if IP_HERINCL socket option is not set) If bind is not called, the kernel sets source IP address as the first IP address of the output interface.
	*/
	if (setsockopt(raw_socket_file_descriptor, IPPROTO_IP, IP_HDRINCL, &sock_opt, sizeof(sock_opt)) < 0) /* Set IP_HDRINCL Options */
	{
		fprintf(stderr, "SET SOCKET OPT ERROR \n");
	}

	event.events = EPOLLIN;
	event.data.fd = raw_socket_file_descriptor;
	if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, raw_socket_file_descriptor, &event) == -1)
	{
		fprintf(stderr, "EPOLL CTL ERROR \n");
	}

	printf("CLIENT POLLING... \n");
	while (1)
	{
		if (recvfrom(raw_socket_file_descriptor, rbuff, BUFSIZ - 1, 0x0, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len) < 0) /* recv packet */
		{
			fprintf(stderr, "RECV ERROR \n");
			continue;
		}
		ip_header = (struct iphdr*) rbuff;
		ip_header_length = ip_header->ihl * 4;
		tcp_header = (struct tcphdr*)((char*)ip_header + ip_header_length);

		event_ready = epoll_wait(epoll_file_descriptor, events, MAX_CLIENT_EVENTS, -1); /* Monitoring */
		if (event_ready == -1)
		{
			if (errno == EINTR)
			{
				continue; /* continue if interrupt */
			}
			else
			{
				fprintf(stderr, "EPOLL WAIT ERROR \n");
			}

		}

		if (ntohs(tcp_header->dest) == BIND_CLIENT_PORT) /* Destination port finds something like BIND_CLIENT_PORT among incoming packets */
		{
			for (i = 0; i < event_ready; i++)
			{
				if (events[i].data.fd == raw_socket_file_descriptor) /* polling a client */
				{

					printf("epoll_file_descriptor : %d \n", epoll_file_descriptor);
					printf("raw_socket_file_descriptor : %d \n", raw_socket_file_descriptor);
					printf("events[i].data.fd : %d \n",events[i].data.fd);

					printf("========== RECV TCP(FTP) SEGMENT ========== \n");
					printf("\n");
					printf("========== IP HEADER ========== \n");
					printf("HEADER LENGTH : %d \n", ip_header_length);
					printf("TOTAL LENGTH : %d \n", ntohs(ip_header->tot_len));
					printf("SOURCE ADDRESS : %15s \n", inet_ntoa(*(struct in_addr*)&ip_header->saddr));
					printf("DESTINATION ADDRESS : %15s \n", inet_ntoa(*(struct in_addr*)&ip_header->daddr));
					printf("TIME TO LIVE : %d \n", ip_header->ttl);
					printf("\n");
					printf("========== TCP HEADER ========== \n");
					printf("HEADER LENGTH : %d \n", ntohs(ip_header->tot_len) - ip_header_length);
					printf("SOURCE PORT : %d \n", ntohs(tcp_header->source));
					printf("DESTINATION PORT : %d \n", ntohs(tcp_header->dest));
					printf("SEQUENCE NO: %u \n", ntohl(tcp_header->seq));
					printf("ACK NO: %u \n", ntohl(tcp_header->ack_seq));
					printf("SYN NO: %d \n", ntohs(tcp_header->syn));
					printf("FLAGS: %c%c%c%c%c%c \n", (tcp_header->fin ? 'F' : 'X'), (tcp_header->syn ? 'F' : 'X'), (tcp_header->rst ? 'F' : 'X'), (tcp_header->psh ? 'F' : 'X'), (tcp_header->ack ? 'F' : 'X'), (tcp_header->urg ? 'F' : 'X'));
					printf("CHECKSUM : %X \n", ntohs(tcp_header->check));
					printf("\n");
					printf("=========================================== \n");
				}
			}
		}
	}
	close(raw_socket_file_descriptor);
	close(epoll_file_descriptor);
}