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

#include <exception.h>
#include <MemoryException.h>
#include <setjmp.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <luaapi/luast.h>

#include <st/public.h>
#include <stdint.h>

int addc(lua_State* l) {
	lua_pushnumber(l, 210);
	return 1;
}


int calllua(lua_State* l) {
	printf("in calllua!!!!\n");
//	printstk(l);
	char* funname = luaL_checkstring(l, 1);
//	printf("-----------------------\n");
//	int paramNum = luaL_checkinteger(l, 2);
	lua_getglobal(l, funname);
	lua_replace(l, 1);
//	printstk(l);
	lua_pcall(l, lua_gettop(l)-1, 0, 0);
	return 0;
}

int main(void) {
	puts("!!!!!!!!!!!!!!!!! UNIT TEST !!!!!!!!!!!!!!!!!!!!"); /* prints !!!Hello World!!! */


	exception_init();

	if (st_init() < 0) {
//		THROW(st_exception, "st init error.");
	}

	lua_State* ls = luaL_newstate();
	luaL_openlibs(ls);

	lua_open_stlib(ls);

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
