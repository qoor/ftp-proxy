#ifndef PROXY_INCLUDE_CLIENT_H_
#define PROXY_INCLUDE_CLIENT_H_

#define MAX_CLIENT_EVENTS 254
#define BIND_CLIENT_PORT 7777

/* CLIENT RETURN CODE DEFINE */
enum client_error_type
{
	POLLING_SUCCESS,
	EPOLL_CREATE_FAILED,
	SOCKET_CREATE_FAILED,
	SOCKET_BIND_FAILED,
	SOCKET_LISTEN_FAILED,
	EPOLL_CTL_FAILED,
	EPOLL_WAIT_FAILED
};
/* */

#endif
