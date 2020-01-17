#include "StdInc.h"
#include "log.h"

#include <time.h>
#include <sys/timeb.h>

static FILE* log_file = NULL;
static char* log_buffer = NULL;

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

	log_buffer = (char*)malloc(MAX_LOG_MESSAGE_SIZE * sizeof (char));
	if (log_buffer == NULL)
	{
		log_free();
		return LOG_INIT_BUFFER_ALLOC_FAILED;
	}
	memset(log_buffer, 0x00, MAX_LOG_MESSAGE_SIZE * sizeof(char));

	return LOG_INIT_SUCCESS;
}

int log_write(const char* message)
{
	struct timeb time_buffer = { 0, };
	time_t time = 0;
	struct tm* local_time = NULL;

	assert(message);

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

	/* 
	 * Write log
	 * Ex) [2020-01-17 13:46:00.000] Hello World!
	*/
	sprintf(log_buffer, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s", local_time->tm_year, local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec, time_buffer.millitm, message);
	fputs(log_buffer, log_file);

	return LOG_WRITE_SUCCESS;
}

void log_free(void)
{
	if (log_file != NULL)
	{
		fflush(log_file);
		fclose(log_file);
	}

	if (log_buffer != NULL)
	{
		free(log_buffer);
	}

	log_file = NULL;
	log_buffer = NULL;
}
