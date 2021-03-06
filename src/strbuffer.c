
//#include <db/dbconfig.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//#include <exception/sql_exception.h>
#include <exception/sys_exception.h>
#include <mem.h>
#include <str.h>

#include <strbuffer.h>

#define STRLEN 256

/**
 * Implementation of the StringBuffer interface.
 */

/* ----------------------------------------------------------- Definitions */

#define T str_buffer_t
struct T {
	int used;
	int length;
	char *buffer;
};


/* ------------------------------------------------------- Private methods */

static inline void append(T S, const char *s, va_list ap) {
	va_list ap_copy;
	while (1) {
		va_copy(ap_copy, ap);
		int n = vsnprintf((char*) (S->buffer + S->used), S->length - S->used, s, ap_copy);
		va_end(ap_copy);
		if ((S->used + n) < S->length) {
			S->used += n;
			break;
		}
		S->length += STRLEN + n;
		REALLOC(S->buffer, S->length);
	}
}

/* Replace all occurences of ? in this string buffer with prefix[1..99] */
static int prepare(T S, char prefix) {
	int n, i;
	for (n = i = 0; S->buffer[i]; i++)
		if (S->buffer[i] == '?')
			n++;
//	if (n > 99)
//		THROW(sql_exception, "Max 99 parameters are allowed in a prepared statement. Found %d parameters in statement", n);
//	else if (n) {
		int j, xl;
		char x[3] = { prefix };
		int required = (n * 2) + S->used;
		if (required >= S->length) {
			S->length = required;
			REALLOC(S->buffer, S->length);
		}
		for (i = 0, j = 1; (j <= n); i++) {
			if (S->buffer[i] == '?') {
				if (j < 10) {
					xl = 2;
					x[1] = j + '0';
				} else {
					xl = 3;
					x[1] = (j / 10) + '0';
					x[2] = (j % 10) + '0';
				}
				memmove(S->buffer + i + xl, S->buffer + i + 1, (S->used - (i + 1)));
				memmove(S->buffer + i, x, xl);
				S->used += xl - 1;
				j++;
			}
		}
		S->buffer[S->used] = 0;
//	}
	return n;
}

static inline T ctor(int hint) {
	T S;
	NEW(S);
	S->length = hint;
	S->buffer = MALLOC(hint);
	*S->buffer = 0;
	return S;
}

/* ----------------------------------------------------- Protected methods */

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T strbuffer_new(const char *s) {
	return strbuffer_append(ctor(STRLEN), "%s", s);
}

T strbuffer_create(int hint) {
	if (hint <= 0)
		THROW(sys_exception, "Illegal hint value");
	return ctor(hint);
}

void strbuffer_free(T *S) {
	assert(S && *S);
	FREE((*S)->buffer);
	FREE(*S);
}

T strbuffer_append(T S, const char *s, ...) {
	assert(S);
	if (STR_DEF(s)) {
		va_list ap;
		va_start(ap, s);
		append(S, s, ap);
		va_end(ap);
	}
	return S;
}

T strbuffer_vappend(T S, const char *s, va_list ap) {
	assert(S);
	if (STR_DEF(s)) {
		va_list ap_copy;
		va_copy(ap_copy, ap);
		append(S, s, ap_copy);
		va_end(ap_copy);
	}
	return S;
}

T strbuffer_set(T S, const char *s, ...) {
	assert(S);
	strbuffer_clear(S);
	if (STR_DEF(s)) {
		va_list ap;
		va_start(ap, s);
		append(S, s, ap);
		va_end(ap);
	}
	return S;
}


T strbuffer_vset(T S, const char *s, va_list ap) {
	assert(S);
	strbuffer_clear(S);
	if (STR_DEF(s)) {
		va_list ap_copy;
		va_copy(ap_copy, ap);
		append(S, s, ap_copy);
		va_end(ap_copy);
	}
	return S;
}

int strbuffer_length(T S) {
	assert(S);
	return S->used;
}

T strbuffer_clear(T S) {
	assert(S);
	S->used = 0;
	*S->buffer = 0;
	return S;
}

const char *strbuffer_toString(T S) {
	assert(S);
	return (const char*) S->buffer;
}

int strbuffer_prepare4postgres(T S) {
	assert(S);
	return prepare(S, '$');
}

int strbuffer_prepare4oracle(T S) {
	assert(S);
	return prepare(S, ':');
}

T strbuffer_trim(T S) {
	assert(S);
	// Right trim and remove trailing semicolon
	while (S->used && ((S->buffer[S->used - 1] == ';') || isspace(S->buffer[S->used - 1])))
		S->buffer[--S->used] = 0;
	// Left trim
	if (isspace(*S->buffer)) {
		int i;
		for (i = 0; isspace(S->buffer[i]); i++)
			;
		memmove(S->buffer, S->buffer + i, S->used - i);
		S->used -= i;
		S->buffer[S->used] = 0;
	}
	return S;
}


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

