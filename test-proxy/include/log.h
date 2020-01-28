#ifndef PROXY_INCLUDE_LOG_H_
#define PROXY_INCLUDE_LOG_H_

#define DEFAULT_LOG_FILE_NAME	"proxy"
#define DEFAULT_LOG_FILE_EXT	"log"
#define DEFAULT_LOG_FILE_PATH 	(DEFAULT_LOG_FILE_NAME "." DEFAULT_LOG_FILE_EXT)

#define MAX_LOG_MESSAGE_SIZE	(1024)

#define LOG_TIMESTAMP_PRINT_KOREAN ("[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s")

int log_init(void);
int log_write(const char* message, ...);
void log_free(void);
void proxy_error(const char* tagname, const char* format, ...);

#endif

