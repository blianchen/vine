
#ifndef _BASE64_H_
#define _BASE64_H_

#ifdef __cplusplus
extern "C" {
#endif

//// 编码需要的buffer大小， 包括'\0'
int base64_encode_buflen(int byteLen);
//// 解码需要的最大字节数大小
int base64_decode_buflen(int base64Len);

/**
 * 译码
 * @value 	输入的字节数组
 * @vlen 	输入的字节数组长度
 * @result 	结果base64码字符串，以'\0'终止
 * 返回base64码字符串的长度
 */
int base64_encode(const unsigned char* value, int vlen, char* result);

/**
 * 解码
 * @value 	输入的base64码字符串
 * @vlen 	输入的base64码长度
 * @result 	结果字节数组
 * 返回结果字节数组的长度
 */
int base64_decode(const char *value, int vlen, unsigned char* result);

#ifdef __cplusplus
}
#endif

#endif
