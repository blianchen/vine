
#ifndef _LUA_NET_H_
#define _LUA_NET_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <exception/NetException.h>
#include <st/public.h>

#define LUA_NET_LIB_NAME "net"

LUA_API int luaopen_netlib(lua_State* l);

#endif
