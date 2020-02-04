#ifndef PROXY_INCLUDE_UTILS_H_
#define PROXY_INCLUDE_UTILS_H_

#include "socket.h"

void skip_whitespace(char** data);
int is_port_command(const char* command, int size);
int generate_port_command(int socket_fd, char* destination);
struct sockaddr_in *get_address_from_port_command(char *buffer, int received_bytes);

#endif

