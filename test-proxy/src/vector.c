#include <stdio.h>
#include <errno.h>

#include <sys/types.h>

#include "vector.h"
#include "proxy.h"

static void expand_if_necessary(struct vector* vector)
{
	size_t new_capacity = 0;
	void** new_container = NULL;

	if (vector->size <= MIN_CAPACITY_CALC(vector->capacity))
	{
		return;
	}

	new_capacity = vector->capacity << 1;
	if ((new_container = (void**)calloc(new_capacity, sizeof(void*))) == NULL)
	{
		return;
	}

	memcpy((void*)new_container, (void*)vector->container, vector->size);
	vector->container = new_container;
}

struct vector* vector_init(size_t capacity)
{
	struct vector* vector = (struct vector*)malloc(sizeof(struct vector));
	size_t min_capacity = 0;

	if (vector == NULL)
	{
		errno = VECTOR_ALLOC_FAILED;

		return NULL;
	}

	if (capacity == 0)
	{
		capacity = DEFAULT_VECTOR_CAPACITY;
	}

	vector->capacity = 1;
	min_capacity = MIN_CAPACITY_CALC(capacity);

	while (vector->capacity <= min_capacity)
	{
		vector->capacity <<= 1;
	}

	if ((vector->container = (void**)calloc(capacity, sizeof(void*))) == NULL)
	{
		errno = VECTOR_ALLOC_FAILED;

		free(vector);
		return NULL;
	}

	vector->capacity = capacity;
	vector->size = 0;

	return vector;
}

int vector_insert(struct vector* vector, int index, void* object)
{
	int i = 0;

	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (index < 0 || index >= vector->size)
	{
		return VECTOR_OUT_OF_BOUNDS;
	}

	for (i = vector->size - 1; i > index; ++i)
	{
		vector->container[i] = vector->container[i - 1];
	}
	vector->container[index] = object;
	++vector->size;
	expand_if_necessary(vector);

	return VECTOR_SUCCESS;
}

int vector_erase(struct vector* vector, int index)
{
	int i = 0;

	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (index < 0 || index >= vector->size)
	{
		return VECTOR_OUT_OF_BOUNDS;
	}

	for (i = index; i < vector->size; ++i)
	{
		vector->container[i] = vector->container[i + 1];
	}

	vector->container[--vector->size] = NULL;

	return VECTOR_SUCCESS;
}

int vector_push_back(struct vector* vector, void* object)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	vector->container[vector->size] = object;
	++vector->size;
	expand_if_necessary(vector);

	return VECTOR_SUCCESS;
}

/* This function will NOT free object automatically */
int vector_pop_back(struct vector* vector)
{
	int last_index = 0;

	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	last_index = vector->size - 1;

	if (last_index >= 0)
	{
		vector->container[last_index] = NULL;
		--vector->size;
	}

	return VECTOR_SUCCESS;
}

void* vector_get(struct vector* vector, int index)
{
	if (vector == NULL)
	{
		return NULL;
	}

	if (index < 0 || index >= vector->size)
	{
		return NULL;
	}

	return vector->container[index];
}

int vector_set(struct vector* vector, int index, void* object)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (index < 0 || index >= vector->size)
	{
		return VECTOR_OUT_OF_BOUNDS;
	}

	vector->container[index] = object;

	return VECTOR_SUCCESS;
}

int vector_clear(struct vector* vector)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	memset((void*)vector->container, 0x00, vector->capacity * sizeof(void*));
	vector->size = 0;

	return VECTOR_SUCCESS;
}

/*
 * Memory free of vector
 * This function will NOT free object automatically
*/
int vector_free(struct vector* vector)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (vector->container != NULL)
	{
		free(vector->container);
		vector->container = NULL;
	}

	free(vector);

	return VECTOR_SUCCESS;
}
