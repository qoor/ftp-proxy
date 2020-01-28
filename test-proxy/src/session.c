#include "session.h"

#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "types.h"

static int session_cmp(const struct session* session1, const struct session* session2)
{
	if (session1 == session2)
	{
		return TRUE;
	}

	if (session1 == NULL || session2 == NULL)
	{
		return FALSE;
	}

	return memcmp(session1, session2, sizeof(struct session));
}

int add_session_to_list(struct vector* session_list, int socket_fd, int socket_type, int port_type)
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

	vector_push_back(session_list, new_session);
	return SESSION_SUCCESS;
}

int remove_session_from_list(struct vector* session_list, struct session* target_session)
{
	struct session* current_session = NULL;
	int i = 0;

	if (session_list == NULL)
	{
		return SESSION_INVALID_LIST;
	}

	if (target_session == NULL)
	{
		return SESSION_INVALID_SESSION;
	}

	for ( ; i < session_list->size; ++i)
	{
		current_session = session_list->container[i];
		if (session_cmp(current_session, target_session))
		{
			free(current_session);
			vector_erase(session_list, i);
			return SESSION_SUCCESS;
		}

	}
	
	return SESSION_INVALID_SESSION;
}

struct session* get_session_from_list(const struct vector* session_list, int socket_fd, int socket_type, int port_type)
{
	struct session* current_session = NULL;
	int i = 0;

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

	for ( ; i < session_list->size; ++i)
	{
		current_session = session_list->container[i];

		if (socket_type == SOCKET_TYPE_CLIENT && current_session->client_socket[port_type] == socket_fd)
		{
			return current_session;
		}
		else if (current_session->server_socket[port_type] == socket_fd)
		{
			return current_session;
		}
	}

	errno = SESSION_INVALID_SOCKET;
	return NULL;
}

