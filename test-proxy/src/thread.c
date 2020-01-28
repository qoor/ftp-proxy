#include "thread.h"

#include <errno.h>
#include <malloc.h>

#include <sys/prctl.h>

#include "types.h"

#define MAX_THREAD_NAME_SIZE (128)

static int threads_on_hold = FALSE;
static int threads_keep_alive = TRUE;

static struct job_queue* job_queue_create()
{
	struct job_queue* new_job_queue = NULL;

	errno = 0;

	new_job_queue = (struct job_queue*)malloc(sizeof(struct job_queue));
	if (new_job_queue == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		return NULL;
	}

	new_job_queue->front_job = NULL;
	new_job_queue->rear_job = NULL;
	new_job_queue->job_count = 0;
	pthread_mutex_init(&new_job_queue->io_mutex, NULL);

	return new_job_queue;
}

static int job_queue_push(struct job_queue* target_job_queue, struct job* new_job)
{
	if (new_job == NULL)
	{
		return THREAD_INVALID;
	}

	pthread_mutex_lock(&target_job_queue->io_mutex);
	new_job->prev = NULL;

	if (target_job_queue->job_count == 0)
	{
		target_job_queue->front_job = new_job;
		target_job_queue->rear_job = new_job;
	}
	else
	{
		target_job_queue->rear_job->prev = new_job;
		target_job_queue->rear_job = new_job;
	}
	
	++target_job_queue->job_count;
	pthread_mutex_unlock(&target_job_queue->io_mutex);
	
	return THREAD_SUCCESS;
}

static struct job* job_queue_pull(struct job_queue* current_job_queue)
{
	struct job* current_job = NULL;
	int job_count = 0;

	errno = 0;

	if (current_job_queue == NULL)
	{
		errno = THREAD_INVALID;
		return NULL;
	}

	pthread_mutex_lock(&current_job_queue->io_mutex);
	current_job = current_job_queue->front_job;
	job_count = current_job_queue->job_count;

	if (job_count == 1)
	{
		current_job_queue->front_job = NULL;
		current_job_queue->rear_job = NULL;
		current_job_queue->job_count = 0;
	}
	else
	{
		current_job_queue->front_job = current_job->prev;
		--current_job_queue->job_count;
	}

	pthread_mutex_unlock(&current_job_queue->io_mutex);

	return current_job;
}

static void job_queue_clear(struct job_queue* target_job_queue)
{
	struct job* current_job = NULL;

	if (target_job_queue == NULL)
	{
		return;
	}

	while (target_job_queue->job_count)
	{
		current_job = job_queue_pull(target_job_queue);
		if (current_job != NULL)
		{
			free(current_job);
		}
	}

	target_job_queue->front_job = NULL;
	target_job_queue->rear_job = NULL;
	target_job_queue->job_count = 0;
	pthread_mutex_destroy(&target_job_queue->io_mutex);
}

static void job_queue_free(struct job_queue* target_job_queue)
{
	job_queue_clear(target_job_queue);
}

static void thread_do(struct thread* current_thread)
{
	char thread_name[MAX_THREAD_NAME_SIZE] = { 0, };
	struct thread_pool* parent_thread_pool = NULL;
	void (*function_buffer)(void*) = NULL;
	void* arg_buffer = NULL;
	struct job* pulled_job = NULL;

	if (current_thread == NULL)
	{
		return;
	}

	parent_thread_pool = current_thread->parent_pool;

	sprintf(thread_name, "thread-pool-%d", current_thread->id);
	prctl(PR_SET_NAME, thread_name);

	pthread_mutex_lock(&parent_thread_pool->alive_count_mutex);
	++parent_thread_pool->thread_alive_count;
	pthread_mutex_unlock(&parent_thread_pool->alive_count_mutex);

	while (threads_keep_alive == TRUE)
	{
		if (threads_keep_alive == FALSE)
		{
			break;
		}
		
		pthread_mutex_lock(&parent_thread_pool->alive_count_mutex);
		++parent_thread_pool->thread_working_count;
		pthread_mutex_unlock(&parent_thread_pool->alive_count_mutex);

		pulled_job = job_queue_pull(parent_thread_pool->job_queue);
		if (pulled_job != NULL)
		{
			function_buffer = pulled_job->function;
			arg_buffer = pulled_job->arg;
			function_buffer(arg_buffer);

			free(pulled_job);
		}

		pthread_mutex_lock(&parent_thread_pool->alive_count_mutex);
		--parent_thread_pool->thread_working_count;
		if (parent_thread_pool->thread_working_count == 0)
		{
			/* TODO: Should send signal */
		}
		pthread_mutex_unlock(&parent_thread_pool->alive_count_mutex);
	}

	pthread_mutex_lock(&parent_thread_pool->alive_count_mutex);
	--parent_thread_pool->thread_alive_count;
	pthread_mutex_unlock(&parent_thread_pool->alive_count_mutex);
}

static struct thread* thread_create(struct thread_pool* parent_thread_pool)
{
	struct thread* new_thread = NULL;

	errno = 0;

	if (parent_thread_pool == NULL)
	{
		errno = THREAD_INVALID;
		return NULL;
	}

	new_thread = (struct thread*)malloc(sizeof(struct thread));
	if (new_thread == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		return NULL;
	}

	vector_push_back(parent_thread_pool->threads, new_thread);

	new_thread->parent_pool = parent_thread_pool;
	new_thread->id = parent_thread_pool->threads->size - 1;

	pthread_create(&new_thread->thread, NULL, (void*)thread_do, new_thread);
	pthread_detach(new_thread->thread);

	return new_thread;
}

struct thread_pool* thread_pool_create(int max_threads)
{
	struct thread_pool* new_thread_pool = NULL;
	int i = 0;

	errno = 0;

	if (max_threads < 0)
	{
		max_threads = 0;
	}

	new_thread_pool = (struct thread_pool*)malloc(sizeof(struct thread_pool));
	if (new_thread_pool == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		return NULL;
	}

	new_thread_pool->job_queue = job_queue_create();
	if (new_thread_pool->job_queue == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		thread_pool_free(new_thread_pool);
		return NULL;
	}

	new_thread_pool->threads = vector_init(max_threads);
	if (new_thread_pool->threads == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		thread_pool_free(new_thread_pool);
		return NULL;
	}

	for ( ; i < max_threads; ++i)
	{
		vector_push_back(new_thread_pool->threads, thread_create(new_thread_pool));
	}

	new_thread_pool->thread_alive_count = 0;
	new_thread_pool->thread_working_count = 0;
	pthread_mutex_init(&new_thread_pool->alive_count_mutex, NULL);

	return NULL;
}

int thread_pool_free(struct thread_pool* target_thread_pool)
{
	struct vector* threads = NULL;
	int i = 0;

	if (target_thread_pool == NULL)
	{
		return THREAD_INVALID;
	}

	if (target_thread_pool->job_queue != NULL)
	{
		job_queue_free(target_thread_pool->job_queue);
		free(target_thread_pool->job_queue);
		target_thread_pool->job_queue = NULL;
	}

	if (target_thread_pool->threads != NULL)
	{
		threads = target_thread_pool->threads;
		for (i = 0; i < threads->size; ++i)
		{
			if (threads->container[i] != NULL)
			{
				free(threads->container[i]);
				threads->container[i] = NULL;
			}
		}

		free(threads);
		target_thread_pool->threads = NULL;
	}

	free(target_thread_pool);
	return THREAD_SUCCESS;
}

