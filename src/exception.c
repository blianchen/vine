
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>

#include <exception/exception.h>

/**
 * Implementation of the Exception interface. Defines the Thread local 
 * Exception stack and Exceptions used in the library. 
 * 
 * This implementation is a minor modification of the Except code found 
 * in David R. Hanson's excellent book "C Interfaces and Implementations".
 * See http://www.cs.princeton.edu/software/cii/
 *
 * @file
 */

/* ----------------------------------------------------------- Definitions */


/* Placeholder for systems exceptions. */
Exception_T sys_exception = {"sys_exception"};
Exception_T sql_exception = {"sql_exception"};
Exception_T memory_exception = {"memory_exception"};
Exception_T st_exception = {"st_exception"};
Exception_T net_exception = {"net_exception"};

/* Thread specific Exception stack */
ThreadData_T Exception_stack;

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

/* -------------------------------------------------------- Privat methods */

static void init_once(void) { ThreadData_create(Exception_stack); }

/* ----------------------------------------------------- Protected methods */

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

void exception_init(void) { pthread_once(&once_control, init_once); }

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


/* -------------------------------------------------------- Public methods */

#ifndef ZILD_PACKAGE_PROTECTED

void exception_throw(const Exception_T *e, const char *func, const char *file, int line, const char *cause, ...) {
	va_list ap;
	Exception_Frame *p = ThreadData_get(Exception_stack);
//	assert(e);
	if (p) {
		p->exception = e;
		p->func = func;
		p->file = file;
		p->line = line;
		if (cause) {
			va_start(ap, cause);
			vsnprintf(p->message, EXCEPTION_MESSAGE_LENGTH, cause, ap);
			va_end(ap);
		}
		pop_Exception_stack;
		longjmp(p->env, Exception_thrown);
	} else if (cause) {
		char message[EXCEPTION_MESSAGE_LENGTH + 1];
		va_start(ap, cause);
		vsnprintf(message, EXCEPTION_MESSAGE_LENGTH, cause, ap);
		va_end(ap);
		log_warn("%s: %s\n 	raised in %s at %s:%d\n", e->name, message, func ? func : "?", file ? file : "?", line);
		exit(1);
	} else {
		log_warn("%s: 0x%p\n 	raised in %s at %s:%d\n", e->name, e, func ? func : "?", file ? file : "?", line);
		exit(1);
	}
}

#endif
