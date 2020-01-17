#include "StdInc.h"
#include "log.h"

#include <time.h>
#include <sys/timeb.h>

#define DEFAULT_LOG_FILE_PATH "proxy.log"
#define MAX_LOG_MESSAGE_LENGTH 1024

static FILE* log_file = NULL;
static char* log_buffer;

/*
 * Initialize log system
 * Return values
 *  1: Success
 *  0: Failed
*/
int log_init()
{
    if (log_file)
    {
        return 0;
    }

    log_file = fopen(DEFAULT_LOG_FILE_PATH, "a+");

    if (!log_file)
    {
        return 0;
    }

	log_buffer = (char*)malloc(MAX_LOG_MESSAGE_LENGTH);
	log_buffer[0] = '\0';
	return 1;
}

void log_write(const char* message)
{
	struct timeb time_buffer;
	time_t time;
	struct tm* local_time;

	assert(message);

	if (!log_file)
	{
		return;
	}

	ftime(&time_buffer);

	time = time_buffer.time;
	local_time = localtime(&time);

	/* 
	 * Write log
	 * Ex) [2020-01-17 13:46:00.000] Hello World!
	*/
	sprintf(log_buffer, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s", local_time->tm_year, local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec, time_buffer.millitm, message);
	fputs(log_buffer, log_file);
}

int log_free()
{
	if (!log_file)
	{
		return 0;
	}

	fflush(log_file);
	fclose(log_file);
	free(log_buffer);

	log_file = NULL;
	log_buffer = NULL;
	return 1;
}
