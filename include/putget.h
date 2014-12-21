/*
 * putget.h
 *
 *  Created on: Dec 20, 2014
 *      Author: blc
 */

#ifndef _PUTGET_H_
#define _PUTGET_H_

#define put8(s,n) do { \
  (s)[0] = (char)((n) & 0xff); \
  (s) += 1; \
} while (0)

#define put16le(s,n) do { \
  (s)[0] = (n) & 0xff;  \
  (s)[1] = ((n) >>  8) & 0xff; \
  (s) += 2; \
} while (0) \

#define put32le(s,n) do { \
  (s)[0] = (n) & 0xff;  \
  (s)[1] = ((n) >>  8) & 0xff; \
  (s)[2] = ((n) >>  16) & 0xff; \
  (s)[3] = ((n) >>  24) & 0xff; \
  (s) += 4; \
} while (0)

#define put16be(s,n) do { \
  (s)[0] = ((n) >>  8) & 0xff; \
  (s)[1] = (n) & 0xff; \
  (s) += 2; \
} while (0)

#define put32be(s,n) do {  \
  (s)[0] = ((n) >>  24) & 0xff; \
  (s)[1] = ((n) >>  16) & 0xff; \
  (s)[2] = ((n) >>  8) & 0xff;  \
  (s)[3] = (n) & 0xff; \
  (s) += 4; \
} while (0)

#define put64be(s,n) do {  \
  (s)[0] = ((n) >>  56) & 0xff; \
  (s)[1] = ((n) >>  48) & 0xff; \
  (s)[2] = ((n) >>  40) & 0xff; \
  (s)[3] = ((n) >>  32) & 0xff; \
  (s)[4] = ((n) >>  24) & 0xff; \
  (s)[5] = ((n) >>  16) & 0xff; \
  (s)[6] = ((n) >>  8)  & 0xff; \
  (s)[7] = (n) & 0xff; \
  (s) += 8; \
} while (0)

#define get8(s) \
     ((s) += 1, \
      ((unsigned char *)(s))[-1] & 0xff)

#define get16le(s) \
     ((s) += 2, \
      (((((unsigned char *)(s))[-1] << 8) | \
	((unsigned char *)(s))[-2])) & 0xffff)

#define get32le(s) \
     ((s) += 4, \
      ((((unsigned char *)(s))[-1] << 24) | \
       (((unsigned char *)(s))[-2] << 16) | \
       (((unsigned char *)(s))[-3] << 8) | \
       ((unsigned char *)(s))[-4]))

#define get16be(s) \
     ((s) += 2, \
      (((((unsigned char *)(s))[-2] << 8) | \
	((unsigned char *)(s))[-1])) & 0xffff)

#define get32be(s) \
     ((s) += 4, \
      ((((unsigned char *)(s))[-4] << 24) | \
       (((unsigned char *)(s))[-3] << 16) | \
       (((unsigned char *)(s))[-2] << 8) | \
       ((unsigned char *)(s))[-1]))

#define get64be(s) \
     ((s) += 8, \
      (((EI_ULONGLONG)((unsigned char *)(s))[-8] << 56) | \
       ((EI_ULONGLONG)((unsigned char *)(s))[-7] << 48) | \
       ((EI_ULONGLONG)((unsigned char *)(s))[-6] << 40) | \
       ((EI_ULONGLONG)((unsigned char *)(s))[-5] << 32) | \
       ((EI_ULONGLONG)((unsigned char *)(s))[-4] << 24) | \
       ((EI_ULONGLONG)((unsigned char *)(s))[-3] << 16) | \
       ((EI_ULONGLONG)((unsigned char *)(s))[-2] << 8)  | \
        (EI_ULONGLONG)((unsigned char *)(s))[-1]))

#endif /* CORELIB_INCLUDE_PUTGET_H_ */
