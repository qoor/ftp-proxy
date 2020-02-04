#include "packet.h"

#include <unistd.h>
#include <errno.h>

#include "types.h"
#include "log.h"

int packet_read(int fd, void* buffer, size_t size)
{
	int ret = 0;
	int error = 0;

	do {
		ret = read(fd, buffer, size);
		error = errno;
	} while (ret < 0 && error == EINTR);

	return ret;
}

int packet_full_read(int socket_fd, void* buffer, size_t size)
{
	int ret = 0;
	int read_size = 0;

	while (TRUE)
	{
		ret = packet_read(socket_fd, (char*)buffer + read_size, size);
		if (ret < 0)
		{
			return ret;
		}
		else if (ret == 0)
		{
			return read_size;
		}

		if (ret > size)
		{
			/* Overflow */
			return ret;
		}

		read_size += ret;
		size -= (size_t)ret;
		if (size == 0)
		{
			return read_size;
		}
	}

	return read_size;
}

int packet_write(int socket_fd, const void* buffer, size_t size)
{
	int ret = 0;
	int error = 0;

	do {
		ret = write(socket_fd, buffer, size);
		error = errno;
	} while (ret < 0 && error == EINTR);

	return ret;
}

int packet_full_write(int socket_fd, const void* buffer, size_t size)
{
	int ret = 0;
	int written_size = 0;

	while (TRUE)
	{
		ret = packet_write(socket_fd, buffer + written_size, size);
		if (ret < 0)
		{
			return ret;
		}
		else if (ret == 0)
		{
			return written_size;
		}

		if (ret > size)
		{
			/* Overflow */
			return ret;
		}

		written_size += ret;
		size -= (size_t)ret;
		if (size == 0)
		{
			return written_size;
		}
	}

	return written_size;
}

