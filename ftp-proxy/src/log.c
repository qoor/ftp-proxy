#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#include <sys/timeb.h>

static FILE* log_file = NULL;
/* Used to buffer length check to prevent buffer overflow */
static FILE* log_message_size_check_file = NULL;
static char* log_buffer = NULL;

static pthread_mutex_t logfile_mutex;
static pthread_mutex_t error_mutex;

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
	pthread_mutex_init(&error_mutex, NULL);

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
	int ret = 0;

	if (message == NULL)
	{
		return LOG_WRITE_INVALID_MESSAGE;
	}

	if (log_file == NULL)
	{
		return LOG_WRITE_NO_HANDLE;
	}

	ret = ftime(&time_buffer);
	if (ret == -1)
	{
		return LOG_WRITE_TIME_GET_FAILED;
	}
	time = time_buffer.time;
	local_time = localtime(&time);

	pthread_mutex_lock(&logfile_mutex);
	/* Checking timestamp length */
	buffer_length = fprintf(log_message_size_check_file, log_format, local_time->tm_year, local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec, time_buffer.millitm);
	/* */
	
	/* Checking message with va_args length */
	va_start(args, message);
	buffer_length += vfprintf(log_message_size_check_file, message, args);
	va_end(args);
	/* */

	buffer_length += strlen(" ") + strlen("\n"); /* One space between timestamp and message, New line */

	if (buffer_length < 0 || buffer_length >= MAX_LOG_MESSAGE_SIZE)
	{
		memset(log_buffer, 0x00, MAX_LOG_MESSAGE_SIZE);
		pthread_mutex_unlock(&logfile_mutex);
		return LOG_WRITE_INVALID_MESSAGE;
	}

	/* First, logging timestamp */
	buffer_length = sprintf(log_buffer, log_format, local_time->tm_year, local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec, time_buffer.millitm);

	strncat(log_buffer, " ", strlen(" "));
	buffer_length += strlen(" ");
	/* */

	/* Second, logging message with va_args */
	va_start(args, message);
	vsprintf(log_buffer + buffer_length, message, args);
	va_end(args);
	/* */

	/* Last, logging new line */
	strncat(log_buffer, "\n", strlen("\n"));
	/* */

	/* Now write buffer to log file */
	fwrite(log_buffer, sizeof(char), strlen(log_buffer), log_file);
	fflush(log_file);
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
	pthread_mutex_destroy(&logfile_mutex);
	pthread_mutex_destroy(&error_mutex);
}

void proxy_error(const char* tagname, const char* format, ...)
{
	va_list args;

	if (format == NULL)
	{
		return;
	}

	pthread_mutex_lock(&error_mutex);
	if (tagname != NULL)
	{
		fprintf(stderr, "[%s] ", tagname);
	}

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	pthread_mutex_unlock(&error_mutex);
}

