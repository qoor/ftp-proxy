#ifndef __CLIENT_H__
#define __CLIENT_H__

#define BIND_PORT 7777 /* 클라이언트가 로드 밸런스에게 연결할 기본 포트*/
#define MAX_EVENTS 254 /* 모니터링 할 파일 디스크립터의 수 지정 */

void error_proc(const char* str);
void create_epoll(void);
void create_socket(void);
void listen_epoll(void);
#endif