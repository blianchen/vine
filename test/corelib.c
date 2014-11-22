/*
 ============================================================================
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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <luaapi/luast.h>
#include <luaapi/luadb.h>

#include <st/public.h>
#include <stdint.h>

int main(void) {
	puts("!!!!!!!!!!!!!!!!! UNIT TEST !!!!!!!!!!!!!!!!!!!!"); /* prints !!!Hello World!!! */


	exception_init();

	if (st_init() < 0) {
//		THROW(st_exception, "st init error.");
	}

	lua_State* ls = luaL_newstate();
	luaL_openlibs(ls);

	luaopen_stlib(ls);
	luaopen_netlib(ls);
	luaopen_dblib(ls);

	luaL_loadfile(ls, "t.lua");
	lua_pcall(ls, 0, 0, 0);
	lua_getglobal(ls, "luafun");
	lua_pcall(ls, 0, 0, 0);

	st_thread_exit(NULL);

	lua_close(ls);

	return 0;


	SRunner* sr = srunner_create(make_crypto_suite());
	srunner_run_all(sr, CK_NORMAL);

	SRunner* sr1 = srunner_create(make_core_suite());
	srunner_run_all(sr1, CK_NORMAL);

	srunner_free(sr);
	srunner_free(sr1);

	return EXIT_SUCCESS;
}
