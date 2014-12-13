/*
 * luautils.c
 *
 *  Created on: Dec 6, 2014
 *      Author: blc
 */

#include <stdio.h>

#include <logger.h>
#include <mem.h>

#include <luaapi/luautils.h>
#include <lauxlib.h>
#include <lualib.h>

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


void open_lua_libs (lua_State *l) {
  const luaL_Reg *lib = uselualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(l, lib->func);
    lua_pushstring(l, lib->name);
    lua_call(l, 1, 0);
  }
}

void load_lua_file(lua_State* l, char* file) {
	if (luaL_loadfile(l, file)) {
		size_t len;
		const char* str = luaL_checklstring(l, -1, &len);
		log_warn(str);
	}
}

void call_lua_fun(lua_State* l, int nargs, int nresults) {
	if (lua_pcall(l, nargs, nresults, 0)) {
		size_t len;
		const char* str = luaL_checklstring(l, -1, &len);
		log_warn(str);
	}
}

void dump_cstack(lua_State* l) {
	int n = lua_gettop(l);
	int i, t;
	for (i=1; i<=n; i++) {
		t = lua_type(l, i);
		switch (t) {
		case LUA_TNUMBER:
			printf("Call cstack index %i is number %g", i, lua_tonumber(l, i));
			break;
		case LUA_TSTRING:
			printf("Call cstack index %i is string %s", i, lua_tostring(l, i));
			break;
		case LUA_TBOOLEAN:
			printf("Call cstack index %i is boolean %s", i, lua_toboolean(l, i)?"true":"false");
			break;
		default:
			printf("Call cstack index %i type is %s", i, lua_typename(l, t));
			break;
		}
		printf("\n");
	}
}

