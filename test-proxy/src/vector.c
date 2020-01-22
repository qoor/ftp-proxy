#include "StdInc.h"

int vector_init(struct vector* vector, size_t capacity)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (capacity == 0)
	{
		capacity = DEFAULT_VECTOR_CAPACITY;
	}

	if ((vector->container = (void**)malloc(capacity * sizeof(void*))) == NULL)
	{
		return VECTOR_ALLOC_FAILED;
	}

	memset(vector->container, 0x00, capacity * sizeof(void*));

	vector->capacity = capacity;
	vector->size = 0;

	return VECTOR_SUCCESS;
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

	if (vector->size + 1 > vector->capacity && vector_set_capacity(vector, vector->capacity * 2) != VECTOR_SUCCESS)
	{
		return VECTOR_ALLOC_FAILED;
	}

	for (i = vector->size - 1; i > index; ++i)
	{
		vector->container[i] = vector->container[i - 1];
	}

	vector->container[index] = object;

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

	if (vector->size + 1 > vector->capacity && vector_set_capacity(vector, vector->capacity * 2) != VECTOR_SUCCESS)
	{
		return VECTOR_ALLOC_FAILED;
	}

	if (index > 0)
	{
		for (i = vector->size - 1; i >= index; ++i)
		{
			vector->container[i] = vector->container[i - 1];
		}
	}

	return VECTOR_SUCCESS;
}

int vector_push_back(struct vector* vector, void* object)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (vector->size + 1 > vector->capacity && vector_set_capacity(vector, vector->capacity * 2) != VECTOR_SUCCESS)
	{
		return VECTOR_ALLOC_FAILED;
	}

	vector->container[vector->size] = object;
	++vector->size;

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

int vector_set_capacity(struct vector* vector, size_t capacity)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	if (capacity <= 0)
	{
		capacity = DEFAULT_VECTOR_CAPACITY;
	}

	if (vector->capacity != capacity)
	{
		vector->container = (void**)realloc(vector->container, capacity * sizeof(void*));
		if (capacity > vector->capacity)
		{
			memset(vector->container + vector->capacity, 0x00, (capacity - vector->capacity) * sizeof(void*));
		}

		vector->capacity = capacity;
	}

	return VECTOR_SUCCESS;
}

int vector_clear(struct vector* vector)
{
	if (vector == NULL)
	{
		return VECTOR_INVALID;
	}

	memset(vector->container, 0x00, vector->capacity * sizeof(void*));
	vector->size = 0;

	return VECTOR_SUCCESS;
}
