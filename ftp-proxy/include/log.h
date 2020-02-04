#ifndef PROXY_INCLUDE_LOG_H_
#define PROXY_INCLUDE_LOG_H_

#define DEFAULT_LOG_FILE_NAME	"proxy"
#define DEFAULT_LOG_FILE_EXT	"log"
#define DEFAULT_LOG_FILE_PATH 	(DEFAULT_LOG_FILE_NAME "." DEFAULT_LOG_FILE_EXT)

#define MAX_LOG_MESSAGE_SIZE	(1024)

#define LOG_TIMESTAMP_PRINT_KOREAN ("[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s")

/* LOG RETURN CODE DEFINE*/
enum log_init_error_type
{
	LOG_INIT_SUCCESS,
	LOG_INIT_FILE_OPEN_FAILED,
	LOG_INIT_BUFFER_ALLOC_FAILED,
	LOG_INIT_ALREADY_INITED
};

enum log_write_error_type
{
	LOG_WRITE_SUCCESS,
	LOG_WRITE_NO_HANDLE,
	LOG_WRITE_TIME_GET_FAILED,
	LOG_WRITE_INVALID_MESSAGE
};


int log_init(void);
int log_write(const char* message, ...);
void log_free(void);
void proxy_error(const char* tagname, const char* format, ...);

#endif

