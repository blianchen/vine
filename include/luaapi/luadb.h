
#ifndef _LUA_DB_H_
#define _LUA_DB_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <st/st.h>

#define LUA_DB_LIBNAME "db"

LUA_API int luaopen_db(lua_State* l);

#endif
