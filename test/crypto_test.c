/*
 * crypto_test.c
 *
 *  Created on: 2014年9月22日
 *      Author: blc
 */

#include "crypto_test.h"

Suite* make_crypto_suite(void) {
	Suite* s = suite_create("crypto");
	suite_add_tcase(s, make_base64_tcase());
	suite_add_tcase(s, make_md5_tcase());
	suite_add_tcase(s, make_sha1_tcase());
	suite_add_tcase(s, make_uri_tcase());
	return s;
}
