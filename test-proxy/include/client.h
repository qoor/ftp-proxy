<<<<<<< HEAD
#ifndef PROXY_INCLUDE_CLIENT_H_
#define PROXY_INCLUDE_CLIENT_H_
=======
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
>>>>>>> 9f6b4b4201e0adefe7f45fb78867ea6266a8d067

#define MAX_CLIENT_EVENTS 254
#define BIND_CLIENT_PORT 7777

void PollingClient(void); /* 임시 함수임 수정 예정 */
#endif
