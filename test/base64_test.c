/*
 * base64_test.c
 *
 *  Created on: 2014年9月22日
 *      Author: blc
 */

#include <base64.h>
#include "crypto_test.h"

#include <stdlib.h>

START_TEST(test_base64_encode) {
	const char s1[] = "http://www.google.com:8080/index.jsp";
	int len1 = strlen(s1);
	int buflen = base64_encode_buflen(len1);
	char* dstbuf = malloc(buflen);
	base64_encode((unsigned char*)s1, len1, dstbuf);
	ck_assert_str_eq(dstbuf, "aHR0cDovL3d3dy5nb29nbGUuY29tOjgwODAvaW5kZXguanNw");
	free(dstbuf);
}
END_TEST

START_TEST(test_base64_encode_len) {
	const char s1[] = "http://www.google.com:8080/index.jsp";
	int len1 = strlen(s1);
	int buflen = base64_encode_buflen(len1);
	char* dstbuf = malloc(buflen);
	int rlen = base64_encode((unsigned char*)s1, len1, dstbuf);
	ck_assert_int_eq(rlen, 48);
	free(dstbuf);
}
END_TEST

START_TEST(test_base64_decode) {
	char s1[] = "aHR0cDovL3d3dy5nb29nbGUuY29tOjgwODAvaW5kZXguanNw";
	int len1 = strlen(s1);
	int buflen = base64_decode_buflen(len1);
	char* dstbuf = malloc(buflen);
	base64_decode(s1, len1, (unsigned char*)dstbuf);
	ck_assert_str_eq(dstbuf, "http://www.google.com:8080/index.jsp");
	free(dstbuf);
}
END_TEST

START_TEST(test_base64_decode_len) {
	const char s1[] = "aHR0cDovL3d3dy5nb29nbGUuY29tOjgwODAvaW5kZXguanNw";
	int len1 = strlen(s1);
	int buflen = base64_decode_buflen(len1);
	char* dstbuf = malloc(buflen);
	int rlen = base64_decode(s1, len1, (unsigned char*)dstbuf);
	ck_assert_int_eq(rlen, 36);
	free(dstbuf);
}
END_TEST



TCase* make_base64_tcase(void) {
	TCase* tc = tcase_create("base64");

	tcase_add_test(tc, test_base64_encode);
	tcase_add_test(tc, test_base64_encode_len);
	tcase_add_test(tc, test_base64_decode);
	tcase_add_test(tc, test_base64_decode_len);

	return tc;
}
