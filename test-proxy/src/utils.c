#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "types.h"

/* Skipping whitespace of C string pointer */
void skip_whitespace(char** data)
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

int is_port_command(const char* command, int size)
{
	const char* current_pos = command;
	int value_have = FALSE;
	int delimiter_count = 0;

	if (command == NULL)
	{
		return FALSE;
	}

	if (size < 6) /* `PORT (` */
	{
		return FALSE;
	}

	if (strncmp(current_pos, "PORT", 4) != 0)
	{
		return FALSE;
	}

	while (((current_pos - command) < size) && ((*current_pos != '\r') || (*current_pos != '\n') || (*current_pos != '\0')))
	{
		if (isxdigit(*current_pos) == TRUE)
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

	/* delimiter_count value must same or over than 5
	 * Ex) PORT 1.2.3.4,12,34
	*/

	return (delimiter_count >= 5);
}

int generate_port_command(int socket_fd, char* destination)
{
	struct sockaddr_in address = { 0, };
	unsigned int address_size = sizeof(struct sockaddr);
	char* address_str = NULL;
	char* address_str_pos = NULL;
	uint16_t port = 0;
	char port_str[32] = { 0, };
	int ret = 0;
	
	if (socket_fd < 0 || destination == NULL)
	{
		return -1;
	}
	
	ret = getsockname(socket_fd, (struct sockaddr*)&address, &address_size);
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

	while (*address_str_pos != '\0')
	{
		if (*address_str_pos == '.')
		{
			*address_str_pos = ',';
		}
	}

	strncpy(destination, "PORT ", 6);
	strncat(destination, address_str, strlen(address_str));
	strncat(destination, ",", strlen(","));

	sprintf(port_str, "%d,%d", (port / 256), (port % 256));
	strncat(destination, port_str, strlen(port_str));
	strncat(destination, "\n", strlen("\n"));

	free(address_str);

	return strlen(destination);
}

