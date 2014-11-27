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


#ifndef VECTOR_INCLUDED
#define VECTOR_INCLUDED


/**
 * A <b>vector</b> represent a resizable, integer indexed array of
 * any object. Indexing starts at 0 and it is a checked runtime error to 
 * access index out of the range.
 *
 * A vector can also be used as a stack (LIFO) by only using the methods
 * vector_push() and vector_pop() which respectively act as push and pop
 * operations.
 *
 * @file
 */


#define T vector_t
typedef struct T *T;


/**
 * Create a new vector.
 * @param hint The initial capacity of the vector (hint >= 0)
 * @return A vector object
 */
T vector_new(int hint);
        

/**
 * Destroy a vector object.
 * @param V A vector object reference
 */
void vector_free(T *V);


/**
 * Insert the element at the specified location in the vector. Shifts
 * the element currently at that position (if any) and any subsequent 
 * elements to the right (adds one to their indices). The index, 
 * <code>i</code>, must be in the range, (i >= 0 && i <= vector_size()).
 * @param V A vector object
 * @param i Index of object to insert
 * @param e Element to be inserted
 */
void vector_insert(T V, int i, void *e);


/**
 * Replace the element at the specified location in the vector. The previous
 * element at this location is returned. The index, <code>i</code>, must be 
 * in the range, (i >= 0 && i < vector_size()).
 * @param V A vector object
 * @param i Index of object to replace
 * @param e Element to be inserted
 * @return The previous object at this location
 */
void *vector_set(T V, int i, void *e);

        
/**
 * Returns the element at the specified position. The index, <code>i</code>,
 * must be in the range, (i >= 0 && i < vector_size()).
 * @param V A vector object
 * @param i Index of object to return
 * @return The object at the specified index
 */
void *vector_get(T V, int i);


/**
 * Remove the element at the specified position in the vector. The object
 * removed is returned. Shifts any subsequent elements to the left 
 * (subtracts one from their indices). The index, <code>i</code>, must be 
 * in the range, (i >= 0 && i < vector_size()).
 * @param V A vector object
 * @param i Index of element to remove
 * @return The object that was removed
 */
void *vector_remove(T V, int i);


/**
 * Append the element to the end of this vector increasing it's size with 1
 * @param V A vector object
 * @param e Element to be appended
 */
void vector_push(T V, void *e);


/**
 * Remove the last element from the vector. The object removed is returned.
 * It is a checked runtime error to call this method if the vector already
 * is empty.
 * @param V A vector object
 * @return The object that was removed
 */
void *vector_pop(T V);


/**
 * Test if this vector is empty.
 * @param V A vector object
 * @return true if this vector has no elements, i.e. it's size is 0
 * otherwise false.
 */
int vector_isEmpty(T V);


/**
 * Returns the number of elements in this vector
 * @param V A vector object
 * @return Number of elements in this vector
 */
int vector_size(T V);
 

/**
 * Apply the visitor function, <code>apply(const void *element, void *ap),
 * </code> for each element in the vector. Clients can pass an application
 * specific pointer, <code>ap</code>, to vector_map() and this pointer is
 * passed along to the <code>apply</code> function at each call. It is a 
 * checked runtime error for <code>apply</code> to change the vector.
 * @param V A vector object
 * @param apply The function to apply
 * @param ap An application-specific pointer. If such a pointer is 
 * not needed, just use NULL
 */
void vector_map(T V, void apply(const void *element, void *ap), void *ap);


/**
 * Creates a N + 1 length array containing all the elements 
 * in this vector. The last element in the array is <code>NULL</code>.
 * The caller is responsible for deallocating the array.
 * @param V A vector object
 * @return A pointer to the first element in the array
 */
void **vector_toArray(T V);
        
#undef T
#endif
