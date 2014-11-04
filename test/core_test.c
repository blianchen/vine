/*
 * core_test.c
 *
 *  Created on: 2014年9月26日
 *      Author: blc
 */

#include "core_test.h"

Suite* make_core_suite(void) {
	Suite* s = suite_create("core");
	suite_add_tcase(s, make_hashmap_tcase());
	suite_add_tcase(s, make_exception_tcase());
	return s;
}
