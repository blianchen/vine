/*
 * intmap.h
 *
 *  Created on: Jan 27, 2015
 *      Author: blc
 */

#ifndef _INTMAP_H_
#define _INTMAP_H_

#define INTMAP_BUCKET_NUM 256
#define INTMAP_BUCKET_SIZE 8

#define INTMAP_E_NOTFOUND (-3)     /* No such element */
#define INTMAP_E_OVERFLOW (-2)     /* Hashmap is full */
#define INTMAP_E_FAIL     (-1)     /* Hashmap api fail */
#define INTMAP_S_OK       (0)      /* Success */


typedef struct intmap_s intmap_t;


intmap_t *intmap_create(int initBucketNum);
int intmap_put(intmap_t *map, int key, void *value);
void *intmap_get(intmap_t *map, int key);
int intmap_remove(intmap_t *map, int key);


#endif /* _INTMAP_H_ */
