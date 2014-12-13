
#include <str.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

#include <mem.h>
#include <exception/sys_exception.h>


#define INIT_STR_LEN 256


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

int str_isEqual(const char *a, const char *b) {
	if (a && b) {
		while (*a && *b)
			if (toupper(*a++) != toupper(*b++))
				return 0;
		return (*a == *b);
	}
	return 0;
}


int str_isByteEqual(const char *a, const char *b) {
	if (a && b) {
		while (*a && *b)
			if (*a++ != *b++)
				return 0;
		return (*a == *b);
	}
	return 0;
}


int str_startsWith(const char *a, const char *b) {
	if (a && b) {
		do
			if (*a++ != *b++)
				return 0;
		while (*b);
		return 1;
	}
	return 0;
}


//char *str_copy(char *dest, const char *src, int n) {
//	if (src && dest && (n > 0)) {
//		char *t = dest;
//		while (*src && n--)
//			*t++ = *src++;
//		*t = 0;
//	} else if (dest)
//		*dest = 0;
//	return dest;
//}


// We do not use strdup so we can throw MemoryException on OOM
char *str_dup(const char *s) {
	char *t = NULL;
	if (s) {
		size_t n = strlen(s) + 1;
		t = MALLOC(n);
		memcpy(t, s, n);
	}
	return t;
}

char *str_ndup(const char *s, int n) {
	char *t = NULL;
	if (s) {
		int l = (int) strlen(s);
		n = l < n ? l : n; // Use the actual length of s if shorter than n
		t = MALLOC(n + 1);
		memcpy(t, s, n);
		t[n] = 0;
	}
	return t;
}


char *str_cat(const char *s, ...) {
	char *t = 0;
	if (s) {
		va_list ap;
		va_start(ap, s);
		t = str_vcat(s, ap);
		va_end(ap);
	}
	return t;
}

char *str_vcat(const char *s, va_list ap) {
	char *buf = NULL;
	if (s) {
		int n = 0;
		va_list ap_copy;
		int size = INIT_STR_LEN;
		buf = MALLOC(size);
		while (1) {
			va_copy(ap_copy, ap);
			n = vsnprintf(buf, size, s, ap_copy);
			va_end(ap_copy);
			if (n < size)
				break;
			size = n + 1;
			REALLOC(buf, size);
		}
	}
	return buf;
}

int str_parseInt(const char *s) {
	if (STR_UNDEF(s))
		THROW(sys_exception, "NumberFormatException: For input string null");
	errno = 0;
	char *e;
	int i = (int) strtol(s, &e, 10);
	if (errno || (e == s))
		THROW(sys_exception, "NumberFormatException: For input string %s -- %s", s, getLastErrorText());
	return i;
}

long long str_parseLLong(const char *s) {
	if (STR_UNDEF(s))
		THROW(sys_exception, "NumberFormatException: For input string null");
	errno = 0;
	char *e;
	long long ll = strtoll(s, &e, 10);
	if (errno || (e == s))
		THROW(sys_exception, "NumberFormatException: For input string %s -- %s", s, getLastErrorText());
	return ll;
}

double str_parseDouble(const char *s) {
	if (STR_UNDEF(s))
		THROW(sys_exception, "NumberFormatException: For input string null");
	errno = 0;
	char *e;
	double d = strtod(s, &e);
	if (errno || (e == s))
		THROW(sys_exception, "NumberFormatException: For input string %s -- %s", s, getLastErrorText());
	return d;
}

char* str_fromInt(int i) {
	char* s = MALLOC(16);
	sprintf(s, "%d", i);
	return s;
}

char* str_fromLLong(long long ll) {
	char* s = MALLOC(24);
	sprintf(s, "%lld", ll);
	return s;
}

char* str_fromDouble(double d) {
	char* s = MALLOC(24);
	sprintf(s, "%g", d);
	return s;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

