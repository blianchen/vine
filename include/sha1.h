/*
	SHS.H - header file for Secure Hash Standard Code

    Copyright (c) J.S.A.Kapp 1994 - 1996.

	RSAEURO - RSA Library compatible with RSAREF 2.0.

	All functions prototypes are the Same as for RSAREF.
	To aid compatiblity the source and the files follow the
	same naming comventions that RSAREF uses.  This should aid
				direct importing to your applications.

	This library is legal everywhere outside the US.  And should
	NOT be imported to the US and used there.

	Secure Hash Standard Code header file.

	Revision 1.00. - JSAK

    Revision 1.03. - JSAK
*/

////sha-1算法

#ifndef _SHS_H_
#define _SHS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================ sha1.h ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

typedef struct {
    uint32_t state[5];	/* Message digest */
    uint32_t count[2];	/* 64-bit bit count */
    unsigned char buffer[64];	/* SHS data buffer */
} SHA1_CTX;

void sha1_init(SHA1_CTX* context);
void sha1_update(SHA1_CTX* context, const unsigned char* data, int len);
void sha1_final(unsigned char digest[20], SHA1_CTX* context);


#ifdef __cplusplus
}
#endif

#endif /* _SHS_H_ */
