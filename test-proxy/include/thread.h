#ifndef PROXY_INCLUDE_THREAD_H_
#define PROXY_INCLUDE_THREAD_H_

#include <pthread.h>

#include "vector.h"

enum thread_error_type
{
	THREAD_SUCCESS,
	THREAD_INVALID,
	THREAD_INVALID_PARAM,
	THREAD_ALLOC_FAILED
};

struct binary_sem
{
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	int state_value;
};

struct job
{
	struct job* prev; /* pointer to previous job   */
	void (*function)(void* arg); /* function pointer          */
	void* arg; /* function's argument       */
};

struct job_queue
{
	pthread_mutex_t io_mutex;
	struct job* front_job;
	struct job* rear_job;
	struct binary_sem* has_jobs;
	int job_count;
};

struct thread
{
	int id;  /* friendly id               */
	pthread_t thread; /* pointer to actual thread  */
	struct thread_pool* parent_pool; /* access to thpool          */
};

struct thread_pool
{
	struct thread** threads;
	int thread_alive_count;
	int thread_working_count;
	struct job_queue* job_queue;
	pthread_mutex_t thread_count_mutex;
	pthread_cond_t threads_all_idle;
};

struct thread_pool* thread_pool_create(int max_threads);
int thread_pool_free(struct thread_pool* target_thread_pool);
int thread_pool_wait(struct thread_pool* target_thread_pool);
int thread_pool_add_work(struct thread_pool* target_thread_pool, void (*function_ptr)(void*), void* arg_ptr);

#endif

