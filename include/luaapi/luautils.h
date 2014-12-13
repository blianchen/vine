/*
 * luautils.h
 *
 *  Created on: Dec 6, 2014
 *      Author: blc
 */

#ifndef LUAAPI_LUAUTILS_H_
#define LUAAPI_LUAUTILS_H_

#include <stdarg.h>

#include <lua.h>

void open_lua_libs (lua_State *l);

void load_lua_file(lua_State* l, char* file);

void call_lua_fun(lua_State* l, int nargs, int nresults);

void dump_cstack(lua_State* l);

#endif /* LUAAPI_LUAUTILS_H_ */
