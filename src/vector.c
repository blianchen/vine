/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.
 */


#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <mem.h>

#include <vector.h>


/**
 * Implementation of the vector interface.
 *
 * @file
 */


/* ----------------------------------------------------------- Definitions */


#define T vector_t
struct T {
        int length;
        int capacity;
        void **array;
	uint32_t timestamp;
};


/* ------------------------------------------------------- Private methods */

static inline void ensureCapacity(T V) {
	if (V->length >= V->capacity) {
		V->capacity = 2 * V->length;
		REALLOC(V->array, V->capacity * sizeof(void *));
	}
}


/* ----------------------------------------------------- Protected methods */


T vector_new(int hint) {
	T V;
	assert(hint >= 0);
	NEW(V);
	if (hint == 0)
		hint = 16;
	V->capacity = hint;
	V->array = CALLOC(V->capacity, sizeof(void *));
	return V;
}


void vector_free(T *V) {
	assert(V && *V);
	FREE((*V)->array);
	FREE(*V);
}


void vector_insert(T V, int i, void *e) {
	assert(V);
	assert(i >= 0 && i <= V->length);
	V->timestamp++;
	ensureCapacity(V);
	int j;
	for (j = V->length++; j > i; j--)
		V->array[j] = V->array[j - 1];
	V->array[i] = e;
}


void *vector_set(T V, int i, void *e) {
	assert(V);
	assert(i >= 0 && i < V->length);
	V->timestamp++;
	void *prev = V->array[i];
        V->array[i] = e;
	return prev;
}


void *vector_get(T V, int i) {
	assert(V);
	assert(i >= 0 && i < V->length);
        return V->array[i];
}


void *vector_remove(T V, int i) {
	assert(V);
	assert(i >= 0 && i < V->length);
	V->timestamp++;
	void *x = V->array[i];
	V->length--;
	int j;
	for (j = i; j < V->length; j++)
		V->array[j] = V->array[j + 1];
	return x;
}


void vector_push(T V, void *e) {
        assert(V);
	V->timestamp++;
        ensureCapacity(V);
        V->array[V->length++] = e;
}


void *vector_pop(T V) {
        assert(V);
 	assert(V->length>0);
	V->timestamp++;
        return V->array[--V->length];
}


int vector_isEmpty(T V) {
        assert(V);
        return (V->length == 0);
}


int vector_size(T V) {
        assert(V);
        return V->length;
}


void vector_map(T V, void apply(const void *element, void *ap), void *ap) {
	assert(V);
	assert(apply);
	uint32_t stamp = V->timestamp;
	int i;
	for (i = 0; i < V->length; i++) {
		apply(V->array[i], ap);
		assert(V->timestamp == stamp);
	}
}


void **vector_toArray(T V) {
	int i;
	assert(V);
	void **array = MALLOC((V->length + 1) * sizeof(*array));
	for (i = 0; i < V->length; i++)
		array[i] = V->array[i];
	array[i] = NULL;
	return array;
}

