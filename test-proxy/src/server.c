#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#include <netinet/ip.h>
#include <netinet/in.h>

#include "proxy.h"
#include "packet.h"
#include "types.h"

static void server_free(struct server* target_server);

static int remove_server_loop(void* key, void* value, void* context)
{
	server_free((struct server*)value);
	return 1;
}

/* Add server to destination from connection strings */
int add_servers_from_vector(struct hashmap* dest, const struct vector* connection_strings, int epoll_fd)
{
	int i = 0;
	int server_count = 0;
	char* current_string = NULL;
	int ret = SERVER_ADD_SUCCESS;

	if (connection_strings == NULL)
	{
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	server_count = connection_strings->size;

	if (server_count == 0)
	{
		return SERVER_ADD_NO_SERVERS;
	}

	for (i = 0; i < server_count; ++i)
	{
		if ((current_string = connection_strings->container[i]) == NULL)
		{
			continue;
		}

		ret = add_server(dest, current_string, epoll_fd);
		if (ret != SERVER_ADD_SUCCESS)
		{
			break;
		}
	}

	return ret;
}

int add_server(struct hashmap* dest, const char* connection_string, int epoll_fd)
{
	struct server* server_object = NULL;
	char* colon_pos = NULL;
	char server_ip[32];
	size_t address_length = 0;
	int header_include = TRUE;
	int flags = 0;
	struct epoll_event epoll_event;

	memset(&epoll_event, 0x00, sizeof(struct epoll_event));
	epoll_event.events = EPOLLIN;

	if (dest == NULL)
	{
		return SERVER_ADD_ALLOC_FAILED;
	}
	if (connection_string == NULL)
	{
		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	server_object = (struct server*)malloc(sizeof(struct server));
	if (server_object == NULL)
	{
		return SERVER_ADD_ALLOC_FAILED;
	}

	server_object->socket_fd = -1;
	server_object->socket_address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	if (server_object->socket_address == NULL)
	{
		server_free(server_object);

		return SERVER_ADD_ALLOC_FAILED;
	}

	memset(server_object->socket_address, 0x00, sizeof(struct sockaddr_in));

	colon_pos = strchr(connection_string, ':');
	if (colon_pos == NULL)
	{
		server_free(server_object);

		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	address_length = colon_pos - connection_string;
	strncpy(server_ip, connection_string, address_length + 1);
	server_ip[address_length] = '\0';
	server_object->socket_address->sin_addr.s_addr = htonl(atoi(server_ip));
	server_object->socket_address->sin_port = htons(atoi(colon_pos + 1));

	server_object->socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (server_object->socket_fd == -1)
	{
		server_free(server_object);

		return SERVER_ADD_SOCKET_CREATE_FAILED;
	}
	if (setsockopt(server_object->socket_fd, IPPROTO_IP, IP_HDRINCL, &header_include, sizeof(header_include)) == -1 ||
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_object->socket_fd, &epoll_event))
	{
		server_free(server_object);

		return SERVER_ADD_SOCKET_CREATE_FAILED;
	}

	flags = fcntl(server_object->socket_fd, F_GETFL, 0);
	fcntl(server_object->socket_fd, F_SETFL, flags | O_NONBLOCK);

	hashmap_insert(dest, &server_object->socket_fd, server_object);
	if (errno != HASHMAP_SUCCESS)
	{
		server_free(server_object);

		return SERVER_ADD_ALLOC_FAILED;
	}

	return SERVER_ADD_SUCCESS;
}

static void server_free(struct server* target_server)
{
	if (target_server->socket_fd != -1)
	{
		close(target_server->socket_fd);
		target_server->socket_fd = -1;
	}

	if (target_server->socket_address != NULL)
	{
		free(target_server->socket_address);
		target_server->socket_address = NULL;
	}

	free(target_server);
}

void reset_server_list(struct hashmap* dest)
{
	hashmap_foreach(dest, remove_server_loop, NULL);
	hashmap_clear(dest);
}

/* 
 * An handler of Packet received from server
 * This function role full valid packet received event alert to client
*/
void server_packet_received(const struct server* server, unsigned char* packet)
{
	/* Require packet receive handling client.c */
	/*struct iphdr* ip_header = (struct iphdr*)packet;
	struct client* dest_client = get_client_from_address(ip_header->daddr);

	client_packet_receive_handler(dest_client, server, packet);
	*/
}

void send_packet_to_server(struct server* dst_server, unsigned char* packet)
{
	change_packet_address(packet, dst_server->socket_address, PACKET_ADDR_CHANGE_TYPE_DEST);
	sendto(dst_server->socket_fd, packet, ((struct iphdr*)packet)->tot_len, 0x0, (struct sockaddr*)dst_server->socket_address, sizeof(struct sockaddr));
}

int servers_polling(int epoll_fd, const struct hashmap* server_list, struct epoll_event** events)
{
	int active_events = 0;
	int eventid = 0;
	struct server* server_ptr = NULL;
	static struct iphdr ip_header = { 0, };
	unsigned char* packet = NULL;

	errno = 0;

	active_events = epoll_wait(epoll_fd, *events, MAX_EVENTS, EVENT_TIMEOUT);
	if (active_events == -1 && errno != EINTR)
	{
		return SERVERS_POLLING_WAIT_ERROR;
	}

	for (eventid = 0; eventid < active_events; ++eventid)
	{
		server_ptr = (struct server*)hashmap_get(server_list, &events[eventid]->data.fd);
		memset(&ip_header, 0x00, sizeof(struct iphdr));

		if (server_ptr == NULL)
		{
			/* Invalid socket descriptor */
			continue;
		}

		if (get_packet_from_server(server_ptr, &ip_header, &packet) != PACKET_SUCCESS)
		{
			/* Not valid packet */
			continue;
		}

		server_packet_received(server_ptr, packet);
		free(packet);
		packet = NULL;
	}

	return SERVERS_POLLING_SUCCESS;
}

/*
 * Filtering packet from all of server packets
 * This function role filtering FTP (data/command) packet from all of server source packets
*/ 
int get_packet_from_server(const struct server* server, struct iphdr* ip_header, unsigned char** packet)
{	
	struct tcphdr* tcp_header = NULL;

	if (recv(server->socket_fd, ip_header, 20, 0) < 20 || ip_header->tot_len < 40)
	{
		/* Invalid packet */
		return PACKET_INVALID;
	}
	if (ip_header->protocol != IPPROTO_TCP)
	{
		return PACKET_INVALID;
	}

	*packet = (unsigned char*)malloc(ip_header->tot_len);
	if (*packet == NULL)
	{
		/* Packet memory allocation failed */
		return PACKET_ALLOC_FAILED;
	}

	memcpy((char*)*packet, (char*)&ip_header, sizeof(char));
	if (recv(server->socket_fd, *packet + 20, 20, 0) < 20)
	{
		/* Failed to try receiving TCP Header */
		free(*packet);

		return PACKET_INVALID;
	}

	tcp_header = (struct tcphdr*)*packet + 20;
	if (tcp_header->source != FTP_COMMAND_PORT && tcp_header->source != FTP_DATA_PORT)
	{
		/* Is not a FTP packet */
		free(*packet);

		return PACKET_INVALID;
	}

	if (recv(server->socket_fd, *packet + 40, ip_header->tot_len - 40, 0) < ip_header->tot_len - 40)
	{
		/* Failed to trying to get packet data */
		free(*packet);

		return PACKET_INVALID;
	}

	return PACKET_SUCCESS;
}

