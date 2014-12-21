
#ifndef _LUA_NET_H_
#define _LUA_NET_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


#define LUA_NET_LIB_NAME "net"

LUA_API int luaopen_netlib(lua_State* l);

#endif
