#ifndef __SERVER_H__
#define __SERVER_H__

struct server
{
	char* address;
	unsigned short port;
};

extern struct server** server_list;
extern int server_count;

void add_server(const char* connection_string);
