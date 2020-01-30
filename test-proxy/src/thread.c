#include "thread.h"

#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <sys/prctl.h>

#include "types.h"

#define MAX_THREAD_NAME_SIZE (128)

static int threads_keep_alive = TRUE;

static struct binary_sem* binary_sem_create(int state_value)
{
	struct binary_sem* new_binary_sem = NULL;

	errno = 0;

	if (state_value < 0 || state_value > 1)
	{
		errno = THREAD_INVALID_PARAM;
		return NULL;
	}

	new_binary_sem = (struct binary_sem*)malloc(sizeof(struct binary_sem));
	if (new_binary_sem == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		return NULL;
	}
	
	pthread_mutex_init(&new_binary_sem->mutex, NULL);
	pthread_cond_init(&new_binary_sem->condition, NULL);

	return new_binary_sem;
}

static int binary_sem_reset(struct binary_sem** target_binary_sem)
{
	if (target_binary_sem == NULL || *target_binary_sem == NULL)
	{
		return THREAD_INVALID;
	}

	free(*target_binary_sem);
	*target_binary_sem = binary_sem_create(0);
	if (*target_binary_sem == NULL)
	{
		return THREAD_ALLOC_FAILED;
	}

	return THREAD_SUCCESS;
}

static int binary_sem_post(struct binary_sem* target_binary_sem)
{
	if (target_binary_sem == NULL)
	{
		return THREAD_INVALID;
	}

	pthread_mutex_lock(&target_binary_sem->mutex);
	target_binary_sem->state_value = 1;
	pthread_cond_signal(&target_binary_sem->condition);
	pthread_mutex_unlock(&target_binary_sem->mutex);

	return THREAD_SUCCESS;
}

static int binary_sem_post_broadcast(struct binary_sem* target_binary_sem)
{
	if (target_binary_sem == NULL)
	{
		return THREAD_INVALID;
	}

	pthread_mutex_lock(&target_binary_sem->mutex);
	target_binary_sem->state_value = 1;
	pthread_cond_broadcast(&target_binary_sem->condition);
	pthread_mutex_unlock(&target_binary_sem->mutex);

	return THREAD_SUCCESS;
}

static int binary_sem_wait(struct binary_sem* target_binary_sem)
{
	if (target_binary_sem == NULL)
	{
		return THREAD_INVALID;
	}

	pthread_mutex_lock(&target_binary_sem->mutex);
	while (target_binary_sem->state_value != 1)
	{
		pthread_cond_wait(&target_binary_sem->condition, &target_binary_sem->mutex);
	}

	target_binary_sem->state_value = 0;
	pthread_mutex_unlock(&target_binary_sem->mutex);

	return THREAD_SUCCESS;
}

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

	new_job_queue->has_jobs = binary_sem_create(0);
	if (new_job_queue->has_jobs == NULL)
	{
		free(new_job_queue);
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
	binary_sem_post(target_job_queue->has_jobs);

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

		binary_sem_post(current_job_queue->has_jobs);
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
	binary_sem_reset(&target_job_queue->has_jobs);
	target_job_queue->job_count = 0;
}

static void job_queue_free(struct job_queue* target_job_queue)
{
	job_queue_clear(target_job_queue);
	free(target_job_queue->has_jobs);
	free(target_job_queue);
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

	pthread_mutex_lock(&parent_thread_pool->thread_count_mutex);
	++parent_thread_pool->thread_alive_count;
	pthread_mutex_unlock(&parent_thread_pool->thread_count_mutex);

	while (threads_keep_alive == TRUE)
	{
		binary_sem_wait(parent_thread_pool->job_queue->has_jobs);

		if (threads_keep_alive == FALSE)
		{
			break;
		}
		
		pthread_mutex_lock(&parent_thread_pool->thread_count_mutex);
		++parent_thread_pool->thread_working_count;
		pthread_mutex_unlock(&parent_thread_pool->thread_count_mutex);

		pulled_job = job_queue_pull(parent_thread_pool->job_queue);
		if (pulled_job != NULL)
		{
			function_buffer = pulled_job->function;
			arg_buffer = pulled_job->arg;
			function_buffer(arg_buffer);

			free(pulled_job);
		}

		pthread_mutex_lock(&parent_thread_pool->thread_count_mutex);
		--parent_thread_pool->thread_working_count;
		if (parent_thread_pool->thread_working_count == 0)
		{
			pthread_cond_signal(&parent_thread_pool->threads_all_idle);
		}
		pthread_mutex_unlock(&parent_thread_pool->thread_count_mutex);
	}

	pthread_mutex_lock(&parent_thread_pool->thread_count_mutex);
	--parent_thread_pool->thread_alive_count;
	pthread_mutex_unlock(&parent_thread_pool->thread_count_mutex);
}

static struct thread* thread_create(struct thread_pool* parent_thread_pool, int thread_id)
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

	parent_thread_pool->threads[thread_id] = new_thread;

	new_thread->parent_pool = parent_thread_pool;
	new_thread->id = thread_id;

	pthread_create(&new_thread->thread, NULL, (void*)thread_do, new_thread);
	pthread_detach(new_thread->thread);

	return new_thread;
}

struct thread_pool* thread_pool_create(int max_threads)
{
	struct thread_pool* new_thread_pool = NULL;
	int i = 0;

	if (max_threads < 0)
	{
		max_threads = 0;
	}

	errno = 0;

	/* Make new thread pool */
	new_thread_pool = (struct thread_pool*)malloc(sizeof(struct thread_pool));
	if (new_thread_pool == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		return NULL;
	}
	new_thread_pool->thread_alive_count = 0;
	new_thread_pool->thread_working_count = 0;

	/* Initialise the job queue */
	new_thread_pool->job_queue = job_queue_create();
	if (new_thread_pool->job_queue == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		thread_pool_free(new_thread_pool);
		return NULL;
	}

	/* Make threads in pool */
	new_thread_pool->threads = (struct thread**)malloc(max_threads * sizeof(struct thread*));
	if (new_thread_pool->threads == NULL)
	{
		errno = THREAD_ALLOC_FAILED;
		thread_pool_free(new_thread_pool); /* 함수 안에 job_queue_free() 포함 되어있음*/
		return NULL;
	}

	pthread_mutex_init(&new_thread_pool->thread_count_mutex, NULL);
	pthread_cond_init(&new_thread_pool->threads_all_idle, NULL);

	for (i = 0; i < max_threads; ++i)
	{
		thread_create(new_thread_pool, i);
	}

	return new_thread_pool;
}

int thread_pool_free(struct thread_pool* target_thread_pool)
{
	const float TIMEOUT = 1.0;
	float time_passed = 0.0;
	time_t start;
	time_t end;
	int thread_count = 0;
	int i = 0;

	if (target_thread_pool == NULL)
	{
		return THREAD_INVALID;
	}

	threads_keep_alive = 0;

	thread_count = target_thread_pool->thread_alive_count;

	time(&start);
	while (time_passed < TIMEOUT && target_thread_pool->thread_alive_count > 0)
	{
		binary_sem_post_broadcast(target_thread_pool->job_queue->has_jobs);

		time(&end);
		time_passed = difftime(end, start);
	}

	while (target_thread_pool->thread_alive_count > 0)
	{
		binary_sem_post_broadcast(target_thread_pool->job_queue->has_jobs);
		sleep(1);
	}

	job_queue_free(target_thread_pool->job_queue);
	target_thread_pool->job_queue = NULL;
	
	if (target_thread_pool->threads != NULL)
	{
		for (i = 0; i < thread_count; ++i)
		{
			if (target_thread_pool->threads[i] != NULL)
			{
				free(target_thread_pool->threads[i]);
				target_thread_pool->threads[i] = NULL;
			}
		}

		free(target_thread_pool->threads);
		target_thread_pool->threads = NULL;
	}

	free(target_thread_pool);

	return THREAD_SUCCESS;
}

int thread_pool_wait(struct thread_pool* target_thread_pool)
{
	if (target_thread_pool == NULL)
	{
		return THREAD_INVALID;
	}

	pthread_mutex_lock(&target_thread_pool->thread_count_mutex);
	while (target_thread_pool->job_queue->job_count > 0 || target_thread_pool->thread_working_count > 0)
	{
		pthread_cond_wait(&target_thread_pool->threads_all_idle, &target_thread_pool->thread_count_mutex);
	}
	pthread_mutex_unlock(&target_thread_pool->thread_count_mutex);
	
	return THREAD_SUCCESS;
}

int thread_pool_add_work(struct thread_pool* target_thread_pool, void (*function_ptr)(void*), void* arg_ptr)
{
	struct job* new_job = NULL;

	if (target_thread_pool == NULL)
	{
		return THREAD_INVALID;
	}

	new_job = (struct job*)malloc(sizeof(struct job));
	if (new_job == NULL)
	{
		return THREAD_ALLOC_FAILED;
	}

	new_job->function = function_ptr;
	new_job->arg = arg_ptr;
	job_queue_push(target_thread_pool->job_queue, new_job);

	return THREAD_SUCCESS;
}

