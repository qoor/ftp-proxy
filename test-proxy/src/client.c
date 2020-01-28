#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include <netinet/ip.h>
#include <netinet/in.h>

#include "proxy.h"
#include "packet.h"
#include "types.h"

/*
static void client_free(struct client* target_client);

static int remove_client_loop(void* key, void* value, void* context)
{
	client_free((struct client*)value);
	return 1;
}

 Add client to destination from function get_client() 
int add_clients()
{
	int i = 0;
	int client_count = 0;
	int ret = CLIENT_ADD_SUCCESS;

	if ()
}
*/

void polling_client()
{
	int epoll_fd = 0;
	int socket_fd = 0;
	int socket_opt = 1;
	int bind_client_addr_len = 0;
	int ip_header_length = 0;
	int active_events = 0;
	int i = 0; /* eventid */
	char rbuff[BUFSIZ] = { 0, }; /* recv packet */
	struct iphdr* ip_header = NULL;
	struct tcphdr* tcp_header = NULL;
	struct epoll_event event = { 0, };
	struct epoll_event events[MAX_CLIENT_EVENTS] = { {0,}, };
	struct sockaddr_in bind_client_addr = { 0, };
	struct client* client_ptr = malloc(sizeof(struct client));

	/* Creating an EPOLL object */
	epoll_fd = epoll_create(MAX_CLIENT_EVENTS); 
	if (epoll_fd == -1)
	{
		return EPOLL_CREATE_FAILED;
	}

	/* Create RAW Socket */
	socket_fd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
	if (socket_fd < 0)
	{
		return SOCKET_CREATE_FAILED;
	}

/*
	Set the source IP address to be used for datagrams that will be sent to the raw socket when the RAW socket is created and bind is called.
	(Only if IP_HERINCL socket option is not set) If bind is not called, the kernel sets source IP address as the first IP address of the output interface.
*/

	/* Set IP_HDRINCL Option */
	if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &socket_opt, sizeof(socket_opt)) < 0)
	{
		return SOCKET_OPT_FAILED;
	}

	/* recv packet */
	/* Uses revvfrom to store the terminal address from which the received data was sent to bind_client_addr */
	if(recvfrom(socket_fd, rbuff, BUFSIZ - 1, 0x0, (struct sockaddr*)&bind_client_addr, (socklen_t*)&bind_client_addr_len) < 0)
	{
		return SOCKET_RECV_FAILED;
	}

	event.events = EPOLLIN; /* Notify me when events come in */
	event.data.fd = socket_fd; /* Set the listening socket. */
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
	{
		return EPOLL_CTL_FAILED;
	}

	printf("Start Monitoring ... \n");
	while (TRUE)
	{
		/* Obtain headers for received packets */
		ip_header = (struct iphdr*) rbuff;
		ip_header_length = ip_header->ihl * 4;
		tcp_header = (struct tcphdr*)((char*)ip_header + ip_header_length);

		/* Epoll Monitoring */
		active_events = epoll_wait(epoll_fd, events, MAX_CLIENT_EVENTS, -1);
		if (active_events == -1)
		{
			/* continue if interrupt */
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				return EPOLL_WAIT_FAILED;
			}
		}

		/* Destination port finds something like BIND_CLIENT_PORT among incoming packets */
		if (ntohs(tcp_header->dest) == BIND_CLIENT_PORT)
		{
			for (i = 0; i < active_events; ++i)
			{
				/* polling a client */
				if (events[i].data.fd == socket_fd)
				{
					strcpy(client_ptr->client_address, ip_header->saddr);
					strcpy(client_ptr->socket_fd, events[i].data.fd);

					printf("client_ptr->client_address : %15s \n", inet_ntoa(*(struct in_addr*)&ip_header->saddr));
					printf("client_ptr->socket_fd : %d \n", events[i].data.fd);

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


}