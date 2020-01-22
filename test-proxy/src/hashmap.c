#include "StdInc.h"

#include "hashmap.h"

static int get_hash_key(struct hashmap* map, void* key)
{
	int hash = map->hash_function(key);

	hash += ~(hash << 9);
	hash ^= (((unsigned int)hash) >> 14);
	hash += (hash << 4);
	hash ^= (((unsigned int)hash) >> 10);

	return hash;
}

static int get_index_from_hash_key(size_t capacity, int hash_key)
{
	return (((size_t)hash_key) & (capacity - 1));
}

static struct hashmap_element* create_element_container(void* key, int hash, void* value)
{
	struct hashmap_element* element = (struct hashmap_element*)malloc(sizeof(struct hashmap_element));

	if (element == NULL)
	{
		return NULL;
	}

	element->key = key;
	element->hash = hash;
	element->value = value;
	element->next = NULL;

	return element;
}

static int equal_keys(void* key1, int hash_key1, void* key2, int hash_key2, int (*equals)(void*, void*))
{
	if (key1 == key2)
	{
		return 1;
	}

	if (hash_key1 != hash_key2)
	{
		return 0;
	}

	if (equals == NULL)
	{
		return 1;
	}

	return equals(key1, key2);
}

static void expand_if_necessary(struct hashmap* map)
{
	size_t i = 0;
	size_t index = 0;
	size_t new_capacity = 1;
	struct hashmap_element** new_elements = NULL;
	struct hashmap_element* current_element = NULL;
	struct hashmap_element* next_element = NULL;

	if (map->size <= MIN_CAPACITY_CALC(map->capacity))
	{
		return;
	}

	new_capacity = map->capacity << 1;
	if ((new_elements = (struct hashmap_element**)calloc(new_capacity, sizeof(struct hashmap_element*))) == NULL)
	{
		return;
	}

	for (i = 0; i < map->capacity; ++i)
	{
		current_element = map->element_containers[i];

		while (current_element != NULL)
		{
			next_element = current_element->next;
			index = get_index_from_hash_key(new_capacity, current_element->hash);

			current_element->next = new_elements[index];
			new_elements[index] = current_element;
			current_element = next_element;
		}
	}

	free(map->element_containers);
	map->element_containers = new_elements;
	map->capacity = new_capacity;
}

struct hashmap* hashmap_init(size_t capacity, int hash_function(void*), int (*equals)(void*, void*))
{
	struct hashmap* object = (struct hashmap*)malloc(sizeof(struct hashmap));
	size_t min_capacity = MIN_CAPACITY_CALC(capacity);

	if (object == NULL)
	{
		return NULL;
	}

	object->capacity = 1;

	while (object->capacity <= min_capacity)
	{
		object->capacity <<= 1;
	}

	object->element_containers = (struct hashmap_element**)calloc(object->capacity, sizeof(struct hashmap_element*));

	if (object->element_containers == NULL)
	{
		free(object);
		return NULL;
	}

	object->size = 0;
	object->hash_function = hash_function;
	object->equals = equals;

	return object;
}

void* hashmap_insert(struct hashmap* map, void* key, void* value)
{
	int hash_key = 0;
	struct hashmap_element** map_pointer = NULL;
	struct hashmap_element* current_map = NULL;
	void* old_value = NULL;
	size_t index = 0;

	if (map == NULL)
	{
		return NULL;
	}

	hash_key = get_hash_key(map, key);
	index = get_index_from_hash_key(map->capacity, hash_key);

	map_pointer = &map->element_containers[index];

	while (1)
	{
		current_map = *map_pointer;

		if (current_map == NULL)
		{
			if ((current_map = create_element_container(key, hash_key, value)) == NULL)
			{
				return NULL;
			}

			map->size++;
			expand_if_necessary(map);

			return NULL;
		}

		if (equal_keys(current_map->key, current_map->hash, key, hash_key, map->equals) == 1)
		{
			old_value = current_map->value;
			current_map->value = value;
			return old_value;
		}

		map_pointer = &current_map->next;
	}
}

void hashmap_foreach(struct hashmap* map, int (*callback)(void* key, void* value, void* context), void* context)
{
	size_t i;
	struct hashmap_element* current_map = NULL;
	struct hashmap_element* next_map = NULL;

	if (map == NULL || callback == NULL)
	{
		return;
	}

	for (i = 0; i < map->capacity; ++i)
	{
		current_map = map->element_containers[i];

		while (current_map != NULL)
		{
			next_map = current_map->next;

			if (callback(current_map->key, current_map->value, context) == 0)
			{
				return;
			}

			current_map = next_map;
		}
	}
}

void* hashmap_erase(struct hashmap* map, void* key)
{
	int hash_key = 0;
	size_t index = 0;
	struct hashmap_element** map_pointer = NULL;
	struct hashmap_element* current_map = NULL;
	void* value = NULL;

	if (map == NULL)
	{
		return NULL;
	}

	hash_key = get_hash_key(map, key);
	index = get_index_from_hash_key(map->capacity, hash_key);

	map_pointer = &map->element_containers[index];
	while ((current_map = *map_pointer) != NULL)
	{
		if (equal_keys(current_map->key, current_map->hash, key, hash_key, map->equals))
		{
			value = current_map->value;
			*map_pointer = current_map->next;
			free(current_map);
			--map->size;
			return value;
		}

		map_pointer = &current_map->next;
	}

	return NULL;
}

void hashmap_free(struct hashmap* map)
{
	size_t i;
	struct hashmap_element* current_map = NULL;
	struct hashmap_element* next_map = NULL;

	if (map == NULL)
	{
		return;
	}

	for (i = 0; i < map->capacity; ++i)
	{
		current_map = map->element_containers[i];
		while (current_map != NULL)
		{
			next_map = current_map->next;
			free(current_map);
			current_map = next_map;
		}
	}

	free(map->element_containers);
	free(map);
}

int hashmap_hash(void* key, size_t key_size)
{
	int hash_key = key_size;
	char* data = (char*)key;
	size_t i = 0;

	for (i = 0; i < key_size; ++i)
	{
		hash_key = (hash_key * 31) + *data;
		++data;
	}

	return hash_key;
}

int hashmap_hash_int(void* key)
{
	return (*((int*)key));
}

int hashmap_equals_int(void* key1, void* key2)
{
	int a = *((int*)key1);
	int b = *((int*)key2);

	return (a == b);
}
