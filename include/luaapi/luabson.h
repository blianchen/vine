
#ifndef _LUA_BSON_H_
#define _LUA_BSON_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <st/st.h>

#define LUA_BSON_LIBNAME "bson"

LUA_API int luaopen_bson(lua_State* l);

#endif
