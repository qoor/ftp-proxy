#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#define DEFAULT_HASHMAP_SIZE 256

struct hashmap_element
{
	void* key;
	int hash;
	void* value;
	struct hashmap_element* next;
};

struct hashmap
{
	size_t capacity;
	size_t size;
	struct hashmap_element** element_containers;
	int (*hash_function)(void* key);
	int (*equals)(void* key1, void* key2);
};

struct hashmap* hashmap_init(size_t capacity, int hash_function(void*), int (*equals)(void*, void*));
void* hashmap_insert(struct hashmap* map, void* key, void* value);
void hashmap_foreach(struct hashmap* map, int (*callback)(void* key, void* value, void* context), void* context);
void* hashmap_erase(struct hashmap* map, void* key);
void hashmap_free(struct hashmap* map);
int hashmap_hash(void* key, size_t key_size);
int hashmap_hash_int(void* key);
int hashmap_equals_int(void* key1, void* key2);

#endif
