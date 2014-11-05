/*
 * exception_test.c
 *
 *  Created on: 2014年10月10日
 *      Author: blc
 */

#include "core_test.h"

#include <exception/MemoryException.h>

////// 移植的，原来有完整的测试

void thr() {
	THROW(MemoryException, "A exception");
}

void indirect() {
	thr();
}

START_TEST(test_exception_trycatch) {
	int e;
	TRY {
		thr();
		e = 1;
	} CATCH(MemoryException) {
		e = 2;
	}
	END_TRY
	ck_assert_int_eq(e, 2);
}
END_TEST

START_TEST(test_exception_indirect_throw) {
	int e;
	TRY {
		indirect();
		e = 1;
	} CATCH(MemoryException) {
		e = 2;
	}
	END_TRY
	ck_assert_int_eq(e, 2);
}
END_TEST


TCase* make_exception_tcase(void) {
	TCase* t = tcase_create("exception");
	tcase_add_test(t, test_exception_trycatch);
	tcase_add_test(t, test_exception_indirect_throw);
	return t;
}
