#ifndef __SERVER_H__
#define __SERVER_H__

/* Server info structure */
struct server
{
	int socket_fd;
	struct sockaddr_in socket_address;
};
/* */

int add_servers_from_vector(struct vector* dest, const struct vector* connection_strings);
int add_server(struct vector* dest, const char* connection_string);
void reset_server_list(void);
void send_packet_to_server(struct server* dst_server, unsigned char* packet);

#endif
