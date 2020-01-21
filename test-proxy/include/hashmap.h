#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#define DEFAULT_HASHMAP_SIZE 256

enum hashmap_key_type
{
	HASHMAP_KEY_TYPE_INT,
	HASHMAP_KEY_TYPE_DOUBLE,
	HASHMAP_KEY_TYPE_STRING
};

struct hashmap_element
{
	void* key;
	void* value;
};

struct hashmap
{
	int key_type;
	int table_size;
	int size;
	struct hashmap_element* data;

};

struct hashmap* hashmap_init();
int hashmap_insert(struct hashmap* map, void* key, void* value);
int hashmap_insert_int(struct hashmap* map, int key, int value);
void hashmap_free(struct hashmap* map);

int generate_hash(struct hashmap* map, void* key);

#endif
