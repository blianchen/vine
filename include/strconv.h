
#ifndef _STRCONV_H_
#define _STRCONV_H_

#include <stdlib.h>

// 预定义的字符集名最大长度
#define MAX_CODESET_NAME_LEN 16

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *aliasName;
	char *codeSet;
} CodeSetName;

static const CodeSetName charconv_codeSets[] = {
#if defined(__CYGWIN__)
	{"ASCII", "en_US.ascii"},
	{"UTF8", "zh_CN.utf-8"},
	{"GB2312", "zh_CN.GB2312"},
	{"GBK", "zh_CN.GBK"},
	{"BIG5", "zh_TW.BIG5"},
	{"SJIS", "ja_JP.SJIS"},
	{"EUCJP", "ja_JP.EUC-JP"},
	{"EUCKR", "ko_KR.EUC-KR"}
#elif defined(WIN32)
	{"ASCII", "en_US.ascii"},
	{"UTF8", "zh_CN.utf-8"},
	{"GB2312", "zh_CN.GB2312"},
	{"GBK", "zh_CN.GBK"},
	{"BIG5", "zh_TW.BIG5"},
	{"SJIS", "ja_JP.SJIS"},
	{"EUCJP", "ja_JP.EUC-JP"},
	{"EUCKR", "ko_KR.EUC-KR"}
#elif defined(__MINGW64__)
	{"ASCII", "ascii"},
	{"UTF8", ".utf-8"},
	{"GB2312", ".936"},
	{"GBK", ".936"},
//	{"GB18030 ".936"},
	{"BIG5", ".936"}
#else
	{"ASCII", "en_US.ascii"},
	{"UTF8", "zh_CN.utf-8"},
	{"GB2312", "zh_CN.GB2312"},
	{"GBK", "zh_CN.GBK"},
//	{"GB18030 "zh_CN.GB18030"},
	{"BIG5", "zh_TW.BIG5"},
	{"SJIS", "ja_JP.SJIS"},
	{"EUCJP", "ja_JP.EUC-JP"},
	{"EUCKR", "ko_KR.EUC-KR"}
#endif
};

//// 输入字符串必须以\0结尾
extern size_t strconv_wctomb(const char* codeSet, const wchar_t* instr, char* outstr, int outByteLen);
extern size_t strconv_mbtowc(const char* codeSet, const char* instr, wchar_t* outstr, int outByteLen);


typedef struct {
	char* from;
	char* to;
} char_conv_t;

extern char_conv_t* strconv_open(const char* fromCode, const char* toCode);
extern size_t strconv(char_conv_t* convt, const char *instr, char *outstr, int outlen);
extern void strconv_close(char_conv_t* convt);


#ifdef __cplusplus
}
#endif

#endif

