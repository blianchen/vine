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
		/*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
		/* 0 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 1 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 2 */-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		/* 3 */0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,		// 0, 1, 2, 3,  4, 5, 6, 7,  8, 9,

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
#define SET_PROTOCOL(PORT) *(YYCURSOR-3)=0; U->protocol=U->token; U->port=PORT; goto authority


/* ------------------------------------------------------- Private methods */

static int parse_uri(T U) {
        param_t param = NULL;

proto:
	if (YYCURSOR >= YYLIMIT)
		return 0;
	YYTOKEN = YYCURSOR;

{
	unsigned char yych;

	yych = *U->buffer;
	switch (yych) {
	case '\t':
	case '\n':
	case '\r':
	case ' ':	goto yy2;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'n':
	case 'q':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy10;
	case 'h':	goto yy9;
	case 'm':	goto yy6;
	case 'o':	goto yy8;
	case 'p':	goto yy7;
	case 'r':	goto yy4;
	default:	goto yy11;
	}
yy2:
	++U->buffer;
	{
                goto proto;
         }
yy4:
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy15;
	case 'e':	goto yy56;
	default:	goto yy5;
	}
yy5:
	{
                goto proto;
         }
yy6:
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'z':	goto yy15;
	case 'y':	goto yy48;
	default:	goto yy5;
	}
yy7:
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy15;
	case 'o':	goto yy35;
	default:	goto yy5;
	}
yy8:
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy15;
	case 'r':	goto yy26;
	default:	goto yy5;
	}
yy9:
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy15;
	case 't':	goto yy19;
	default:	goto yy5;
	}
yy10:
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy15;
	default:	goto yy5;
	}
yy11:
	yych = *++U->buffer;
	goto yy5;
yy12:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy16;
	default:	goto yy13;
	}
yy13:
	U->buffer = U->marker;
	goto yy5;
yy14:
	++U->buffer;
	yych = *U->buffer;
yy15:
	switch (yych) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy14;
	case ':':	goto yy12;
	default:	goto yy13;
	}
yy16:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy17;
	default:	goto yy13;
	}
yy17:
	++U->buffer;
	{
                SET_PROTOCOL(UNKNOWN_PORT);
         }
yy19:
	yych = *++U->buffer;
	switch (yych) {
	case 't':	goto yy20;
	default:	goto yy15;
	}
yy20:
	yych = *++U->buffer;
	switch (yych) {
	case 'p':	goto yy21;
	default:	goto yy15;
	}
yy21:
	yych = *++U->buffer;
	switch (yych) {
	case ':':	goto yy22;
	default:	goto yy15;
	}
yy22:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy23;
	default:	goto yy13;
	}
yy23:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy24;
	default:	goto yy13;
	}
yy24:
	++U->buffer;
	{
                SET_PROTOCOL(HTTP_DEFAULT_PORT);
         }
yy26:
	yych = *++U->buffer;
	switch (yych) {
	case 'a':	goto yy27;
	default:	goto yy15;
	}
yy27:
	yych = *++U->buffer;
	switch (yych) {
	case 'c':	goto yy28;
	default:	goto yy15;
	}
yy28:
	yych = *++U->buffer;
	switch (yych) {
	case 'l':	goto yy29;
	default:	goto yy15;
	}
yy29:
	yych = *++U->buffer;
	switch (yych) {
	case 'e':	goto yy30;
	default:	goto yy15;
	}
yy30:
	yych = *++U->buffer;
	switch (yych) {
	case ':':	goto yy31;
	default:	goto yy15;
	}
yy31:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy32;
	default:	goto yy13;
	}
yy32:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy33;
	default:	goto yy13;
	}
yy33:
	++U->buffer;
	{
                SET_PROTOCOL(ORACLE_DEFAULT_PORT);
         }
yy35:
	yych = *++U->buffer;
	switch (yych) {
	case 's':	goto yy36;
	default:	goto yy15;
	}
yy36:
	yych = *++U->buffer;
	switch (yych) {
	case 't':	goto yy37;
	default:	goto yy15;
	}
yy37:
	yych = *++U->buffer;
	switch (yych) {
	case 'g':	goto yy38;
	default:	goto yy15;
	}
yy38:
	yych = *++U->buffer;
	switch (yych) {
	case 'r':	goto yy39;
	default:	goto yy15;
	}
yy39:
	yych = *++U->buffer;
	switch (yych) {
	case 'e':	goto yy40;
	default:	goto yy15;
	}
yy40:
	yych = *++U->buffer;
	switch (yych) {
	case 's':	goto yy41;
	default:	goto yy15;
	}
yy41:
	yych = *++U->buffer;
	switch (yych) {
	case 'q':	goto yy42;
	default:	goto yy15;
	}
yy42:
	yych = *++U->buffer;
	switch (yych) {
	case 'l':	goto yy43;
	default:	goto yy15;
	}
yy43:
	yych = *++U->buffer;
	switch (yych) {
	case ':':	goto yy44;
	default:	goto yy15;
	}
yy44:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy45;
	default:	goto yy13;
	}
yy45:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy46;
	default:	goto yy13;
	}
yy46:
	++U->buffer;
	{
                SET_PROTOCOL(POSTGRESQL_DEFAULT_PORT);
         }
yy48:
	yych = *++U->buffer;
	switch (yych) {
	case 's':	goto yy49;
	default:	goto yy15;
	}
yy49:
	yych = *++U->buffer;
	switch (yych) {
	case 'q':	goto yy50;
	default:	goto yy15;
	}
yy50:
	yych = *++U->buffer;
	switch (yych) {
	case 'l':	goto yy51;
	default:	goto yy15;
	}
yy51:
	yych = *++U->buffer;
	switch (yych) {
	case ':':	goto yy52;
	default:	goto yy15;
	}
yy52:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy53;
	default:	goto yy13;
	}
yy53:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy54;
	default:	goto yy13;
	}
yy54:
	++U->buffer;
	{
                SET_PROTOCOL(MYSQL_DEFAULT_PORT);
         }
yy56:
	yych = *++U->buffer;
	switch (yych) {
	case 'd':	goto yy57;
	default:	goto yy15;
	}
yy57:
	yych = *++U->buffer;
	switch (yych) {
	case 'i':	goto yy58;
	default:	goto yy15;
	}
yy58:
	yych = *++U->buffer;
	switch (yych) {
	case 's':	goto yy59;
	default:	goto yy15;
	}
yy59:
	yych = *++U->buffer;
	switch (yych) {
	case ':':	goto yy60;
	default:	goto yy15;
	}
yy60:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy61;
	default:	goto yy13;
	}
yy61:
	yych = *++U->buffer;
	switch (yych) {
	case '/':	goto yy62;
	default:	goto yy13;
	}
yy62:
	++U->buffer;
	{
                SET_PROTOCOL(REDIS_DEFAULT_PORT);
         }
}

authority:
	if (YYCURSOR >= YYLIMIT)
		return 1;
	YYTOKEN = YYCURSOR;

{
	unsigned char yych;
	unsigned int yyaccept = 0;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\v':
	case '\f':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '@':
	case ']':	goto yy77;
	case '\t':
	case '\n':
	case '\r':	goto yy66;
	case ' ':	goto yy68;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy71;
	case '/':	goto yy73;
	case ':':	goto yy75;
	case '[':	goto yy76;
	default:	goto yy69;
	}
yy66:
	++U->buffer;
yy67:
	{
                goto authority;
         }
yy68:
	yyaccept = 0;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy67;
	default:	goto yy89;
	}
yy69:
	yyaccept = 1;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy70;
	default:	goto yy89;
	}
yy70:
	{
                return 1;
         }
yy71:
	yyaccept = 2;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy72;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy99;
	case '.':	goto yy98;
	default:	goto yy89;
	}
yy72:
	{
                U->host = str_ndup(YYTOKEN, (int)(YYCURSOR - YYTOKEN));
                goto authority;
         }
yy73:
	yyaccept = 3;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:	goto yy74;
	case ' ':
	case '#':
	case ';':	goto yy88;
	case '?':	goto yy95;
	case '@':	goto yy92;
	case '[':
	case ']':	goto yy93;
	default:	goto yy90;
	}
yy74:
	{
                *YYCURSOR = 0;
                U->path = uri_unescape(YYTOKEN);
                return 1;
         }
yy75:
	yyaccept = 1;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy70;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy83;
	default:	goto yy89;
	}
yy76:
	yyaccept = 1;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case '%':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy78;
	default:	goto yy70;
	}
yy77:
	yych = *++U->buffer;
	goto yy70;
yy78:
	++U->buffer;
	yych = *U->buffer;
	switch (yych) {
	case '%':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case ':':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy78;
	case ']':	goto yy81;
	default:	goto yy80;
	}
yy80:
	U->buffer = U->marker;
	switch (yyaccept) {
	case 0: 	goto yy67;
	case 1: 	goto yy70;
	case 2: 	goto yy72;
	case 3: 	goto yy74;
	case 4: 	goto yy85;
	default:	goto yy96;
	}
yy81:
	++U->buffer;
	{
                U->ip6 = 1;
                U->host = str_ndup(YYTOKEN + 1, (int)(YYCURSOR - YYTOKEN - 2));
                goto authority;
         }
yy83:
	yyaccept = 4;
	U->marker = ++U->buffer;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy85;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':	goto yy83;
	case '@':	goto yy86;
	default:	goto yy88;
	}
yy85:
	{
                U->portStr = YYTOKEN + 1; // read past ':'
                U->port = str_parseInt(U->portStr);
                goto authority;
         }
yy86:
	++U->buffer;
yy87:
	{
                *(YYCURSOR - 1) = 0;
                U->user = YYTOKEN;
                char *p = strchr(U->user, ':');
                if (p) {
                        *(p++) = 0;
                        U->password = uri_unescape(p);
                }
                uri_unescape(U->user);
                goto authority;
         }
yy88:
	++U->buffer;
	yych = *U->buffer;
yy89:
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy80;
	case '@':	goto yy86;
	default:	goto yy88;
	}
yy90:
	yyaccept = 3;
	U->marker = ++U->buffer;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:	goto yy74;
	case ' ':
	case '#':
	case ';':	goto yy88;
	case '?':	goto yy95;
	case '@':	goto yy92;
	case '[':
	case ']':	goto yy93;
	default:	goto yy90;
	}
yy92:
	yych = *++U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case ' ':
	case '#':
	case ';':	goto yy87;
	default:	goto yy94;
	}
yy93:
	++U->buffer;
	yych = *U->buffer;
yy94:
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case ' ':
	case '#':
	case ';':	goto yy74;
	case '?':	goto yy97;
	default:	goto yy93;
	}
yy95:
	yyaccept = 5;
	yych = *(U->marker = ++U->buffer);
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy96;
	default:	goto yy89;
	}
yy96:
	{
                *(YYCURSOR-1) = 0;
                U->path = uri_unescape(YYTOKEN);
                goto query;
         }
yy97:
	yych = *++U->buffer;
	goto yy96;
yy98:
	++U->buffer;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy80;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy101;
	case '@':	goto yy86;
	default:	goto yy88;
	}
yy99:
	yyaccept = 2;
	U->marker = ++U->buffer;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy72;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy99;
	case '.':	goto yy98;
	case '@':	goto yy86;
	default:	goto yy88;
	}
yy101:
	yyaccept = 2;
	U->marker = ++U->buffer;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '[':
	case ']':	goto yy72;
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':	goto yy101;
	case '.':	goto yy98;
	case '@':	goto yy86;
	default:	goto yy88;
	}
}

query:
        if (YYCURSOR >= YYLIMIT)
		return 1;
	YYTOKEN =  YYCURSOR;

{
	unsigned char yych;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '#':	goto yy107;
	default:	goto yy106;
	}
yy105:
	{
                *YYCURSOR = 0;
                U->query = str_ndup(YYTOKEN, (int)(YYCURSOR - YYTOKEN));
                YYCURSOR = YYTOKEN; // backtrack to start of query string after terminating it and
                goto params;
         }
yy106:
	yych = *++U->buffer;
	goto yy110;
yy107:
	++U->buffer;
	{
                return 1;
         }
yy109:
	++U->buffer;
	yych = *U->buffer;
yy110:
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case '#':	goto yy105;
	default:	goto yy109;
	}
}

params:
	if (YYCURSOR >= YYLIMIT)
		return 1;
	YYTOKEN =  YYCURSOR;

{
	unsigned char yych;
	yych = *U->buffer;
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case ' ':	goto yy117;
	case '=':	goto yy115;
	default:	goto yy113;
	}
yy113:
	U->ctx = U->buffer + 1;
	yych = *(U->marker = ++U->buffer);
	if (yych >= '!') goto yy124;
yy114:
	{
                return 1;
         }
yy115:
	++U->buffer;
	yych = *U->buffer;
	goto yy119;
yy116:
	{
                *YYTOKEN++ = 0;
                if (*(YYCURSOR - 1) == '&')
                        *(YYCURSOR - 1) = 0;
                if (! param) // format error
                        return 1;
                param->value = uri_unescape(YYTOKEN);
                goto params;
         }
yy117:
	yych = *++U->buffer;
	goto yy114;
yy118:
	++U->buffer;
	yych = *U->buffer;
yy119:
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:	goto yy116;
	case '&':	goto yy120;
	default:	goto yy118;
	}
yy120:
	yych = *++U->buffer;
	goto yy116;
yy121:
	++U->buffer;
	U->buffer = U->ctx;
	{
                NEW(param);
                param->name = YYTOKEN;
                param->next = U->params;
                U->params = param;
                goto params;
         }
yy123:
	U->ctx = U->buffer + 1;
	++U->buffer;
	yych = *U->buffer;
yy124:
	switch (yych) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	case 0x0E:
	case 0x0F:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1A:
	case 0x1B:
	case 0x1C:
	case 0x1D:
	case 0x1E:
	case 0x1F:
	case ' ':	goto yy125;
	case '=':	goto yy121;
	default:	goto yy123;
	}
yy125:
	U->buffer = U->marker;
	goto yy114;
}

        return 0;
}

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
	YYCURSOR = U->data;
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
			snprintf(port, 10, ":%d", U->port);
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
	return url;
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
