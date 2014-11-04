/*
 * hashmap_test.c
 *
 *  Created on: 2014年9月26日
 *      Author: blc
 */

#include "core_test.h"

#include <stdlib.h>
#include <stdio.h>
#include <hashmap.h>

START_TEST(test_hashmap_put) {
	hmap_t map = hashmap_create();
	const char* va = "v123";
	hashmap_put(map, "k1", (void_ptr)va);
	char* v;
	hashmap_get(map, "k1", (void_ptr)&v);
	ck_assert_str_eq(v, va);
	hashmap_destroy(map, NULL, NULL);
}
END_TEST

START_TEST(test_hashmap_get) {
	hmap_t map = hashmap_create();
	const char* va = "v123";
	int vb = 456;
	hashmap_put(map, "k1", (void_ptr)va);
	hashmap_put(map, "k2", (void_ptr)&vb);
	char* v;
	hashmap_get(map, "k1", (void_ptr)&v);
	ck_assert_str_eq(v, va);
	int *vi;
	hashmap_get(map, "k2", (void_ptr)&vi);
	ck_assert_int_eq(*vi, vb);

	hashmap_destroy(map, NULL, NULL);
}
END_TEST

START_TEST(test_hashmap_size) {
	hmap_t map = hashmap_create();
	const char* va = "v123";
	int vb = 456;
	hashmap_put(map, "k1", (void_ptr)va);
	hashmap_put(map, "k2", (void_ptr)&vb);
	int i;
	for (i=0; i<50; i++) {
		char* key = malloc(100);
		sprintf(key, "test1%d", i);
		int v = i;
		hashmap_put(map, key, &v);
	}
	ck_assert_int_eq(hashmap_size(map), 52);

	hashmap_destroy(map, NULL, NULL);
}
END_TEST

int mapECnt = 0;
int iterValuefun(void_ptr a, void_ptr b) {
	mapECnt += *(int*)a;
	return 0;
}
START_TEST(test_hashmap_iterate) {
	hmap_t map = hashmap_create();
	int vs[50];
	int i;
	for (i=0; i<50; i++) {
		char* key = malloc(100);
		sprintf(key, "test1%d", i);
		vs[i] = 2;
		hashmap_put(map, key, vs+i);
	}

	hashmap_iterate(map, iterValuefun, NULL);

	ck_assert_int_eq(mapECnt, 100);

	hashmap_destroy(map, NULL, NULL);
}
END_TEST

START_TEST(test_hashmap_remove) {
	hmap_t map = hashmap_create();
	int vs[50];
	int i;
	for (i=0; i<50; i++) {
		char* key = malloc(100);
		sprintf(key, "test%d", i);
		vs[i] = i;
		hashmap_put(map, key, vs+i);
	}

	int* v;
	hashmap_get(map, "test5", (void_ptr)&v);
	ck_assert_int_eq(*v, 5);
	int* vr;
	hashmap_remove(map, "test5", (void_ptr)&vr);
	ck_assert_int_eq(*vr, 5);
	int* vv;
	int r = hashmap_get(map, "test5", (void_ptr)&vv);
	ck_assert_int_eq(r, HMAP_E_NOTFOUND);

	hashmap_destroy(map, NULL, NULL);
}
END_TEST

TCase* make_hashmap_tcase(void) {
	TCase* tc = tcase_create("hashmap");
	tcase_add_test(tc, test_hashmap_put);
	tcase_add_test(tc, test_hashmap_get);
	tcase_add_test(tc, test_hashmap_size);
	tcase_add_test(tc, test_hashmap_iterate);
	tcase_add_test(tc, test_hashmap_remove);
	return tc;
}
