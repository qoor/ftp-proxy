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
	struct job* prev;
	void (*function)(void* arg);
	void* arg;
};

struct job_queue
{
	pthread_mutex_t io_mutex;
	struct job* front_job;
	struct job* rear_job;
	int job_count;
};

struct thread
{
	int id;
	pthread_t thread;
	struct thread_pool* parent_pool;
};

struct thread_pool
{
	struct vector* threads;
	int thread_alive_count;
	int thread_working_count;
	struct job_queue* job_queue;
	pthread_mutex_t alive_count_mutex;
};

int thread_pool_free(struct thread_pool* target_thread_pool);

#endif

