#ifndef PROXY_INCLUDE_THREAD_H_
#define PROXY_INCLUDE_THREAD_H_

#include <pthread.h>

#include "vector.h"

enum thread_error_type
{
	THREAD_SUCCESS,
	THREAD_INVALID,
	THREAD_ALLOC_FAILED
};

struct job
{
	struct job* prev; /* pointer to previous job   */
	void (*function)(void* arg); /* function pointer          */
	void* arg; /* function's argument       */
};

struct job_queue
{
	pthread_mutex_t io_mutex;  /* used for queue r/w access */
	struct job* front_job; /* pointer to front of queue */
	struct job* rear_job;  /* pointer to rear  of queue */
	int job_count;  /* number of jobs in queue   */
};

struct thread
{
	int id;  /* friendly id               */
	pthread_t thread; /* pointer to actual thread  */
	struct thread_pool* parent_pool; /* access to thpool          */
};

struct thread_pool
{
	struct vector* threads;
	int thread_alive_count;
	int thread_working_count;
	struct job_queue* job_queue;
	pthread_mutex_t alive_count_mutex;
};

struct thread_pool* thread_pool_create(int max_threads);
int thread_pool_free(struct thread_pool* target_thread_pool);

#endif

