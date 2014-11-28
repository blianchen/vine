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
#include <luaapi/luanet.h>

#include <st/st.h>
#include <stdint.h>


static const luaL_Reg uselualibs[] = {
  {"", luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_TABLIBNAME, luaopen_table},
//  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};


LUALIB_API void use_lua_openlibs (lua_State *L) {
  const luaL_Reg *lib = uselualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}


int main(void) {
	puts("!!!!!!!!!!!!!!!!! UNIT TEST !!!!!!!!!!!!!!!!!!!!"); /* prints !!!Hello World!!! */


	exception_init();


	lua_State* ls = luaL_newstate();
//	luaL_openlibs(ls);
	use_lua_openlibs(ls);

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


	if (st_init() < 0) {
//		THROW(st_exception, "st init error.");
	}

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
