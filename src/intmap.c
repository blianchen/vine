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
} intmap_s;

struct intmap_iterator_s {
	int idx;
	intmap_t map;
};

#define BUCKETINDEX(map, key) (((unsigned int)key % map->bucket_num) * INTMAP_BUCKET_SIZE)

void _intmap_expand(struct intmap_s *map);
int _intmap_index(struct intmap_s *map, int key);


intmap_t intmap_create(int initBucketNum) {
	if (initBucketNum <= 0) {
		initBucketNum = INTMAP_BUCKET_NUM;
	}
	intmap_t map = MALLOC(sizeof(intmap_s));
	map->elem = CALLOC(initBucketNum*INTMAP_BUCKET_SIZE, sizeof(intmap_elem_t));
	map->bucket_num = initBucketNum;
	map->size = 0;
	return map;
}

int intmap_put(intmap_t map, int key, void *value) {
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

void *intmap_get(intmap_t map, int key) {
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

int intmap_remove(intmap_t map, int key) {
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

int intmap_size(intmap_t map) {
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

void intmap_destroy(intmap_t map) {
	FREE(map->elem);
	FREE(map);
}

//int intmap_keys(intmap_t *map, int keys[]) {
//	int i, j=0;
//	int size = map->bucket_num*INTMAP_BUCKET_SIZE;
//	intmap_elem_t *tmp;
//	for (i=0; i<size; i++) {
//		tmp = map->elem + i;
//		if (tmp->used) {
//			keys[j] = tmp->key;
//			j++;
//		}
//	}
//	return j;
//}
intmap_iterator *intmap_new_iterator(intmap_t map) {
	intmap_iterator *it = MALLOC(sizeof(intmap_iterator));
	it->idx = 0;
	it->map = map;
	return it;
}
void *intmap_next(intmap_iterator *it, int *outkey) {
	intmap_t map = it->map;
	intmap_elem_t *tmp;
	int i;
	int size = map->bucket_num*INTMAP_BUCKET_SIZE;
	for (i=it->idx; i<size; i++) {
		tmp = map->elem + i;
		if (tmp->used) {
			it->idx = i + 1;
			*outkey = tmp->key;
			return tmp->value;
		}
	}
	return NULL;
}
void intmap_del_iterator(intmap_iterator *it) {
	FREE(it);
}


//////////////////////////////////////////////////////////////////////////////
///////////// int64 //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
typedef struct int64map_elem_s {
	int used;
	uint64_t key;
	void *value;
} int64map_elem_t;

struct int64map_s {
	int bucket_num;
	int size;
	int64map_elem_t *elem;
};

struct int64map_iterator_s {
	int idx;
	int64map_t map;
};

#define BUCKETINDEX64(map, key) (((uint64_t)key % map->bucket_num) * INTMAP_BUCKET_SIZE)

void _int64map_expand(struct int64map_s *map);
int _int64map_index(struct int64map_s *map, uint64_t key);


int64map_t int64map_create(int initBucketNum) {
	if (initBucketNum <= 0) {
		initBucketNum = INTMAP_BUCKET_NUM;
	}
	int64map_t map = MALLOC(sizeof(struct int64map_s));
	map->elem = CALLOC(initBucketNum*INTMAP_BUCKET_SIZE, sizeof(int64map_elem_t));
	map->bucket_num = initBucketNum;
	map->size = 0;
	return map;
}

int int64map_put(int64map_t map, uint64_t key, void *value) {
	int index = _int64map_index(map, key);
	while (index == INTMAP_E_OVERFLOW) {
		_int64map_expand(map);
		index = _int64map_index(map, key);
	}
	int64map_elem_t *e = map->elem + index;
	if (!e->used) {
		map->size++;
	}
	e->used = 1;
	e->key = key;
	e->value = value;

	return INTMAP_S_OK;
}

void *int64map_get(int64map_t map, uint64_t key) {
	int bucketIndex = BUCKETINDEX64(map, key);
	int64map_elem_t *elem;
	int i;
	for (i = 0; i < INTMAP_BUCKET_SIZE; i++) {
		elem = map->elem + bucketIndex + i;
		if (elem->used && elem->key == key) {
			return elem->value;
		}
	}
	return NULL;
}

int int64map_remove(int64map_t map, uint64_t key) {
	int index = _int64map_index(map, key);
	int64map_elem_t *e = map->elem + index;
	if (e->used && e->key == key) {
		map->size--;
		e->used = 0;
		e->key = 0;
		e->value = NULL;
		return index;
	}
	return INTMAP_E_NOTFOUND;
}

int int64map_size(int64map_t map) {
	return map->size;
}

int _int64map_index(struct int64map_s *map, uint64_t key) {
	int bucketIndex = BUCKETINDEX64(map, key);
	int64map_elem_t *e;
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

void _int64map_expand(struct int64map_s *map) {
	int oldSize = map->bucket_num * INTMAP_BUCKET_SIZE;
	int64map_elem_t *oldElem = map->elem;

	map->bucket_num = map->bucket_num << 1;
	map->elem = CALLOC(map->bucket_num*INTMAP_BUCKET_SIZE, sizeof(int64map_elem_t));
	map->size = 0;

	int64map_elem_t *tmp;
	int i;
	for (i=0; i<oldSize; i++) {
		tmp = oldElem + i;
		if (tmp->used) {
			int64map_put(map, tmp->key, tmp->value);
		}
	}
	FREE(oldElem);
}

void int64map_destroy(int64map_t map) {
	FREE(map->elem);
	FREE(map);
}

//int int64map_keys(int64map_t *map, uint64_t keys[]) {
//	int i, j=0;
//	int size = map->bucket_num*INTMAP_BUCKET_SIZE;
//	int64map_elem_t *tmp;
//	for (i=0; i<size; i++) {
//		tmp = map->elem + i;
//		if (tmp->used) {
//			keys[j] = tmp->key;
//			j++;
//		}
//	}
//	return j;
//
//}
int64map_iterator *int64map_new_iterator(int64map_t map) {
	int64map_iterator *it = MALLOC(sizeof(int64map_iterator));
	it->idx = 0;
	it->map = map;
	return it;
}
void *int64map_next(int64map_iterator *it, uint64_t *outkey) {
	int64map_t map = it->map;
	int64map_elem_t *tmp;
	int i;
	int size = map->bucket_num*INTMAP_BUCKET_SIZE;
	for (i=it->idx; i<size; i++) {
		tmp = map->elem + i;
		if (tmp->used) {
			it->idx = i + 1;
			*outkey = tmp->key;
			return tmp->value;
		}
	}
	return NULL;
}
void int64map_del_iterator(int64map_iterator *it) {
	FREE(it);
}
