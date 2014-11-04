/* Modifications Copyright (c) J.S.A.Kapp 1994 - 1996.

	 MD5 Component Of RSAEURO using RSA Data Security, Inc.
	 MD5 Message Digest Algorithm.

	 All Trademarks Acknowledged.

	 Future versions may not use this modified RSADSI implementation
	 of MD5.
*/


/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
	 Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
*/

#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MD5 context. */
typedef struct {
  uint32_t state[4];                                   /* state (ABCD) */
  uint32_t count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;

void md5_init (MD5_CTX *);
void md5_update (MD5_CTX *, unsigned char *, unsigned int);
void md5_final (unsigned char [16], MD5_CTX *);


/* Standard library routines. */
#ifndef USE_ANSI
#define USE_ANSI 1
#endif

#ifdef USE_ANSI
#define R_memset(x, y, z) memset(x, y, z)
#define R_memcpy(x, y, z) memcpy(x, y, z)
#define R_memcmp(x, y, z) memcmp(x, y, z)
#else
void R_memset (unsigned char *, int, unsigned int);
void R_memcpy (unsigned char *, unsigned char *, unsigned int);
int R_memcmp (unsigned char *, unsigned char *, unsigned int);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MD5_H_ */
