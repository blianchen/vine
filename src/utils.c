/*
 * utils.c
 *
 *  Created on: 2014年9月11日
 *      Author: blc
 */

#include <utils.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#include <mem.h>
#include <putget.h>

#ifdef WIN32
#include <windows.h>
#else
//#include <wchar.h>
#endif

#include "kv_options.rec"
void clean_kv_option(kv_option_t *kv) {
	if (*kv == NULL) return;
	kv_option_t kv1, kv2;
	kv1 = *kv;
	kv2 = kv1->next;
	FREE(kv1);
	while (kv2) {
		kv1 = kv2;
		kv2 = kv1->next;
		FREE(kv1);
	}
}

int cpu_count(void) {
	int n;
#if defined (_SC_NPROCESSORS_ONLN)
	n = (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined (_SC_NPROC_ONLN)
	n = (int) sysconf(_SC_NPROC_ONLN);
#elif defined (HPUX)
#include <sys/mpctl.h>
	n = mpctl(MPC_GETNUMSPUS, 0, 0);
#else
	n = -1;
	errno = ENOSYS;
#endif
	return n;
}


unsigned int ELFHash(char* str, unsigned int len) {
   unsigned int hash = 0;
   unsigned int x    = 0;
   unsigned int i    = 0;
   for(i = 0; i < len; str++, i++) {
      hash = (hash << 4) + (*str);
      if((x = hash & 0xF0000000L) != 0) {
         hash ^= (x >> 24);
      }
      hash &= ~x;
   }
   return hash;
}


/**
* Create an error message from GetLastError() using the
*  FormatMessage API Call...
*/
#ifdef WIN32
TCHAR lastErrBuf[1024];
TCHAR* getLastErrorText() {
	DWORD dwRet;
	TCHAR* lpszTemp = NULL;

	dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		GetLastError(),
		LANG_NEUTRAL,
		(TCHAR*)&lpszTemp,
		0,
		NULL);

	/* supplied buffer is not long enough */
	if (!dwRet || ((long)1023 < (long)dwRet + 14)) {
		lastErrBuf[0] = TEXT('\0');
	}
	else {
		lpszTemp[lstrlen(lpszTemp) - 2] = TEXT('\0');  /*remove cr and newline character */
		_sntprintf_s(lastErrBuf, 1024, TEXT("%s (0x%x)"), lpszTemp, GetLastError());
	}

	if (lpszTemp) {
		GlobalFree((HGLOBAL)lpszTemp);
	}

	return lastErrBuf;
}
#else
char* getLastErrorText() {
	return strerror(errno);
}


//字节流转换为十六进制字符串
void byteToUperHexStr(const unsigned char* source, int sourceLen, char* dest) {
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i++) {
        highByte = source[i] >> 4;
        lowByte = source[i] & 0x0f ;

        highByte += 0x30;

        if (highByte > 0x39)
        	dest[i * 2] = highByte + 0x07;
        else
        	dest[i * 2] = highByte;

        lowByte += 0x30;
        if (lowByte > 0x39)
            dest[i * 2 + 1] = lowByte + 0x07;
        else
            dest[i * 2 + 1] = lowByte;
    }
}

//十六进制字符串转换为字节流
void hexStrToByte(const char* source, int sourceLen, unsigned char* dest) {
    short i;
    unsigned char highByte, lowByte;

    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper((int)source[i]);
        lowByte  = toupper((int)source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
}


unsigned int leb2i(unsigned char b[], int len) {
	switch (len) {
	case 1:
		return get8(b);
	case 2:
		return get16le(b);
	case 4:
		return get32le(b);
	}
	return 0;
}

void lei2b(unsigned char b[], int len, unsigned int i) {
	switch (len) {
	case 1:
		put8(b, i);
		break;
	case 2:
		put16le(b, i);
		break;
	case 4:
		put32le(b, i);
		break;
	default:
		break;
	}
}

#endif
