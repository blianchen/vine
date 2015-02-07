/*
 * intmap.c
 *
 *  Created on: Jan 27, 2015
 *      Author: blc
 */

#include <stdlib.h>

#include <mem.h>

#include <intmap.h>


typedef struct intmap_elem_s {
	int used;
	int key;
	void *value;
} intmap_elem_t;

struct intmap_s {
	int bucket_num;
	int size;
	intmap_elem_t *elem;
};

#define BUCKETINDEX(map, key) (((unsigned int)key % map->bucket_num) * INTMAP_BUCKET_SIZE)

void _intmap_expand(struct intmap_s *map);
int _intmap_index(struct intmap_s *map, int key);


intmap_t *intmap_create(int initBucketNum) {
	if (initBucketNum <= 0) {
		initBucketNum = INTMAP_BUCKET_NUM;
	}
	intmap_t *map = MALLOC(sizeof(intmap_t));
	map->elem = CALLOC(initBucketNum*INTMAP_BUCKET_SIZE, sizeof(intmap_elem_t));
	map->bucket_num = initBucketNum;
	map->size = 0;
	return map;
}

int intmap_put(intmap_t *map, int key, void *value) {
	int index = _intmap_index(map, key);
	while (index == INTMAP_E_OVERFLOW) {
		_intmap_expand(map);
		index = _intmap_index(map, key);
	}
	intmap_elem_t *e = map->elem + index;
	if (!e->used) {
		map->size++;
	}
	e->used = 1;
	e->key = key;
	e->value = value;

	return INTMAP_S_OK;
}

void *intmap_get(intmap_t *map, int key) {
	int bucketIndex = BUCKETINDEX(map, key);
	intmap_elem_t *elem;
	int i;
	for (i = 0; i < INTMAP_BUCKET_SIZE; i++) {
		elem = map->elem + bucketIndex + i;
		if (elem->used && elem->key == key) {
			return elem->value;
		}
	}
	return NULL;
}

int intmap_remove(intmap_t *map, int key) {
	int index = _intmap_index(map, key);
	intmap_elem_t *e = map->elem + index;
	if (e->used && e->key == key) {
		map->size--;
		e->used = 0;
		e->key = 0;
		e->value = NULL;
		return index;
	}
	return INTMAP_E_NOTFOUND;
}

int intmap_size(intmap_t *map) {
	return map->size;
}

int _intmap_index(struct intmap_s *map, int key) {
	int bucketIndex = BUCKETINDEX(map, key);
	intmap_elem_t *e;
	int i;
	for (i=0; i<INTMAP_BUCKET_SIZE; i++) {
		e = map->elem + bucketIndex + i;
		if (!e->used) {
			return bucketIndex + i;
		}
		if (e->used && e->key == key) {
			return bucketIndex + i;
		}
	}
	return INTMAP_E_OVERFLOW;
}

void _intmap_expand(struct intmap_s *map) {
	int oldSize = map->bucket_num * INTMAP_BUCKET_SIZE;
	intmap_elem_t *oldElem = map->elem;

	map->bucket_num = map->bucket_num << 1;
	map->elem = CALLOC(map->bucket_num*INTMAP_BUCKET_SIZE, sizeof(intmap_elem_t));
	map->size = 0;

	intmap_elem_t *tmp;
	int i;
	for (i=0; i<oldSize; i++) {
		tmp = oldElem + i;
		if (tmp->used) {
			intmap_put(map, tmp->key, tmp->value);
		}
	}
	FREE(oldElem);
}
