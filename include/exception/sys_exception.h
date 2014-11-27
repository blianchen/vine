#ifndef SYSEXCEPTION_INCLUDED
#define SYSEXCEPTION_INCLUDED
#include <exception/exception.h>


/**
 * Thrown to indicate that a memory allocation failed. Every object
 * constructor method may throw a MemoryException if the underlying
 * allocator failed.
 * @see Exception.h, Mem.h
 * @file
 */
extern Exception_T sys_exception;


#endif
