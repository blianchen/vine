/*
 * md5_test.c
 *
 *  Created on: 2014年9月22日
 *      Author: blc
 */

#include "crypto_test.h"

#include <stdlib.h>

#include <md5.h>
#include <utils.h>


START_TEST(test_md5_encode) {
	const char* str = "sa第三方sdfs防守对方fsd#$%(*&%$Y^&^^&dfsdf";
	unsigned char digest[16];
	MD5_CTX* ctx = malloc(sizeof(MD5_CTX));
	md5_init(ctx);
	md5_update(ctx, (unsigned char*)str, strlen(str));
	md5_final(digest, ctx);

	char hb[33];
	byteToUperHexStr(digest, 16, hb);
	hb[32] = '\0';
	ck_assert_str_eq(hb, "FB92749F431E433DE80B8C6A789986EB");

	free(ctx);
}
END_TEST


TCase* make_md5_tcase(void) {
	TCase* tc = tcase_create("md5");

	tcase_add_test(tc, test_md5_encode);

	return tc;
}
