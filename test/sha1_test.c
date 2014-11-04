/*
 * sha1_test.c
 *
 *  Created on: 2014年9月22日
 *      Author: blc
 */

#include "crypto_test.h"

#include <stdlib.h>

#include <sha1.h>
#include <utils.h>


START_TEST(test_sha1_encode) {
	const char* str = "sa第三方sdfs防守对方fsd#$%(*&%$Y^&^^&dfsdf";
	unsigned char digest[20];
	SHA1_CTX* ctx = malloc(sizeof(SHA1_CTX));
	sha1_init(ctx);
	sha1_update(ctx, (unsigned char*)str, strlen(str));
	sha1_final(digest, ctx);

	char hb[41];
	byteToUperHexStr(digest, 20, hb);
	hb[40] = '\0';
	ck_assert_str_eq(hb, "7EA9CF52295D944608502B80FFDDE07FD6F56228");

	free(ctx);
}
END_TEST


TCase* make_sha1_tcase(void) {
	TCase* tc = tcase_create("sha1");

	tcase_add_test(tc, test_sha1_encode);

	return tc;
}
