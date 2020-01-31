#include "session.h"

#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "types.h"
#include "server.h"

static int session_socket_cmp(const struct session* source_session, int socket_fd, int socket_type, int port_type)
{
	int source_socket_fd = 0;

	if (socket_type == SOCKET_TYPE_CLIENT)
	{
		if (port_type == PORT_TYPE_COMMAND)
		{
			source_socket_fd = source_session->client_command_socket->fd;
		}
		else
		{
			source_socket_fd = source_session->client_data_socket->fd;
		}
	}
	else
	{
		if (port_type == PORT_TYPE_COMMAND)
		{
			source_socket_fd = source_session->server->command_socket->fd;
		}
		else
		{
			source_socket_fd = source_session->server->data_socket->fd;
		}
	}

	return (socket_fd == source_socket_fd);
}

int add_session_to_list(struct list* session_list, int socket_fd, int socket_type, int port_type)
{
	struct session* new_session = NULL;

	if (session_list == NULL)
	{
		return SESSION_INVALID_LIST;
	}

	if (socket_fd < 0)
	{
		return SESSION_INVALID_SOCKET;
	}

	if ((socket_type != SOCKET_TYPE_CLIENT && socket_type != SOCKET_TYPE_SERVER) ||
		(port_type != PORT_TYPE_DATA && port_type != PORT_TYPE_COMMAND))
	{
		return SESSION_INVALID_PARAMS;
	}

	new_session = (struct session*)malloc(sizeof(struct session));
	if (new_session == NULL)
	{
		return SESSION_ALLOC_FAILED;
	}

	new_session->client_command_socket = NULL;
	new_session->client_data_socket = NULL;
	new_session->server = NULL;

	LIST_ADD(session_list, &new_session->list);

	return SESSION_ADD_SUCCESS;
}

int remove_session_from_list(struct list* session_list, struct session* target_session)
{
	if (session_list == NULL)
	{
		return SESSION_INVALID_LIST;
	}

	if (target_session == NULL)
	{
		return SESSION_INVALID_SESSION;
	}

	if (target_session->client_command_socket != NULL)
	{
		socket_free(target_session->client_command_socket);
		target_session->client_command_socket = NULL;
	}
	if (target_session->client_data_socket != NULL)
	{
		socket_free(target_session->client_data_socket);
		target_session->client_data_socket = NULL;
	}

	if (target_session->server != NULL)
	{
		server_free(target_session->server);
		target_session->server = NULL;
	}

	LIST_DEL(&target_session->list);

	return SESSION_INVALID_SESSION;
}

struct session* get_session_from_list(const struct list* session_list, int socket_fd, int socket_type, int port_type)
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

	if (socket_type != SOCKET_TYPE_CLIENT && socket_type != SOCKET_TYPE_SERVER)
	{
		errno = SESSION_INVALID_PARAMS;
		return NULL;
	}

	if (port_type != PORT_TYPE_DATA && port_type != PORT_TYPE_COMMAND)
	{
		errno = SESSION_INVALID_PARAMS;
		return NULL;
	}

	list_for_each_entry(current_session, session_list, list, struct session*)
	{
		if (session_socket_cmp(current_session, socket_fd, socket_type, port_type) == TRUE)
		{
			return current_session;
		}
	}

	errno = SESSION_INVALID_SOCKET;
	return NULL;
}

