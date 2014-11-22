
#ifndef _UTILS_H_
#define _UTILS_H_

#include <errno.h>


#ifdef WIN32
#define getLastError() GetLastError()
#else
#define getLastError() errno
#endif
extern char* getLastErrorText();

// 转大写，不修改原字符内容 (strupr会修改原字符串内容，也不是c标准)
#define STRUPR(in, out) \
	do { \
		char* bp = (out); \
		const char* cp = (in); \
		unsigned char c; \
		while (*cp != '\0') { \
			c = * (unsigned char *)cp; \
			if (c >= 0x80) break; /* 非ascii字符 */ \
			if (c >= 'a' && c <= 'z') { \
				c -= 'a' - 'A'; \
			} \
			*bp = c; \
			cp++; \
			bp++; \
		} \
		*bp = '\0'; \
	} while (0);


#ifdef __cplusplus
extern "C" {
#endif


//// trim实现，会修改原字符串内容
//#define IS_SPACE(x) ((x)==' '||(x)=='\r'||(x)=='\n'||(x)=='\f'||(x)=='\b'||(x)=='\t')
#define IS_SPACE(x) ((x)==' '||(x)=='\r'||(x)=='\n'||(x)=='\t')
extern char* trim(char* str);

int cpu_count(void);

/*
 * 字节数组转为大写的16进制字符串，以'\0'终止
 */
void byteToUperHexStr(const unsigned char* source, int sourceLen, char* dest);
/*
 * 16进制字符串（不区分大小写）转为字节数组
 * dest 长度为 sourceLen / 2
 */
void hexStrToByte(const char* source, int sourceLen, unsigned char* dest);


#ifdef __cplusplus
}
#endif

#endif
