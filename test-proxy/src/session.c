#include "session.h"

#include <malloc.h>
#include <string.h>
#include <errno.h>

#include <sys/epoll.h>
#include <arpa/inet.h>

#include "types.h"
#include "client.h"
#include "server.h"
#include "log.h"
#include "option.h"
#include "packet.h"

/*
TODO : SESSION.C 
- RETURN 코드 수정하기 : 함수 동작은 클라이언트 서버 모두 담당하나 리턴코드는 대부분 서버 리턴코드를 반환함
*/

static int session_get_client_socket_type(int socket_fd, const struct client* source_client)
{
	if ((source_client == NULL) || (socket_fd == -1))
	{
		return -1;
	}

	if (source_client->command_socket != NULL && source_client->command_socket->fd == socket_fd)
	{
		return PORT_TYPE_COMMAND;
	}
	else if (source_client->data_socket != NULL && source_client->data_socket->fd == socket_fd)
	{
		return PORT_TYPE_DATA;
	}

	return -1;
}

static int session_get_server_socket_type(int socket_fd, const struct server* source_server)
{
	if ((source_server == NULL) || (socket_fd == -1))
	{
		return -1;
	}

	if (source_server->command_socket != NULL && source_server->command_socket->fd == socket_fd)
	{
		return PORT_TYPE_COMMAND;
	}
	else if (source_server->data_socket != NULL && source_server->data_socket->fd == socket_fd)
	{
		return PORT_TYPE_DATA;
	}
	else if (source_server->connection_socket != NULL && source_server->connection_socket->fd == socket_fd)
	{
		return PORT_TYPE_DATA_CONNECTION;
	}

	return -1;
}

static int socket_del_from_epoll(int epoll_fd, int socket_fd)
{
	static struct epoll_event event = { 0, };
	int ret = 0;

	if ((epoll_fd == -1) || (socket_fd == -1))
	{
		return SESSION_INVALID_SOCKET;
	}

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, &event);
	if (ret < 0)
	{
		return SESSION_EPOLL_CTL_FAILED;
	}
	
	return SESSION_SUCCESS;
}

static int is_socket_in_session(const struct session* source_session, int socket_fd)
{
	int ret = 0;

	if (source_session == NULL)
	{
		return FALSE;
	}

	if (socket_fd < 0)
	{
		return FALSE;
	}

	if (source_session->client != NULL)
	{
		ret = session_get_client_socket_type(socket_fd, source_session->client);
		if (ret != -1)
		{
			return TRUE;
		}
	}
	if (source_session->server != NULL)
	{
		ret = session_get_server_socket_type(socket_fd, source_session->server);
		if (ret != -1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

static struct session* add_session_to_list(struct list* session_list, int epoll_fd, int event_socket)
{
	struct session* new_session = NULL;
	struct client* new_client = NULL;
	struct server* new_server = NULL;
	struct sockaddr_in client_address = { 0, };
	uint32_t address_length = sizeof(struct sockaddr);
	const struct sockaddr_in* server_address = NULL;
	int connected_socket = -1;

	if (session_list == NULL)
	{
		return NULL;
	}

	new_session = (struct session*)malloc(sizeof(struct session));
	if (new_session == NULL)
	{
		return NULL;
	}
	
	new_session->client = NULL;
	new_session->server = NULL;
	LIST_INIT(&new_session->list);

	connected_socket = accept(event_socket, (struct sockaddr*)&client_address, &address_length);
	if ((connected_socket < 0) && (errno != EINPROGRESS))
	{
		session_remove_from_list(new_session);

		return NULL;
	}
	
	getsockname(connected_socket, (struct sockaddr*)&(new_session->host_address), &address_length); /* Getting my external IP */

	server_address = server_get_available_address(&global_option->server_list);
	if (server_address == NULL)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	new_client = client_create(new_session, connected_socket);
	if (new_client == NULL)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	new_server = server_create(server_address);
	if (new_server == NULL)
	{
		client_free(new_client);
		session_remove_from_list(new_session);

		return NULL;
	}

	new_session->client = new_client;
	new_session->server = new_server;

	LIST_ADD(session_list, &new_session->list);

	return new_session;
}

int session_remove_from_list(struct session* target_session)
{
	if (target_session == NULL)
	{
		return SESSION_INVALID_SESSION;
	}

	if (target_session->client != NULL)
	{
		client_free(target_session->client);
		target_session->client = NULL;
	}

	if (target_session->server != NULL)
	{
		server_free(target_session->server);
		target_session->server = NULL;
	}

	if (LIST_ISEMPTY(&target_session->list) == FALSE)
	{
		LIST_DEL(&target_session->list);
	}

	return SESSION_INVALID_SESSION;
}

struct session* get_session_from_list(const struct list* session_list, int socket_fd)
{
	struct session* current_session = NULL;

	errno = 0;

	if (session_list == NULL)
	{
		errno = SESSION_INVALID_LIST;
		return NULL;
	}

	if (socket_fd < 0)
	{
		errno = SESSION_INVALID_PARAMS;
		return NULL;
	}

	list_for_each_entry(current_session, session_list, list, struct session*)
	{
		if (is_socket_in_session(current_session, socket_fd) == TRUE)
		{
			return current_session;
		}
	}

	errno = SESSION_INVALID_SOCKET;

	return NULL;
}

int session_polling(int epoll_fd, struct list* session_list, int proxy_connect_socket, struct epoll_event* events)
{
	int active_event_count = 0;
	int event_id = 0;
	int event_socket = -1;
	struct session* target_session = NULL;
	struct sockaddr_in data_client_address = { 0, };
	struct socket* data_client_socket = NULL;
	int ret = 0;

	active_event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
	if (active_event_count == -1)
	{
		if (errno == EINTR)
		{
			return SESSION_SUCCESS;
		}
		
		return SERVER_POLLING_WAIT_ERROR;
	}

	for (event_id = 0; event_id < active_event_count; ++event_id)
	{
		event_socket = events[event_id].data.fd;
		if (event_socket == proxy_connect_socket)
		{
			target_session = add_session_to_list(session_list, epoll_fd, event_socket);
			if (target_session == NULL)
			{
				proxy_error("session", "Session of socket fd %d create failed", event_socket);
				continue;
			}

			proxy_error("session", "Proxy listen socket: %d, event socket: %d registered", proxy_connect_socket, event_socket);
		}
		else
		{
			target_session = get_session_from_list(session_list, event_socket);
			if (target_session == NULL)
			{
				proxy_error("session", "Unknown socket fd: %d", event_socket);
				
				continue;
			}

			if ((target_session->server->data_socket != NULL) &&
				(event_socket == target_session->server->data_socket->fd))
			{
				/* Data socket connection */
				data_client_socket = server_accept(target_session->server, &data_client_address);
				if (data_client_socket == NULL)
				{
					proxy_error("session", "Proxy data socket failed to accept FTP server");
				}
				
				continue;
			}

			ret = session_read_packet(target_session, event_socket);
			if (ret != SESSION_SUCCESS)
			{
				/* Error of processing packet */
				proxy_error("session", "Packet receive error: %d (socket fd: %d)", ret, event_socket);

				break;
			}
		}
	}

	return SESSION_SUCCESS;
}

int session_read_packet(struct session* target_session, int event_socket)
{
	int received_bytes = 0;
	static char buffer[COMMAND_BUFFER_SIZE] = { 0, };
	int port_type = 0;
	int from = 0;

	if ((target_session == NULL) || (event_socket < 0))
	{
		return SERVER_INVALID_PARAM;
	}

	memset(buffer, 0x00, sizeof(buffer));
	received_bytes = packet_read(event_socket, buffer, sizeof(buffer));
	if (received_bytes <= 0)
	{
		socket_del_from_epoll(global_option->epoll_fd, event_socket);
		session_remove_from_list(target_session);

		/* Socket error */
		if (received_bytes == 0)
		{
			/* If connection closed result is not error */
			return SERVER_CONNECTION_CLOSED;
		}

		return SERVER_CONNECTION_ERROR;
	}

	port_type = session_get_client_socket_type(event_socket, target_session->client);

	if (port_type == PORT_TYPE_COMMAND)
	{
		from = FROM_CLIENT;
		client_command_received(target_session, buffer, received_bytes);
	}
	else if (port_type == PORT_TYPE_DATA)
	{
		from = FROM_CLIENT;
		client_data_received(target_session, buffer, received_bytes);
	}
	else
	{
		from = FROM_SERVER;
		port_type = session_get_server_socket_type(event_socket, target_session->server);
		if (port_type == PORT_TYPE_COMMAND)
		{
			server_command_received(target_session, buffer, received_bytes);
		}
		else if (port_type == PORT_TYPE_DATA)
		{
			server_data_received(target_session, buffer, received_bytes);
		}
		else if (port_type == PORT_TYPE_DATA_CONNECTION)
		{
			server_data_connection_received(target_session, buffer, received_bytes);
		}
		else
		{
			return SESSION_INVALID_SOCKET;
		}
	}

	if (from == FROM_CLIENT)
	{
		proxy_error("session", "[client] %s", buffer);
	}
	else
	{
		proxy_error("session", "[server] %s", buffer);
	}

	return SESSION_SUCCESS;
}

