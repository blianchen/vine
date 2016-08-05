/*
 * uri.c
 *
 *  Created on: 2014年9月21日
 *      Author: blc
 */
#include <uri.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <mem.h>
#include <str.h>

#include <db/dbconfig.h>

#define uchar_t unsigned char

const char HEX2DEC[256] = {
		/*      0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
		/* 0 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 1 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 2 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 3 */ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,		// 0, 1, 2, 3,  4, 5, 6, 7,  8, 9,

		/* 4 */-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// -1, A, B, C,  D, E, F,-1, -1,-1,
		/* 5 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 6 */-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// -1, a, b, c,  d, e, f,-1,
		/* 7 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

		/* 8 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 9 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* A */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* B */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

		/* C */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* D */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* E */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* F */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

int uri_decode(const char * psrc, int len, char * pres) {
	// Note from RFC1630:  "Sequences which start with a percent sign
	// but are not followed by two hexadecimal characters (0-9, A-F) are reserved
	// for future extension"
	unsigned char* srcuri = (unsigned char *) psrc;
	const unsigned char * const SRC_END = srcuri + len;
	const unsigned char * const SRC_LAST_DEC = SRC_END - 2; // last decodable '%'

	char * pstart = (char *) MALLOC(len);
	char * pend = pstart;

	while (srcuri < SRC_LAST_DEC) {
		if (*srcuri == '%') {
			char dec1, dec2;
			if (-1 != (dec1 = HEX2DEC[*(srcuri + 1)]) && -1 != (dec2 = HEX2DEC[*(srcuri + 2)])) {
				*pend++ = (dec1 << 4) + dec2;
				srcuri += 3;
				continue;
			}
		}
		*pend++ = *srcuri++;
	}

	// the last 2- chars
	while (srcuri < SRC_END)
		*pend++ = *srcuri++;
	int plen = (pend - pstart);
	memcpy(pres, pstart, plen);
	FREE(pstart);
	return plen;
}

char SAFE[256] = {
		/*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
		/* 0 */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 1 */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 2 */0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //  ,!,",#, $,%,&,', (,),*,+, ,,-,.,/
		/* 3 */1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, // 0,1,2,3, 4,5,6,7, 8,9,:,;, <,=,>,?

		/* 4 */1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // @,A,B,C, D,E,F,G, H,I,J,K, L,M,N,O
		/* 5 */1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // P,Q,R,S, T,U,V,W, X,Y,Z,[, \,],^,_
		/* 6 */0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // `,a,b,c, d,e,f,g, h,i,j,k, l,m,n,o
		/* 7 */1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, // p,q,r,s, t,u,v,w, x,y,z,{, |,},~,

		/* 8 */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* 9 */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* A */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* B */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		/* C */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* D */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* E */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* F */0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int uri_encode(const char * psrc, int len, char * pres) {
	unsigned char* srcuri = (unsigned char*) psrc;
	const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
	unsigned char * pstart = (unsigned char *) MALLOC(len * 3);
	unsigned char * pend = pstart;
	const unsigned char * const SRC_END = srcuri + len;
	for (; srcuri < SRC_END; ++srcuri) {
		if (SAFE[*srcuri]) {
			*pend++ = *srcuri;
		} else {
			// escape this char
			*pend++ = '%';
			*pend++ = DEC2HEX[*srcuri >> 4];
			*pend++ = DEC2HEX[*srcuri & 0x0F];
		}
	}
	int plen = pend - pstart;
	memcpy(pres, pstart, plen);
	FREE(pstart);
	return plen;
}

typedef struct param_t {
	char *name;
	char *value;
	struct param_t *next;
}*param_t;

#define T uri_t
struct URI_S {
	int ip6;
	int port;
	char *ref;
	char *path;
	char *host;
	char *user;
	char *qptr;
	char *query;
	char *portStr;
	char *protocol;
	char *password;
	char *toString;
	param_t params;
	char **paramNames;
	uchar_t *data;

	uchar_t *buffer;
	uchar_t *marker, *ctx, *limit, *token;
/* Keep the above align with zild uri_T */
};


#define UNKNOWN_PORT -1
#define YYCTYPE       unsigned char
#define YYCURSOR      U->buffer
#define YYLIMIT       U->limit
#define YYMARKER      U->marker
#define YYCTXMARKER   U->ctx
#define YYFILL(n)     ((void)0)
#define YYTOKEN       U->token
#define SET_PROTOCOL(PORT) *(YYCURSOR-3)=0; U->protocol=(char*)U->token; U->port=PORT; goto authority


/* ------------------------------------------------------- Private methods */

#include "parseuri.rec"

static inline int _x2b(uchar_t *x) {
	register int b;
	b = ((x[0] >= 'A') ? ((x[0] & 0xdf) - 'A')+10 : (x[0] - '0'));
	b *= 16;
	b += (x[1] >= 'A' ? ((x[1] & 0xdf) - 'A')+10 : (x[1] - '0'));
	return b;
}


static inline uchar_t *_b2x(uchar_t b, uchar_t *x) {
        static const char _b2x_table[] = "0123456789ABCDEF";
        *x++ = '%';
        *x++ = _b2x_table[b >> 4];
        *x = _b2x_table[b & 0xf];
        return x;
}


static void _freeParams(param_t p) {
	param_t q;
	for (q = NULL; p; p = q) {
		q = p->next;
		FREE(p);
	}
}


static T _ctor(uchar_t *data) {
	T U;
	NEW(U);
	U->data = data;
	U->buffer = U->data;
	U->port = UNKNOWN_PORT;
	YYLIMIT = U->data + strlen((char*) U->data);
	if (!parse_uri(U))
		uri_free(&U);
	return U;
}


/* -------------------------------------------------------- Public methods */

T uri_new(const char *url) {
	if (STR_UNDEF(url))
		return NULL;
	return _ctor((uchar_t*) str_dup(url));
}

T uri_create(const char *url, ...) {
	if (STR_UNDEF(url))
		return NULL;
	va_list ap;
	va_start(ap, url);
	T U = _ctor((uchar_t*) str_vcat(url, ap));
	va_end(ap);
	return U;
}

void uri_free(T *U) {
	assert(U && *U);
    _freeParams((*U)->params);
    FREE((*U)->paramNames);
	FREE((*U)->toString);
	FREE((*U)->query);
	FREE((*U)->data);
	FREE((*U)->host);
	FREE(*U);
}
/* ------------------------------------------------------------ Properties */


const char *uri_getProtocol(T U) {
	assert(U);
	return U->protocol;
}


const char *uri_getUser(T U) {
	assert(U);
	return U->user;
}


const char *uri_getPassword(T U) {
	assert(U);
	return U->password;
}


const char *uri_getHost(T U) {
	assert(U);
	return U->host;
}


int uri_getPort(T U) {
	assert(U);
	return U->port;
}


const char *uri_getPath(T U) {
	assert(U);
	return U->path;
}


const char *uri_getQueryString(T U) {
	assert(U);
	return U->query;
}


const char **uri_getParameterNames(T U) {
	assert(U);
	if (U->params && (U->paramNames == NULL)) {
			param_t p;
			int i = 0, len = 0;
			for (p = U->params; p; p = p->next) len++;
			U->paramNames = MALLOC((len + 1) * sizeof *(U->paramNames));
			for (p = U->params; p; p = p->next)
					U->paramNames[i++] = p->name;
			U->paramNames[i] = NULL;
	}
	return (const char **)U->paramNames;
}


const char *uri_getParameter(T U, const char *name) {
	assert(U);
	assert(name);
	param_t p;
	for (p = U->params; p; p = p->next) {
		if (str_isByteEqual(p->name, name))
			return p->value;
	}
	return NULL;
}


/* ---------------------------------------------------------------- Public */


const char *uri_toString(T U) {
	assert(U);
	if (! U->toString) {
		uchar_t port[11] = {};
		if (U->portStr) // port seen in URL
			snprintf((char*)port, 10, ":%d", U->port);
		U->toString = str_cat("%s://%s%s%s%s%s%s%s%s%s%s%s",
                                      U->protocol,
                                      U->user ? U->user : "",
                                      U->password ? ":" : "",
                                      U->password ? U->password : "",
                                      U->user ? "@" : "",
                                      U->ip6 ? "[" : "",
                                      U->host ? U->host : "",
                                      U->ip6 ? "]" : "",
                                      port,
                                      U->path ? U->path : "",
                                      U->query ? "?" : "",
                                      U->query ? U->query : "");
	}
	return U->toString;
}

char *uri_unescape(unsigned char *url) {
	if (STR_DEF(url)) {
		register int x, y;
		for (x = 0, y = 0; url[y]; x++, y++) {
			if ((url[x] = url[y]) == '+')
				url[x] = ' ';
			else if (url[x] == '%') {
				if (!(url[x + 1] && url[x + 2]))
					break;
				url[x] = _x2b(url + y + 1);
				y += 2;
			}
		}
		url[x] = 0;
	}
	return (char *)url;
}

//char *uri_escape(const char *url) {
//	char *escaped = 0;
//	if (url) {
//		char *p;
//		int i, n;
//		for (n = i = 0; url[i]; i++)
//			if (urlunsafe[(unsigned char) (url[i])])
//				n += 2;
//		p = escaped = ALLOC(i + n + 1);
//		for (; *url; url++, p++) {
//			if (urlunsafe[(unsigned char) (*p = *url)])
//				p = b2x(*url, p);
//		}
//		*p = 0;
//	}
//	return escaped;
//}
