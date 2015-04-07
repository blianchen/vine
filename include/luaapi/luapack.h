
#ifndef _LPACK_H_
#define _LPACK_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define LUA_PACK_LIBNAME "pack"

int luaopen_pack(lua_State *L);

#endif
