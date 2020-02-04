#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include <sys/socket.h>
#include <netinet/in.h>
#define __USE_MISC
#include <arpa/inet.h>

#include "types.h"

/* Skipping whitespace of C string pointer */
void skip_whitespace(char **data)
{
	if (data == NULL || *data == NULL)
	{
		return;
	}

	while (**data != '\0' && (**data == '\t' || **data == ' '))
	{
		++(*data);
	}
}

int is_port_command(const char *command, int size)
{
	const char *current_pos = command;
	int value_have = FALSE;
	int delimiter_count = 0;

	if (command == NULL)
	{
		return FALSE;
	}

	if (size < 5) /* `PORT ` */
	{
		return FALSE;
	}

	if (strncmp(current_pos, "PORT", 4) != 0)
	{
		return FALSE;
	}

	current_pos += 5;

	while (((current_pos - command) < size) && (*current_pos != '\r') && (*current_pos != '\n') && (*current_pos != '\0'))
	{
		if (isxdigit(*current_pos) != FALSE)
		{
			value_have = TRUE;
		}
		else if ((*current_pos == ',') || (*current_pos == '.'))
		{
			if (value_have == FALSE)
			{
				return FALSE;
			}

			++delimiter_count;
			value_have = FALSE;
		}

		++current_pos;
	}

	/* 
	 * delimiter_count value must same or over than 5
	 * Ex) PORT IP AD,DR,ES,S,PORT1,PORT2
	 * PORT 1 X 256 + PORT2 = Destination PORT
	*/

	return (delimiter_count == 5);
}

int generate_port_command(int socket_fd, char *destination)
{
	struct sockaddr_in address = { 0, };
	unsigned int address_size = sizeof(struct sockaddr);
	char *address_str = NULL;
	char *address_str_pos = NULL;
	uint16_t port = 0;
	char port_str[32] = { 0, };
	int ret = 0;

	if (socket_fd < 0 || destination == NULL)
	{
		return -1;
	}

	ret = getsockname(socket_fd, (struct sockaddr *)&address, &address_size);
	if (ret < 0)
	{
		return -2;
	}

	port = ntohs(address.sin_port);

	address_str = inet_ntoa(address.sin_addr);
	address_str_pos = address_str;
	if (address_str == NULL)
	{
		return -3;
	}

	while ((*address_str_pos != '\r') && (*address_str_pos != '\n') && (*address_str_pos != '\0'))
	{
		if (*address_str_pos == '.')
		{
			*address_str_pos = ',';
		}

		++address_str_pos;
	}

	strncpy(destination, "PORT ", 5 + 1); /* Include NULL */
	strncat(destination, address_str, strlen(address_str));
	strncat(destination, ",", strlen(","));

	sprintf(port_str, "%d,%d", (port / 256), (port % 256));
	strncat(destination, port_str, strlen(port_str));
	strncat(destination, "\n", strlen("\n"));

	return strlen(destination);
}

struct sockaddr_in *get_address_from_port_command(char *buffer, int received_bytes)
{
	char *addrstr = NULL;
	char *p = NULL;
	char *portstr = NULL;
	int commas = 0;
	int rc = 0;
	short port = 0;
	unsigned long val = 0;
	struct sockaddr_in* addr = NULL;

	addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	if (addr == NULL)
	{
		return NULL;
	}

	memset(addr, 0x00, sizeof(struct sockaddr_in));

	/* dup the args since they are const and we need to change it */
	addrstr = (char*)malloc(received_bytes + 1);
	if (addrstr == NULL)
	{
		free(addr);
		return NULL;
	}
	
	strncpy(addrstr, buffer, received_bytes);
	addrstr[received_bytes] = '\0';

	/* replace a,b,c,d,e,f with a.b.c.d\0e.f */
	for (p = addrstr; *p; ++p)
	{
		if (*p == ',')
		{
			if (commas != 3)
				*p = '.';
			else
			{
				*p = 0;
				portstr = p + 1;
			}
			++commas;
		}
	}

	/* make sure we got the right number of values */
	if (commas != 5)
	{
		free(addr);
		free(addrstr);
		return NULL;
	}

	/* parse the address */
	rc = inet_aton(addrstr, &addr->sin_addr);
	if (rc == 0)
	{
		free(addr);
		free(addrstr);
		return NULL;
	}

	/* parse the port */
	val = 0;
	port = 0;
	for (p = portstr; *p; ++p)
	{
		if (!isdigit((int)*p))
		{
			if ((*p == '\r') || (*p == '\n'))
			{
				continue;
			}

			if (p == portstr || *p != '.' || val > 0xFF)
			{
				free(addrstr);
				return NULL;
			}
			port <<= 8;
			port += val;
			val = 0;
		}
		else
		{
			val *= 10;
			val += *p - '0';
		}
	}

	/* validate the port */
	if (val > 0xFF || port > 0xFF)
	{
		free(addr);
		free(addrstr);
		return NULL;
	}
	port <<= 8;
	port += val;

	/* fill in the address port and family */
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	free(addrstr);
	return addr;

}
