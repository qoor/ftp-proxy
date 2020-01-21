#include "StdInc.h"

int i = 0;
/* variable declaration for use of epoll, epoll_event structure declaration */
int epoll_file_descriptor = 0, ready = 0, read_file_descriptor = 0;
struct epoll_event event = { 0, };
struct epoll_event events[MAX_EVENTS] = { 0, };
/* Variable declaration for socket use, structure declaration */
int listen_file_descriptor = 0, connect_file_descriptor = 0;
int client_address_len = 0;
struct sockaddr_in server_address = { 0, }, client_address = {0, } ;

/*
Multiple servers based on multi-flecking: kernel provides system calls such as select, pol, and epoll, where a thread can handle multiple sockets (files).
We deployed multiple servers based on multiplexing in an epol method.
*/

void create_epoll()
{
	epoll_file_descriptor = epoll_create(MAX_EVENTS); /* Create an epoll object */
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
	if (bind(listen_file_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
	{
		error_proc("bind");
	}
	if (listen(listen_file_descriptor, 5) < 0)
	{
		error_proc("listen");
	}

	event.events = EPOLLIN;
	event.data.fd = listen_file_descriptor; /* Register the file disk reader with the epoll instance (EpOLLIN for observation) */
	if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, listen_file_descriptor, &event) == -1)
	{
		error_proc("epoll_ctl");
	}

	printf("Monitoring ...\n");
	while (1)
	{
		ready = epoll_wait(epoll_file_descriptor, events, MAX_EVENTS, -1); /* Start Monitoring */
		if (ready == -1)
		{
			if (errno == EINTR)
			{
				continue; /* Continue if interrupt */
			}
			else
			{
				error_proc("epoll_wait");
			}
		}
		for (i = 0; i < ready; i++)
		{
			if (events[i].data.fd = listen_file_descriptor) /* accept a client */
			{
				connect_file_descriptor = accept(listen_file_descriptor, (struct sockaddr *)&client_address, &client_address_len);
				if (connect_file_descriptor == -1)
				{
					fprintf(stderr, "Accept ERROR \n");
					continue;
				}
				fprintf(stderr, " A client is connected... \n");
				event.data.fd = connect_file_descriptor;
				if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, connect_file_descriptor, &event) == -1)
				{
					error_proc("epoll_ctl");
				}
				else
				{
					/* run code */
				}
			}
		}
	}
	close(listen_file_descriptor);
	close(epoll_file_descriptor);
}

void error_proc(const char *str)
{
	fprintf(stderr, "\n [%s]: [%s] \n", str, strerror(errno));
	exit(1);
}