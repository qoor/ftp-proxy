#ifndef __SERVER_H__
#define __SERVER_H__

/* 서버 정보를 담음 */
struct server
{
	char* address;
	unsigned short port;
};

extern struct server** server_list;
extern int server_count;

void add_server(const char* connection_string);
void reset_server_list(void);

#endif

