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


#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <MemoryException.h>
#include <utils.h>

/**
 * Implementation of the Mem interface
 *
 * @file
 */


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

void *mem_malloc(int size, const char *func, const char *file, int line){
	assert(size > 0);
	void *p = malloc(size);
	if (! p)
		exception_throw(&(MemoryException), func, file, line, "%s, size=%d", getLastErrorText(), size);
	return p;
}


void *mem_calloc(int count, long size, const char *func, const char *file, int line) {
//	assert(count > 0);
//	assert(size > 0);
	void *p = calloc(count, size);
	if (! p)
		exception_throw(&(MemoryException), func, file, line, "%s, size=%d", getLastErrorText(), count*size);
	return p;
}


void mem_free(void *p) {
	if (p)
		free(p);
}


void *mem_realloc(void *p, int size, const char *func, const char *file, int line) {
//	assert(p);
//	assert(size > 0);
	p = realloc(p, size);
	if (! p)
		exception_throw(&(MemoryException), func, file, line, "%s, size=%d", getLastErrorText(), size);
	return p;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


