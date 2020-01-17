#ifndef __LOG_H__
#define __LOG_H__

#define DEFAULT_LOG_FILE_NAME	"proxy"
#define DEFAULT_LOG_FILE_EXT	"log"
#define DEFAULT_LOG_FILE_PATH 	("" DEFAULT_LOG_FILE_NAME "." DEFAULT_LOG_FILE_EXT)
#define MAX_LOG_MESSAGE_SIZE	(1024)

#define LOG_INIT_SUCCESS 				0
#define LOG_INIT_FILE_OPEN_FAILED 		-1
#define LOG_INIT_BUFFER_ALLOC_FAILED	-2
#define LOG_INIT_ALREADY_INITED			-3

#define LOG_WRITE_SUCCESS			0
#define LOG_WRITE_NO_HANDLE			-1
#define LOG_WRITE_TIME_GET_FAILED	-2

int log_init(void);
int log_write(const char* message);
void log_free(void);

#endif
