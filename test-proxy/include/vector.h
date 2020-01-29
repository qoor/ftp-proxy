#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <sys/types.h>

#define DEFAULT_VECTOR_CAPACITY 20
#define MIN_CAPACITY_CALC(capacity) ((capacity) * 4 / 3)

/* VECTOR RETURN CODE DEFINE */
enum vector_error_type
{
	VECTOR_SUCCESS,
	VECTOR_INVALID,
	VECTOR_ALLOC_FAILED,
	VECTOR_OUT_OF_BOUNDS
};
/* */

struct vector
{
	void** container;
	size_t size;
	size_t capacity;
};

struct vector* vector_init(size_t capacity);
int vector_insert(struct vector* vector, int index, void* object);
int vector_erase(struct vector* vector, int index);
int vector_push_back(struct vector* vector, void* object);
int vector_pop_back(struct vector* vector);
void* vector_get(struct vector* vector, int index);
int vector_set(struct vector* vector, int index, void* object);
int vector_clear(struct vector* vector);
int vector_free(struct vector* vector);

#endif

