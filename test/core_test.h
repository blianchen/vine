/*
 * core_test.h
 *
 *  Created on: 2014年9月26日
 *      Author: blc
 */

#ifndef _CORE_TEST_H_
#define _CORE_TEST_H_

#include <check.h>

Suite* make_core_suite(void);

TCase* make_hashmap_tcase(void);
TCase* make_exception_tcase(void);

#endif /* CORE_TEST_H_ */
