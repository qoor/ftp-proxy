#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "types.h"

int polling_client()
{
	int epoll_fd = 0;
	int socket_fd = 0;
	int client_fd = 0;
	int client_addr_len = 0;
	int active_events = 0;
	int i = 0; /* eventid */
	struct sockaddr_in bind_addr = { 0, };
	struct sockaddr_in client_addr = { 0, };
	struct epoll_event event = { 0, };
	struct epoll_event events[MAX_CLIENT_EVENTS] = { {0,}, };


	/* Creating an EPOLL object */
	epoll_fd = epoll_create(MAX_CLIENT_EVENTS);
	if (epoll_fd == -1)
	{
		return EPOLL_CREATE_FAILED;
	}

	/* Create Socket */
	socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd < 0)
	{
		return SOCKET_CREATE_FAILED;
	}

	memset(&bind_addr, 0, sizeof(bind_addr));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(BIND_CLIENT_PORT);

	/* Bind Socket */
	if (bind(socket_fd, (struct sockaddr*) &bind_addr, sizeof(bind_addr)) == -1)
	{
		return SOCKET_BIND_FAILED;
	}

	/* Listen Socket */
	if (listen(socket_fd, 5) < 0)
	{
		return SOCKET_LISTEN_FAILED;
	}

	/* Notify me when events come in */
	event.events = EPOLLIN;
	/* Set the listening socket. */
	event.data.fd = socket_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
	{
		return EPOLL_CTL_FAILED;
	}

	client_addr_len = sizeof(client_addr);
	printf("Start Monitoring ... \n");
	while (TRUE)
	{
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

		for (i = 0; i < active_events; ++i)
		{
			/* Accept a Client */
			if (events[i].data.fd == socket_fd)
			{
				client_fd = accept(socket_fd, (struct sockaddr*) &client_addr, &client_addr_len);
				if (client_fd == -1)
				{
					fprintf(stderr, "ACCEPT ERROR\n");
					continue;
				}
				event.data.fd = client_fd;
				fprintf(stderr, "A Client [%s:%d] is Connected : fd[%d] .... \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),client_fd);
				
				/*
				if (add_session_to_list(,client_fd,SOCKET_TYPE_CLIENT,PORT_TYPE_COMMAND) == SESSION_SUCCESS)  session 리스트에 클라이언트 파일 디스크립터를 등록함 
				{
					if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)  추가된 파일 디스크립터를 감지 목록에 추가 
					{
						return EPOLL_CTL_FAILED;
					}
					else
					{

					}
				}
				*/
			}
		}
	}
	close(socket_fd);
	close(epoll_fd);
	return POLLING_SUCCESS;
}