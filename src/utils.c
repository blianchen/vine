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

#ifdef WIN32
#include <windows.h>
#else
//#include <wchar.h>
#endif

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


char* trim(char* str) {
	char *start=NULL, *end=NULL;
	if(str==NULL) return NULL;

	while( IS_SPACE(*str) ) str++;

	for(start=str;(*str)!='\0';++str) {
		if ( !IS_SPACE(*str) )
			end=str;
	}
	if(end)  *(end+1) = '\0';
	return start;
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

#endif
