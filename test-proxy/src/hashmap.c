#include "StdInc.h"

#include "hashmap.h"

struct hashmap* hashmap_init(int key_type)
{
	struct hashmap* object = (struct hashmap*)malloc(sizeof(struct hashmap));

	if (object == NULL)
	{
		return NULL;
	}

	object->data = (struct hashmap_element*)malloc(DEFAULT_HASHMAP_SIZE * sizeof(struct hashmap_element));

	if (object->data == NULL)
	{
		free(object);
		return NULL;
	}

	object->key_type = key_type;
	object->table_size = DEFAULT_HASHMAP_SIZE;
	object->size = 0;

	return object;
}

int hashmap_insert(struct hashmap* map, void* key, void* value)
{
	int index = 0;
	struct hashmap* current_map = map;

	if (map == NULL)
	{
		return HASHMAP_INVALID;
	}

	if (key == NULL)
	{
		return HASHMAP_INVALID_KEY;
	}

	index = generate_hash(map, key);
}

void hashmap_free(struct hashmap* map)
{
	if (map == NULL)
	{
		return;
	}

	if (map->data != NULL)
	{
		free(map->data);
		map->data = NULL;
	}

	free(map);
}

int generate_hash(struct hashmap* map, void* key)
{
	return 0;
}
