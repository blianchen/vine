/*
 * intmap.h
 *
 *  Created on: Jan 27, 2015
 *      Author: blc
 */

#ifndef _INTMAP_H_
#define _INTMAP_H_

#include <stdint.h>

#define INTMAP_BUCKET_NUM 256
#define INTMAP_BUCKET_SIZE 8

#define INTMAP_E_NOTFOUND (-3)     /* No such element */
#define INTMAP_E_OVERFLOW (-2)     /* Hashmap is full */
#define INTMAP_E_FAIL     (-1)     /* Hashmap api fail */
#define INTMAP_S_OK       (0)      /* Success */


typedef struct intmap_s* intmap_t;
typedef struct intmap_iterator_s intmap_iterator;

intmap_t intmap_create(int initBucketNum);
int intmap_put(intmap_t map, int key, void *value);
void *intmap_get(intmap_t map, int key);
int intmap_remove(intmap_t map, int key);
void intmap_destroy(intmap_t map);

//int intmap_keys(intmap_t *map, int keys[]);
intmap_iterator *intmap_new_iterator(intmap_t map);
void *intmap_next(intmap_iterator *it, int *outkey);
void intmap_del_iterator(intmap_iterator *it);


typedef struct int64map_s* int64map_t;
typedef struct int64map_iterator_s int64map_iterator;

int64map_t int64map_create(int initBucketNum);
int int64map_put(int64map_t map, uint64_t key, void *value);
void *int64map_get(int64map_t map, uint64_t key);
int int64map_remove(int64map_t map, uint64_t key);
void int64map_destroy(int64map_t map);

//int int64map_keys(int64map_t *map, uint64_t keys[]);
int64map_iterator *int64map_new_iterator(int64map_t map);
void *int64map_next(int64map_iterator *it, uint64_t *outkey);
void int64map_del_iterator(int64map_iterator *it);


#endif /* _INTMAP_H_ */
