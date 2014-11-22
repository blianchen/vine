
#ifndef _LUA_DB_H_
#define _LUA_DB_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <exception/NetException.h>
#include <st/public.h>

#include <hiredis/hiredis.h>


#define LUA_DB_LIB_NAME "db"

LUA_API int luaopen_dblib(lua_State* l);

#endif
