#include "session.h"

#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

#include "types.h"
#include "client.h"
#include "server.h"
#include "log.h"
#include "option.h"

extern struct option* global_option; /* For debugging */

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

static int socket_add_to_epoll(int epoll_fd, int socket_fd)
{
	struct epoll_event event = { 0, };
	int ret = 0;

	if ((epoll_fd == -1) || (socket_fd == -1))
	{
		return SESSION_INVALID_SOCKET;
	}

	event.events = EPOLLIN;
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
	if (ret < 0)
	{
		return SESSION_EPOLL_CTL_FAILED;
	}

	return SESSION_SUCCESS;
}

static int process_client_event(struct session* target_session, int epoll_fd, int event_socket)
{
	int ret = 0;

	if (event_socket == target_session->client->data_socket->fd)
	{
		/* Data transfer */
	}
	else if (event_socket == target_session->client->command_socket->fd)
	{
		/* If this event occurred cause by command socket of connected client event */
	}
	else
	{
		proxy_error("session", "Invalid client socket fd: %d", event_socket);

		return SESSION_INVALID_SOCKET;
	}

	return SESSION_SUCCESS;
}

static int process_server_event(struct session* target_session, int epoll_fd, int event_socket)
{
	struct sockaddr_in client_address = { 0, };
	int ret = 0;

	if (event_socket == target_session->server->command_socket->fd)
	{
		ret = server_read_packet(target_session, PORT_TYPE_COMMAND);
		if (ret != SERVER_SUCCESS)
		{
			socket_del_from_epoll(epoll_fd, event_socket);
			session_remove_from_list(target_session);

			return ret;
		}
	}
	else if (event_socket == target_session->client->data_socket->fd)
	{
		/* If FTP server request connect to proxy data socket */
		ret = server_accept(target_session->server, &client_address);
		if (ret <= 0)
		{
			return SESSION_INVALID_SOCKET;
		}

		ret = socket_add_to_epoll(epoll_fd, event_socket, EPOLLIN);
		if (ret < 0)
		{
			return SESSION_EPOLL_CTL_FAILED;
		}
	}
	else
	{
		/* If this event occurred cause by data socket of connected FTP server event */
		ret = server_read_packet(target_session, PORT_TYPE_DATA);
		if (ret != SERVER_SUCCESS)
		{
			socket_del_from_epoll(epoll_fd, event_socket);
			session_remove_from_list(target_session);
			
			return ret;
		}
	}

	return SESSION_SUCCESS;
}

static int is_socket_in_session(const struct session* source_session, int socket_fd)
{
	if (source_session == NULL)
	{
		return FALSE;
	}

	if (socket_fd < 0)
	{
		return FALSE;
	}

	if ((source_session->client->command_socket->fd == socket_fd) ||
		(source_session->client->data_socket->fd == socket_fd))
	{
		return TRUE;
	}
	
	if ((source_session->server->command_socket->fd == socket_fd) ||
		(source_session->server->data_socket->fd == socket_fd))
	{
		return TRUE;
	}

	return FALSE;
}

static int session_add_socket_to_epoll(int epoll_fd, const struct session* target_session)
{
	struct epoll_event event = { 0, };
	int ret = 0;

	event.events = EPOLLIN;
	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_session->client->command_socket->fd, &event);
	if (ret < 0)
	{
		return SESSION_EPOLL_CTL_FAILED;
	}

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_session->client->data_socket->fd, &event);
	if (ret < 0)
	{
		return SESSION_EPOLL_CTL_FAILED;
	}

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_session->server->command_socket->fd, &event);
	if (ret < 0)
	{
		return SESSION_EPOLL_CTL_FAILED;
	}

	ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, target_session->server->data_socket->fd, &event);
	if (ret < 0)
	{
		return SESSION_EPOLL_CTL_FAILED;
	}

	return SESSION_SUCCESS;
}

static struct session* add_session_to_list(struct list* session_list, int epoll_fd, int event_socket)
{
	struct session* new_session = NULL;
	struct client* new_client = NULL;
	struct server* new_server = NULL;
	struct sockaddr_in client_address = { 0, };
	uint32_t address_length = sizeof(struct sockaddr);
	const struct sockaddr_in* server_address = NULL;
	int ret = 0;

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

	/* new_client = client_create(event_socket); */
	if (new_client == NULL)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	ret = accept(event_socket, (struct sockaddr*)&client_address, &address_length);
	if (ret < 0)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	new_client->command_socket->fd = ret;

	server_address = server_get_available_address(&global_option->server_list);
	if (server_address == NULL)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	new_server = server_create(server_address);
	if (new_server == NULL)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	ret = session_add_socket_to_epoll(epoll_fd, new_session);
	if (ret != SESSION_SUCCESS)
	{
		session_remove_from_list(new_session);

		return NULL;
	}

	LIST_ADD(session_list, &new_session->list);

	return new_session;
}

int session_remove_from_list(struct session* target_session)
{
	if (target_session == NULL)
	{
		return SESSION_INVALID_SESSION;
	}

	if (target_session->client->command_socket != NULL)
	{
		socket_free(target_session->client->command_socket);
		target_session->client->command_socket = NULL;
	}
	if (target_session->client->data_socket != NULL)
	{
		socket_free(target_session->client->data_socket);
		target_session->client->data_socket = NULL;
	}

	if (target_session->client != NULL)
	{
		/*
		 * client_free(target_session->client);
		 * target_session->client = NULL;
		*/
	}

	if (target_session->server != NULL)
	{
		server_free(target_session->server);
		target_session->server = NULL;
	}

	LIST_DEL(&target_session->list);

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

int session_polling(int epoll_fd, struct list* session_list, int client_command_socket)
{
	int active_event_count = 0;
	int event_id = 0;
	int event_socket = -1;
	static struct epoll_event events[MAX_EVENTS] = { { 0, } };
	struct session* target_session = NULL;
	int ret = 0;

	active_event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, EVENT_TIMEOUT);
	if (active_event_count == -1)
	{
		return SERVER_POLLING_WAIT_ERROR;
	}

	for (event_id = 0; event_id < active_event_count; ++event_id)
	{
		event_socket = events[event_id].data.fd;
		target_session = get_session_from_list(session_list, event_socket);

		if (target_session == NULL)
		{
			if (event_socket == client_command_socket)
			{
				target_session = add_session_to_list(session_list, epoll_fd, event_socket);
			}

			if (target_session == NULL)
			{
				/* Invalid socket */
				proxy_error("session", "Target session from socket %d not found", event_socket);
			}

			continue;
		}

		ret = process_client_event(target_session, epoll_fd, event_socket);
		if (ret != SESSION_SUCCESS)
		{
			ret = process_server_event(target_session, epoll_fd, event_socket);
			if (ret != SESSION_SUCCESS)
			{
				/* Error of processing packet */
				proxy_error("session", "Unknown error [Error: %d]", ret);

				continue;
			}
		}	
	}

	return SESSION_SUCCESS;
}


