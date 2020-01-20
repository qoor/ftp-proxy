#ifndef __VECTOR_H__
#define __VECTOR_H__

#define DEFAULT_VECTOR_CAPACITY 20

struct vector
{
	void** container;
	size_t size;
	size_t capacity;
};

int vector_init(struct vector* vector, size_t capacity);
int vector_insert(struct vector* vector, int index, void* object);
int vector_erase(struct vector* vector, int index);
int vector_push_back(struct vector* vector, void* object);
int vector_pop_back(struct vector* vector);
void* vector_get(struct vector* vector, int index);
int vector_set(struct vector* vector, int index, void* object);
int vector_set_capacity(struct vector* vector, size_t capacity);

#endif
