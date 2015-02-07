#include <strconv.h>

#include <string.h>
#include <locale.h>

#include <utils.h>
#include <logger.h>
#include <mem.h>

//#include <setjmp.h>
//#include <error.h>
//#include <errno.h>


size_t strconv_wctomb(const char* codeSet, const wchar_t* instr, char* outstr, int outByteLen) {
	//转大写
	char toCode[MAX_CODESET_NAME_LEN];
	STRUPR(codeSet, toCode);

	//把标准字符集名转为当前环境用的字符集名
	char* aliasName;
	int i;
	for (i = 0; i < sizeof(charconv_codeSets)/sizeof(CodeSetName); i++) {
		if (strcmp(charconv_codeSets[i].aliasName, toCode) == 0) {
			aliasName = charconv_codeSets[i].codeSet;
			break;
		}
	}
	if (!aliasName) return 0;

	setlocale(LC_CTYPE, aliasName);
	return wcstombs(outstr, instr, outByteLen);
}


size_t strconv_mbtowc(const char* codeSet, const char* instr, wchar_t* outstr, int outByteLen) {
	//转大写
	char fromCode[MAX_CODESET_NAME_LEN];
	STRUPR(codeSet, fromCode);

	//把标准字符集名转为当前环境用的字符集名
	char* aliasName;
	int i;
	for (i = 0; i < sizeof(charconv_codeSets)/sizeof(CodeSetName); i++) {
		if (strcmp(charconv_codeSets[i].aliasName, fromCode) == 0) {
			aliasName = charconv_codeSets[i].codeSet;
			break;
		}
	}
	if (!aliasName) return 0;

	setlocale(LC_CTYPE, aliasName);
	return mbstowcs(outstr, instr, outByteLen);
}


////////////////// 模拟iconv方式 //////////////////
char_conv_t* strconv_open(const char* fromCode, const char* toCode) {
	char_conv_t* ct = (char_conv_t*) MALLOC(sizeof(char_conv_t));
//	if (!ct) {
//		LOG_WARN("malloc error!");
//		return NULL;
//	}

	int i;
	char upCode[MAX_CODESET_NAME_LEN];
	STRUPR(fromCode, upCode);
	for (i = 0; i < sizeof(charconv_codeSets)/sizeof(CodeSetName); i++) {
		if (strcmp(charconv_codeSets[i].aliasName, upCode) == 0) {
			ct->from = charconv_codeSets[i].codeSet;
			break;
		}
	}

	STRUPR(toCode, upCode);
	for (i = 0; i < sizeof(charconv_codeSets)/sizeof(CodeSetName); i++) {
		if (strcmp(charconv_codeSets[i].aliasName, upCode) == 0) {
			ct->to = charconv_codeSets[i].codeSet;
			break;
		}
	}

	if (!ct->from || !ct->to || strcmp(ct->from, ct->to) == 0) {
		FREE(ct);
		return NULL;
	}

	return ct;
}

size_t strconv(char_conv_t* convt, const char *instr, char *outstr, int outlen) {
	if (!convt || !instr || !outstr) return 0;
//	if (*instr == NULL) return 0;
	int wlen = sizeof(wchar_t) * strlen(instr);
	wchar_t* wstr = (wchar_t*) MALLOC(wlen+1);
//	if (!wstr) {
//		LOG_WARN("malloc error!");
//		return -1;
//	}
	setlocale(LC_CTYPE, convt->from);
	int wl = mbstowcs(wstr, instr, wlen);
	if (wl <= 0) {
		LOG_DEBUG("转码错误，返回值%d", wl);
		return -1;
	}
	setlocale(LC_CTYPE, convt->to);
	int ri = wcstombs(outstr, wstr, outlen);

	FREE(wstr);
	return ri;
}

void strconv_close(char_conv_t* convt) {
	FREE(convt);
	convt = NULL;
}
