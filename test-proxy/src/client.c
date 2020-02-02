#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "types.h"
#include "session.h"

#define PROXY_INCLUDE_CLIENT_DEBUGGING_ 0

static void client_close_server_socket(int epoll_fd, int target_socket)
{
	static struct epoll_event evnet = {
		0,
	};
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, target_socket, &event);
}

static int client_command_received(struct session *target_session, char *buffer, int received_bytes)
{
	if (target_session == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_COMMAND_RECEIVED_INVALID \n");
#endif
		return CLIENT_COMMAND_RECEIVED_INVALID;
	}
	if (buffer == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_COMMAND_RECEIVED_INVALID_PARAM \n");
#endif
		return CLIENT_COMMAND_RECEIVED_INVALID_PARAM;
	}
	/*
	TODO : Must implement packet sender function to server
	*/
}

static int client_data_received(struct session *target_session, char *buffer, int received_bytes)
{
	if (target_session == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_DATA_RECEIVED_INVALID \n");
#endif
		return CLIENT_DATA_RECEIVED_INVALID;
	}
	if (buffer == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_DATA_RECEIVED_INVALID_PARAM \n");
#endif
		return CLIENT_DATA_RECEIVED_INVALID_PARAM;
	}
	/*
	TODO : Must implement packet sender function to server
	*/
}

static int client_read_packet(struct session *target_session, int port_type)
{
	int received_bytes = 0;
	int socket_fd = -1;
	static char buffer[COMMAND_BUFFER_SIZE] = {
		0,
	};

	if (target_session == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
#endif
		fprintf(stderr, "CLIENT_READ_PACKET_INVALID \n");
		return CLIENT_READ_PACKET_INVALID;
	}
	if (port_type == PORT_TYPE_COMMAND)
	{
		socket_fd = target_session->client->command_socket->fd;
	}
	else if (port_type == PORT_TYPE_DATA)
	{
		socket_fd = target_session->client->data_socket->fd;
	}
	else
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_READ_PAKCET_INVALID_PARAM \n");
#endif
		return CLIENT_READ_PAKCET_INVALID_PARAM;
	}

	memset(buffer, 0x00, sizeof(buffer));
	received_bytes = packet_full_read(socket_fd, buffer, sizeof(buffer));
	if (received_bytes <= 0)
	{
		/* Socket error */
		if (received_bytes == 0)
		{
			/* IF connection closed result is not error */
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
			fprintf(stderr, "CLIENT_READ_PACKET_CONNECTION_CLOSED \n");
#endif
			return CLIENT_READ_PACKET_CONNECTION_CLOSED;
		}
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_READ_PACKET_CONNECTION_ERROR \n");
#endif
		return CLIENT_READ_PACKET_CONNECTION_ERROR;
	}
	if (port_type == PORT_TYPE_COMMAND)
	{
		client_command_received(target_session, buffer, received_bytes);
	}
	else
	{
		client_data_received(target_session, buffer, received_bytes);
	}
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
	fprintf(stderr, "CLIENT_READ_PACKET_SUCCESS \n");
#endif
	return CLIENT_READ_PACKET_SUCCESS;
}

int clients_connect_polling()
{
	int epoll_fd = 0;
	int socket_fd = 0;
	int client_fd = 0;
	int active_events = 0;
	int client_count = 0;
	int i = 0; /* eventid */
	struct sockaddr_in bind_addr = {
		0,
	};
	struct sockaddr_in client_addr = {
		0,
	};
	struct epoll_event event = {
		0,
	};
	struct epoll_event events[MAX_CLIENT_EVENTS];
	socklen_t client_addr_len = 0;

	/* Creating an EPOLL object */
	epoll_fd = epoll_create(MAX_CLIENT_EVENTS);
	if (epoll_fd == -1)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_CONNECT_POLLING_EPOLL_CREATE_FAILED \n");
#endif
		return CLIENT_CONNECT_POLLING_EPOLL_CREATE_FAILED;
	}
	/* Create Socket */
	socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd < 0)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_CONNECT_POLLING_SOCKET_CREATE_FAILED \n");
#endif
		return CLIENT_CONNECT_POLLING_SOCKET_CREATE_FAILED;
	}

	memset(&bind_addr, 0x00, sizeof(bind_addr));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(FTP_COMMAND_PORT);

	/* Bind Socket */
	if (bind(socket_fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_CONNECT_POLLING_SOCKET_BIND_FAILED \n");
#endif
		return CLIENT_CONNECT_POLLING_SOCKET_BIND_FAILED;
	}

	/* Listen Socket */
	if (listen(socket_fd, 5) == -1)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_CONNECT_POLLING_SOCKET_LISTEN_FAILED \n");
#endif
		return CLIENT_CONNECT_POLLING_SOCKET_LISTEN_FAILED;
	}

	/* Notify me when events come in */
	event.events = EPOLLIN;
	/* Set the listening socket. */
	event.data.fd = socket_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
	{
		fprintf(stderr, "CLIENT_CONNECT_POLLING_EPOLL_CTL_FAILED \n");
		return CLIENT_CONNECT_POLLING_EPOLL_CTL_FAILED;
	}
	client_addr_len = sizeof(client_addr);
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
	printf("Start Clients_connect_polling \n");
#endif
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
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
				fprintf(stderr, "CLIENT_CONNECT_POLLING_EPOLL_WAIT_FAILED \n");
#endif
				return CLIENT_CONNECT_POLLING_EPOLL_WAIT_FAILED;
			}
		}

		for (i = 0; i < active_events; ++i)
		{
			/* Accept a Client */
			if (events[i].data.fd == socket_fd)
			{
				client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
				if (client_fd == -1)
				{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
					fprintf(stderr, "CLIENT_CONNECT_POLLING_SOCKET_ACCEPT_ERROR \n");
#endif
					continue;
				}
				event.data.fd = client_fd;
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1)
				{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
					fprintf(stderr, "CLIENT_CONNECT_POLLING_EPOLL_CTL_FAILED \n");
#endif
					return CLIENT_CONNECT_POLLING_EPOLL_CTL_FAILED;
				}
				else
				{
					/*
						TODO : client_fd, epoll_fd 를 session 리스트 command_socket, epoll_fd 에 담기
					*/
				}
			}
			else
			{
				current_session = get_session_from_list(session_list, events[i].data.fd, SOCKET_TYPE_CLIENT, PORT_TYPE_DATA);
				if (current_session == NULL)
				{
					continue;
				}

				if (events[i].data.fd == current_session->data_socket->fd)
				{
					recv();
					send();
				}
				else if (events[i].data.fd == current_session->command_socket->fd)
				{
					/* Do nothing */
				}
				else
				{
					/* Command socket poll in */
					recv();
					send();
				}
			}
		}
	}
	close(socket_fd);
	close(epoll_fd);
	return CLIENT_CONNECT_POLLING_SUCCESS;
}

int client_free(struct client *target_client)
{
	if (target_client == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_FREE_INVALID \n");
#endif
		return CLIENT_FREE_INVALID;
	}
	if (target_client->command_socket != NULL)
	{
		socket_free(target_client->command_socket);
		target_client->command_socket = NULL;
	}
	if (target_client->data_socket != NULL)
	{
		socket_free(target_client->data_socket);
		target_client->data_socket = NULL;
	}
	if (target_client->epoll_fd >= 0)
	{
		close(target_client->epoll_fd);
		target_client->epoll_fd = -1;
	}
	free(target_client);
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
	fprintf(stderr, "CLIENT_FREE_SUCCESS \n");
#endif
	return CLIENT_FREE_SUCCESS;
}

int client_loop(struct list *session_list)
{
	if (session_list == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_LOOP_INVALID \n");
#endif
		return CLIENT_LOOP_INVALID;
	}
	struct session *current_session = NULL;
	list_for_each_entry(current_session, session_list, list, struct session *)
	{
		client_session_polling(current_session);
	}
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
	fprintf(stderr, "CLIENT_LOOP_SUCCESS \n");
#endif
	return CLIENT_LOOP_SUCCESS;
}

int send_packet_to_client(struct client *target_client, char *buffer, int received_bytes, int port_type)
{
	int socket_fd = -1;
	int new_buffer_size = received_bytes;

	if (target_client == NULL)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_SEND_PACKET_INVALID \n");
#endif
		return CLIENT_SEND_PACKET_INVALID;
	}

	if (port_type == PORT_TYPE_COMMAND)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_SEND_PACKET_PORT_TYPE -> COMMAND \n");
#endif
		socket_fd = target_client->command_socket->fd;
	}
	else if (port_type == PORT_TYPE_DATA)
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_SEND_PACKET_PORT_TYPE -> DATA \n");
#endif
		socket_fd = target_client->data_socket->fd;
	}
	else
	{
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
		fprintf(stderr, "CLIENT_SEND_PACKET_INVALID_PARAM \n");
#endif
		return CLIENT_SEND_PACKET_INVALID_PARAM;
	}

	/* Buffer max size is always same as COMMAND_BUFFER_SIZE or DATA_BUFFER_SIZE */
	if (strncmp(buffer, "PORT", 4) == 0 && strlen(buffer) > 4)
	{
		/* TODO: Must modify new PORT command packet */
		buffer[4] = '\0';
		new_buffer_size = 5;
	}
	packet_full_write(socket_fd, buffer, new_buffer_size);
#if PROXY_INCLUDE_CLIENT_DEBUGGING_
	fprintf(stderr, "CLIENT_SEND_PACKET_SUCCESS \n")
#endif
		return CLIENT_SEND_PACKET_SUCCESS;
}