#include "StdInc.h"

int i = 0;
/* epoll 사용을 위한 변수선언, epoll_event 구조체 선언 */
int epoll_file_descriptor = 0, ready = 0, read_file_descriptor = 0;
struct epoll_event event = { 0, };
struct epoll_event events[MAX_EVENTS] = { 0, };
/* socket 사용을 위한 변수선언, 구조체 선언 */
int listen_file_descriptor = 0, connect_file_descriptor = 0;
struct sockaddr_in server_address = { 0, };

/*
멀티 플렉싱 기반의 다중 서버 : kernel 에서는 하나의 스레드가 여러 개의 소켓(파일)을 핸들링 할 수 있는 select, poll, epoll 같은 시스템 콜을 제공한다.
epoll 방식으로 멀티 플렉싱 기반의 다중 서버를 구현하였습니다.
*/

void create_epoll()
{
	epoll_file_descriptor = epoll_create(MAX_EVENTS); /* epoll 객체 생성 */
	if (epoll_file_descriptor == -1)
	{
		error_proc("epoll_create");
	}
}

void create_socket()
{
	listen_file_descriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_file_descriptor == -1)
	{
		error_proc("socket_create");
	}
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_family = AF_INET;
	/*server_address.sin_port = htons(atoi(BIND_PORT));*/
	server_address.sin_port = BIND_PORT;
}

void listen_epoll()
{
	if (bind(listen_file_descriptor, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
	{
		error_proc("bind");
	}
	if (listen(listen_file_descriptor, 5) < 0)
	{
		error_proc("listen");
	}

	event.events = EPOLLIN;
	event.data.fd = listen_file_descriptor; /* 파일 디스크립터를 epoll 인스턴스에 등록한다 (관찰대상의 관찰 이벤트 유형은 EPOLLIN) */
	if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, listen_file_descriptor, &event) == -1)
	{
		error_proc("epoll_ctl");
	}

	printf("Monitoring ...\n");
	while (1)
	{
		ready = epoll_wait(epoll_file_descriptor, events, MAX_EVENTS, -1); /* 모니터링 시작 */
		if (ready == -1)
		{
			if (errno == EINTR)
			{
				continue; /* 인터럽트인 경우 계속 실행 */
			}
			else
			{
				error_proc("epoll_wait");
			}
		}
	}
	close(listen_file_descriptor);
	close(epoll_file_descriptor);
}

void error_proc(const char* str)
{
	fprintf(stderr, "\n [%s]: [%s] \n", str, strerror(errno));
	exit(1);
}


int main()
{
	create_epoll();
	create_socket();
	listen_epoll();
}