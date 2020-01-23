#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <netinet/in.h>

/* client info structure */
struct client
{
    int socket_fd;
    int epoll_fd;
    struct sockaddr_in socket_address;
};
/* */

#define MAX_CLIENT_EVENTS 254
#define BIND_CLIENT_PORT 7777
void polling_client(void); /* 임시 함수임 수정 예정 */
#endif
