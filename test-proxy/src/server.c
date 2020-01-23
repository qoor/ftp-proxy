#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <netinet/ip.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include "server.h"
#include "vector.h"
#include "hashmap.h"
#include "proxy.h"
#include "packet.h"

static void server_free(struct server* target_server);

static int remove_server_loop(void* key, void* value, void* context)
{
	server_free((struct server*)value);
	return 1;
}

/* Add server to destination from connection strings */
int add_servers_from_vector(struct hashmap* dest, const struct vector* connection_strings)
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

		if ((ret = add_server(dest, current_string)))
		{
			break;
		}
	}

	return ret;
}

int add_server(struct hashmap* dest, const char* connection_string)
{
	struct server* server_object = NULL;
	char* colon_pos = NULL;
	char* server_ip = NULL;
	size_t address_length = 0;
	int header_include = 1;

	if (dest == NULL || (server_object = (struct server*)malloc(sizeof(struct server))) == NULL)
	{
		free(server_object);

		return SERVER_ADD_ALLOC_FAILED;
	}

	server_object->socket_fd = -1;
	if ((server_object->socket_address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in))) == NULL)
	{
		server_free(server_object);

		return SERVER_ADD_ALLOC_FAILED;
	}

	memset(server_object->socket_address, 0x00, sizeof(struct sockaddr_in));

	if (connection_string == NULL || (colon_pos = strchr(connection_string, ':')) == NULL)
	{
		server_free(server_object);

		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	address_length = colon_pos - connection_string;
	if ((server_ip = (char*)malloc((address_length + 1) * sizeof(char))) == NULL)
	{
		server_free(server_object);

		return SERVER_ADD_INCORRECT_CONNECTION_STRING;
	}

	strncpy(server_ip, connection_string, address_length + 1);
	server_ip[address_length] = '\0';
	server_object->socket_address->sin_addr.s_addr = htonl(atoi(server_ip));
	server_object->socket_address->sin_port = htons(atoi(colon_pos + 1));
	free(server_ip);

	if ((server_object->socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1 ||
		setsockopt(server_object->socket_fd, IPPROTO_IP, IP_HDRINCL, &header_include, sizeof(header_include)) == -1 ||
		(server_object->epoll_fd = epoll_create(MAX_EVENTS)) == -1)
	{
		server_free(server_object);

		return SERVER_ADD_SOCKET_CREATE_FAILED;
	}

	fcntl(server_object->socket_fd, F_SETFL, O_NONBLOCK);

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
	int epoll_errno = 0;
	struct server* server_ptr = NULL;
	struct iphdr ip_header = { 0, };
	unsigned char* packet = NULL;

	active_events = epoll_wait(epoll_fd, *events, MAX_EVENTS, EVENT_TIMEOUT);

	if (active_events == -1 && (epoll_errno = errno) != EINTR)
	{
		return SERVERS_POLLING_WAIT_ERROR;
	}

	for (eventid = 0; eventid < active_events; ++eventid)
	{
		server_ptr = NULL;
		memset(&ip_header, 0x00, sizeof(struct iphdr));
		packet = NULL;

		if ((server_ptr = get_server_from_fd(server_list, events[eventid]->data.fd)) == NULL)
		{
			/* Invalid socket descriptor */
			continue;
		}

		if (recv(server_ptr->socket_fd, &ip_header, 20, 0) < 20)
		{
			/* Invalid packet */
			continue;
		}

		if ((packet = (unsigned char*)malloc(ip_header.tot_len)) == NULL)
		{
			/* Packet memory allocation failed */
			continue;
		}

		strncpy((char*)packet, (char*)&ip_header, 20);

		if (recv(server_ptr->socket_fd, packet + 20, ip_header.tot_len - 20, 0) < ip_header.tot_len - 20)
		{
			/* Invalid packet */
			continue;
		}

		server_packet_received(server_ptr, packet);

		free(packet);
	}

	return SERVERS_POLLING_SUCCESS;
}

struct server* get_server_from_fd(const struct hashmap* server_list, int fd)
{
	return hashmap_get(server_list, &fd);
}
