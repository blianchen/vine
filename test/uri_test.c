/*
 * uri_test.c
 *
 *  Created on: 2014年9月22日
 *      Author: blc
 */

#include "crypto_test.h"

#include <stdlib.h>

#include <uri.h>
#include <utils.h>


START_TEST(test_uri_encode) {
	const char s1[] = "http://www.example.com/application.jsp?user=<user name='some user'></user>";
	int len1 = strlen(s1) ;
	char s2[100] ;
	memset(s2, 0, sizeof(s2)) ;
	int len2 = uri_encode(s1, len1, s2);
	ck_assert_str_eq(s2, "http://www.example.com/application.jsp?user=%3Cuser%20name='some%20user'%3E%3C/user%3E");
	ck_assert_int_eq(len2, 86);
}
END_TEST

START_TEST(test_uri_decode) {
	const char s1[] = "http://www.example.com/application.jsp?user=%3Cuser%20name='some%20user'%3E%3C/user%3E";
	int len1 = strlen(s1) ;
	char s2[100] ;
	memset(s2, 0, sizeof(s2)) ;
	int len2 = uri_decode(s1, len1, s2);
	ck_assert_str_eq(s2, "http://www.example.com/application.jsp?user=<user name='some user'></user>");
	ck_assert_int_eq(len2, 74);
}
END_TEST


TCase* make_uri_tcase(void) {
	TCase* tc = tcase_create("uri");

	tcase_add_test(tc, test_uri_encode);
	tcase_add_test(tc, test_uri_decode);

	return tc;
}
