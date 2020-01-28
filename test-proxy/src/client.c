#include "client.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h> /* uintx_t */

void polling_client()
{
	/*Declare create_epoll related variables*/
	int epoll_fd = 0;
	/*Create_raw_socket related variable declaration*/
	int socket_fd = 0;
	int client_addr_len = 0;
	int sock_opt = 1;
	int flags = 0;
	struct sockaddr_in client_addr = { 0, };
	char rbuff[BUFSIZ] = { 0, };
	/*declaration of variables associated with listen_epoll*/
	struct epoll_event event = { 0, };
	struct epoll_event events[MAX_CLIENT_EVENTS] = { {0,}, };
	struct client_data* client_data = NULL;
	int active_events = 0;
	int i = 0; /* eventid */
	int client_file_descriptor = 0;
	/* declaration of packet-related variables */
	int ip_header_length = 0;
	struct iphdr* ip_header = NULL;
	struct tcphdr* tcp_header = NULL;


	epoll_fd = epoll_create(MAX_CLIENT_EVENTS); /* Creating an EPOLL object */
	if (epoll_fd == -1)
	{
		fprintf(stderr, "EPOLL CREATE ERROR");
	}

	socket_fd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP); /* Create RAW Sockets */
	if (socket_fd < 0)
	{
		fprintf(stderr, "RAW SOCKET CREATE ERROR");
	}

	/*
	Set the source IP address to be used for datagrams that will be sent to the raw socket when the RAW socket is created and bind is called.
	(Only if IP_HERINCL socket option is not set) If bind is not called, the kernel sets source IP address as the first IP address of the output interface.
	*/

	if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &sock_opt, sizeof(sock_opt)) < 0) /* Set IP_HDRINCL Options */
	{
		fprintf(stderr, "SET SOCKET OPT ERROR \n");
	}

	event.events = EPOLLIN; /* 이벤트가 들어오면 알림 */
	event.data.fd = socket_fd; /* 듣기 소켓을 추가한다 */
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
	{
		fprintf(stderr, "EPOLL CTL ERROR \n");
	}
/* 넣으면 에러발생해서 일단 빼둠
	flags = fcntl(socket_fd, F_GETFL, 0);
	fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
*/
	printf("CLIENT POLLING... \n");
	while (1)
	{
		if (recvfrom(socket_fd, rbuff, BUFSIZ - 1, 0x0, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len) < 0) /* recv packet */
		/* 수신받은 데이터를 송신한 단말 주소를 client_addr 에 저장하기 위해 recvfrom 사용함 */
		{
			fprintf(stderr, "RECV ERROR \n");
			continue;
		}
		ip_header = (struct iphdr*) rbuff;
		ip_header_length = ip_header->ihl * 4;
		tcp_header = (struct tcphdr*)((char*)ip_header + ip_header_length);

		/* 
		사건 발생까지 무한 대기
		epoll_fd의 사건 발생 시 events에 fd를 채운다
		active_events은 listen에 성공한 fd의 수
		*/
		active_events = epoll_wait(epoll_fd, events, MAX_CLIENT_EVENTS, -1); /* Monitoring */
		if (active_events == -1)
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
			for (i = 0; i < active_events; i++)
			{
				if (events[i].data.fd == socket_fd) /* polling a client */
				{
					printf("epoll_fd : %d \n", epoll_fd);
					printf("socket_fd : %d \n", socket_fd);
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
	close(socket_fd);
	close(epoll_fd);
}