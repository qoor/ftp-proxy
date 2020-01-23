#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <malloc.h>
#include <string.h>

#include <sys/timeb.h>
#include <pthread.h>

#include "log.h"
#include "proxy.h"

static FILE* log_file = NULL;
/* Used to buffer length check to prevent buffer overflow */
static FILE* log_message_size_check_file = NULL;
static char* log_buffer = NULL;

static pthread_mutex_t logfile_mutex;

/*
 * Initialize log system
 * Return values
 *  0: Success
 *  -1: Failed file open
 * 	-2: Failed allocating buffer
*/
int log_init(void)
{
    if (log_file != NULL)
    {
        return LOG_INIT_ALREADY_INITED;
    }

    log_file = fopen(DEFAULT_LOG_FILE_PATH, "a+");
    if (log_file == NULL)
    {
        return LOG_INIT_FILE_OPEN_FAILED;
    }

	log_message_size_check_file = fopen("/dev/null", "w");
	if (log_message_size_check_file == NULL)
	{
		return LOG_INIT_FILE_OPEN_FAILED;
	}

	log_buffer = (char*)malloc(MAX_LOG_MESSAGE_SIZE * sizeof (char));
	if (log_buffer == NULL)
	{
		log_free();
		return LOG_INIT_BUFFER_ALLOC_FAILED;
	}
	memset(log_buffer, 0x00, MAX_LOG_MESSAGE_SIZE * sizeof(char));

	pthread_mutex_init(&logfile_mutex, NULL);

	return LOG_INIT_SUCCESS;
}

int log_write(const char* message, ...)
{
	struct timeb time_buffer = { 0, };
	time_t time = 0;
	struct tm* local_time = NULL;
	va_list args;
	size_t buffer_length = 0;
	const char* log_format = LOG_TIMESTAMP_PRINT_KOREAN;

	if (message == NULL)
	{
		return LOG_WRITE_INVALID_MESSAGE;
	}

	if (log_file == NULL)
	{
		return LOG_WRITE_NO_HANDLE;
	}

	if (ftime(&time_buffer) == -1) /* If failed to get time */
	{
		return LOG_WRITE_TIME_GET_FAILED;
	}
	time = time_buffer.time;
	local_time = localtime(&time);

	pthread_mutex_lock(&logfile_mutex);

	/* Buffer length check before write buffer to prevent buffer overflow */
	buffer_length = fprintf(log_message_size_check_file, log_format, local_time->tm_year, local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec, time_buffer.millitm, message);
	if (buffer_length >= MAX_LOG_MESSAGE_SIZE)
	{
		return LOG_WRITE_INVALID_MESSAGE;
	}
	/* */

	/* 
	 * Write log
	 * Ex) [2020-01-17 13:46:00.000] Hello World!
	*/
	sprintf(log_buffer,  "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s", local_time->tm_year, local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec, time_buffer.millitm, message);
	/* */

	/* Also buffer length check with arguments */
	va_start(args, log_buffer);
	buffer_length = vfprintf(log_message_size_check_file, log_buffer, args);
	va_end(args);
	if (buffer_length >= MAX_LOG_MESSAGE_SIZE)
	{
		memset(log_buffer, 0x00, MAX_LOG_MESSAGE_SIZE);
		pthread_mutex_unlock(&logfile_mutex);
		return LOG_WRITE_INVALID_MESSAGE;
	}
	/* */

	/* Argument push to buffer */
	va_start(args, log_buffer);
	buffer_length = vsprintf(log_buffer, log_buffer, args);
	va_end(args);
	/* */

	if (buffer_length < 0)
	{
		memset(log_buffer, 0x00, MAX_LOG_MESSAGE_SIZE);
		pthread_mutex_unlock(&logfile_mutex);
		return LOG_WRITE_INVALID_MESSAGE;
	}
	else if (buffer_length >= MAX_LOG_MESSAGE_SIZE)
	{
		log_buffer[MAX_LOG_MESSAGE_SIZE - 1] = '\0'; /* Ensure EOS */
		buffer_length = MAX_LOG_MESSAGE_SIZE;
	}

	/* Write buffer result to log file */
	fwrite(log_buffer, sizeof(char), buffer_length, log_file);
	/* */

	/* */
	pthread_mutex_unlock(&logfile_mutex);

	return LOG_WRITE_SUCCESS;
}

void log_free(void)
{
	pthread_mutex_lock(&logfile_mutex);

	if (log_file != NULL)
	{
		fflush(log_file);
		fclose(log_file);
	}

	if (log_message_size_check_file != NULL)
	{
		fclose(log_message_size_check_file);
	}

	if (log_buffer != NULL)
	{
		free(log_buffer);
	}

	log_file = NULL;
	log_buffer = NULL;
	log_message_size_check_file = NULL;

	pthread_mutex_unlock(&logfile_mutex);
}
