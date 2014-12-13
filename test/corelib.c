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

#include <luaapi/luautils.h>
#include <luaapi/luast.h>
#include <luaapi/luadb.h>
#include <luaapi/luanet.h>

#include <st/st.h>
#include <stdint.h>


void* startLuaMain(void* arg) {
	lua_State* ls = arg;

	load_lua_file(ls, "t.lua");		// lua file
	call_lua_fun(ls, 0, 0);
	lua_getglobal(ls, "luafun");	// lua function
	call_lua_fun(ls, 0, 0);
	return NULL;
}


int main(void) {
	puts("!!!!!!!!!!!!!!!!! UNIT TEST !!!!!!!!!!!!!!!!!!!!"); /* prints !!!Hello World!!! */


	exception_init();

	lua_State* ls = luaL_newstate();
	open_lua_libs(ls);

	luaopen_stlib(ls);
	luaopen_netlib(ls);
	luaopen_dblib(ls);

	/*
	 * register a table to anchor lua coroutines reliably:
	 * {([int]ref) = [cort]}
	 */
	lua_pushlightuserdata(ls, &LUA_THREAD_GLOBAL_IDX);
	lua_newtable(ls);
	lua_rawset(ls, LUA_GLOBALSINDEX);

	//// st
	st_set_eventsys(ST_EVENTSYS_ALT);

	if (st_init() < 0) {
//		THROW(st_exception, "st init error.");
	}

	//// lua main fun
	st_thread_create(startLuaMain, ls, 0);

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
