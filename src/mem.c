
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <exception/memory_exception.h>
#include <utils.h>

/**
 * Implementation of the Mem interface
 */


/* ----------------------------------------------------- Protected methods */

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

void *mem_malloc(int size, const char *func, const char *file, int line){
	assert(size > 0);
	void *p = malloc(size);
	if (! p)
		exception_throw(&(memory_exception), func, file, line, "%s, size=%d", getLastErrorText(), size);
	return p;
}


void *mem_calloc(int count, int size, const char *func, const char *file, int line) {
	assert(count > 0);
	assert(size > 0);
	void *p = calloc(count, size);
	if (! p)
		exception_throw(&(memory_exception), func, file, line, "%s, size=%d", getLastErrorText(), count*size);
	return p;
}


void mem_free(void *p) {
	if (p)
		free(p);
}


void *mem_realloc(void *p, int size, const char *func, const char *file, int line) {
	assert(p);
	assert(size > 0);
	p = realloc(p, size);
	if (! p)
		exception_throw(&(memory_exception), func, file, line, "%s, size=%d", getLastErrorText(), size);
	return p;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


