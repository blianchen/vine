/**
 * hashmap.h
 * 这个map里面没有复制任何数据，key和value都只是保存指针，
 * 因此put的原始key和value在外部改变或销毁都会对get产生影响。
 * hashmap_destroy中的回调函数可以用来销毁key和value的空间。
 * TODO hashmap_remove中并没有销毁数据
 */
#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#define HMAP_E_OUTMEM   (-4)     /* Out of Memory */
#define HMAP_E_NOTFOUND (-3)     /* No such element */
#define HMAP_E_OVERFLOW (-2)     /* Hashmap is full */
#define HMAP_E_FAIL     (-1)     /* Hashmap api fail */
#define HMAP_S_OK       (0)      /* Success */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * void_ptr is a pointer. This allows you to put arbitrary structures in the hashmap.
 */
typedef void* void_ptr;

/**
 * hmap_t is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented. They see and manipulate only hmap_t's.
 */
typedef struct hashmap_s* hashmap_t;

/**
 * hmap_callback_func is a pointer to a function that can take two void_ptr arguments
 * and return an integer. Returns status code..
 */
typedef int (*hashmap_callback_func)(void_ptr key, void_ptr value);


typedef struct hashmap_iterator_s hashmap_iterator;

/**
 * Return an empty hashmap. Returns NULL if empty.
 */
extern hashmap_t hashmap_create();

/**
 * Iteratively call fn with argument (value, arg) for each element data
 * in the hashmap. The function returns anything other than HMAP_S_OK
 * the traversal is terminated. fn must not modify any hashmap functions.
 */
//extern int hashmap_iterate(hmap_t in, hmap_callback_func fnIterValue, void_ptr arg);

/**
 * Add an element to the hashmap. Return HMAP_S_OK or HMAP_E_OUTMEM.
 */
extern int hashmap_put(hashmap_t in, char* key, void_ptr elem);

/**
 * Get an element from the hashmap. Return HMAP_S_OK or HMAP_E_NOTFOUND.
 */
extern int hashmap_get(hashmap_t in, char* key, void_ptr *elem);

/**
 * Remove an element from the hashmap. Return HMAP_S_OK or HMAP_E_NOTFOUND.
 */
extern int hashmap_remove(hashmap_t in, char* key);

/**
 * Free the hashmap
 */
extern void hashmap_destroy(hashmap_t in, hashmap_callback_func fnFree);

/**
 * Get the current size of a hashmap
 */
extern int hashmap_size(hashmap_t in);

extern hashmap_iterator *hashmap_new_iterator(hashmap_t map);
extern void *hashmap_next(hashmap_iterator *it, void **outkey);
extern void hashmap_del_iterator(hashmap_iterator *it);

#ifdef __cplusplus
}
#endif

#endif /* _HASHMAP_H_INCLUDED */
