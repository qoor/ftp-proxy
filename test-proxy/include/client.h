#ifndef PROXY_INCLUDE_CLIENT_H_
#define PROXY_INCLUDE_CLIENT_H_

#define MAX_CLIENT_EVENTS 254
#define BIND_CLIENT_PORT 7777

#include <stdio.h>
#include <sys/epoll.h>
#include <netinet/ip.h>

/* Client info structure */
struct client
{
    
};
void polling_client(void); /* 임시 함수임 수정 예정 */
#endif
