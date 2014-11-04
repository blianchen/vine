/*
 * crypto_test.h
 *
 *  Created on: 2014年9月22日
 *      Author: blc
 */

#ifndef _RSA_TEST_H_
#define _RSA_TEST_H_

#include <check.h>

Suite* make_crypto_suite(void);

TCase* make_base64_tcase(void);
TCase* make_md5_tcase(void);
TCase* make_sha1_tcase(void);
TCase* make_uri_tcase(void);

#endif /* BASE64_TEST_H_ */
