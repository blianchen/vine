/*
 * luast.h
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#ifndef LUAAPI_LUAST_H_
#define LUAAPI_LUAST_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

//void st_context_init();

LUA_API int lua_open_stlib(lua_State* l);

#endif /* LUAAPI_LUAST_H_ */
