#ifndef __CLIENT_H__
#define __CLIENT_H__

#define BIND_PORT 7777 /* Default port of client connection */

void error_proc(const char* str);
void create_epoll(void);
void create_socket(void);
void listen_epoll(void);
#endif
