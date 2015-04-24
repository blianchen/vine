/*
 ============================================================================
 vine
 Name        : ccore.c
 Author      : blc
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "crypto_test.h"
#include "core_test.h"

#include <exception/exception.h>
#include <mem.h>
#include <str.h>

#include <st/st.h>
#include <stdint.h>

#include <nets.h>

int main(void) {
	puts("!!!!!!!!!!!!!!!!! UNIT TEST !!!!!!!!!!!!!!!!!!!!"); /* prints !!!Hello World!!! */


	exception_init();


	SRunner* sr = srunner_create(make_crypto_suite());
	srunner_run_all(sr, CK_NORMAL);

	SRunner* sr1 = srunner_create(make_core_suite());
	srunner_run_all(sr1, CK_NORMAL);

	srunner_free(sr);
	srunner_free(sr1);

	return EXIT_SUCCESS;
}
