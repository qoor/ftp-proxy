#ifndef __PROXY_H__
#define __PROXY_H__

/* 
This header is the header that defines the RETURN code.
[Reference to the availability of error code macros : https://www.gnu.org/software/libc/manual/html_node/Exit-Status.html]
[Using error code macro enum, error code header integrated sample code : https://github.com/curl/curl]
*/

#define MAX_EVENTS 254 /* Amount of file descriptor of monitoring */
#define EVENT_TIMEOUT 100 /* EPOLL event timeout as milliseconds */
#define MIN_CAPACITY_CALC(capacity) ((capacity) * 4 / 3)

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
/* */

/* OPTION RETURN CODE DEFINE */
enum log_option_get_error_type
{
	OPTION_GET_SUCCESS,
	OPTION_GET_FILE_PARSE_ERROR,
	OPTION_GET_ARGS_PARSE_ERROR
};
/* */

/* ARGUMENT RETURN CODE DEFINE */
enum get_argument_error_type
{
	ARGUMENT_SUCCESS,
	ARGUMENT_FAIL,
	ARGUMENT_NOT_FOUND
};
/* */

/* SERVER RETURN CODE DEFINE */
enum server_add_error_type
{
	SERVER_ADD_SUCCESS,
	SERVER_ADD_ALLOC_FAILED,
	SERVER_ADD_INCORRECT_CONNECTION_STRING,
	SERVER_ADD_SOCKET_CREATE_FAILED,
	SERVER_ADD_NO_SERVERS
};

enum server_remove_error_type
{
	SERVER_REMOVE_SUCCESS,
	SERVER_REMOVE_INVALID_SERVER
};

enum servers_polling_error_type
{
	SERVERS_POLLING_SUCCESS,
	SERVERS_POLLING_WAIT_ERROR
};
/* */

/* VECTOR RETURN CODE DEFINE */
enum vector_error_type
{
	VECTOR_SUCCESS,
	VECTOR_INVALID,
	VECTOR_ALLOC_FAILED,
	VECTOR_OUT_OF_BOUNDS
};
/* */

/* HASHMAP RETURN CODE DEFINE */
enum hashmap_error_type
{
	HASHMAP_SUCCESS,
	HASHMAP_INVALID,
	HASHMAP_INVALID_KEY
};

#endif
