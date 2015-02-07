/*
 * luanet.c
 *
 *  Created on: Nov 18, 2014
 *      Author: blc
 */

#include <luaapi/luanet.h>

#include <stdlib.h>
#include <string.h>

#include <logger.h>
#include <nets.h>

static int lnets_listen(lua_State* l) {
	int port = luaL_checkinteger(l, 1);	//param 1: port
	const char* host = NULL;
	if (lua_gettop(l) > 1 && lua_type(l, 2) == LUA_TSTRING) {
		host = luaL_checkstring(l, 2);	//param 2: ip or domain name
	}
	//param 3:  options : reuseaddr=true,keepalive=true, package=1/2/4, timeout=3000(ms)
	const char* options = NULL;
	if (lua_gettop(l) > 2 && lua_type(l, 3) == LUA_TSTRING) {
		options = luaL_checkstring(l, 3);
	}

	socket_t sock = nets_listen(port, host, options);

	lua_pushlightuserdata(l, sock);		// return a server_socket
	return 1;
}


static int lnets_accept(lua_State* l) {
	socket_t srvsock = lua_touserdata(l, 1);		//a server_socket

	socket_t cltsock = nets_accept(srvsock);

	lua_pushlightuserdata(l, cltsock);		// client socket
	return 1;
}

static int lnets_connect(lua_State* l) {
	const char* addr = luaL_checkstring(l, 1);
	const char* options = NULL;
	if (lua_gettop(l) > 1 && lua_type(l, 2) == LUA_TSTRING) {
		options = luaL_checkstring(l, 2);
	}

	socket_t sock = nets_connect(addr, options);

	lua_pushlightuserdata(l, sock);
	return 1;
}

static int lnets_read(lua_State* l) {
	socket_t sock = lua_touserdata(l, 1);

	int rn = nets_read(sock);

	lua_pushinteger(l, rn);
	if (rn == 0) {
		// connect close
		LOG_DEBUG("connect closed.");
		lua_pushnil(l);
	} else if (rn < 0) {
		lua_pushnil(l);
	} else
		lua_pushlstring(l, sock->buf, rn);
	return 2;
}

static int lnets_write(lua_State* l) {
	socket_t sock = lua_touserdata(l, 1);
	size_t size;
	char* buf = (char*)luaL_checklstring(l, 2, &size);

	int rn = nets_write(sock, buf, size);

	lua_pushinteger(l, rn);
	return 1;
}

static int lnets_close(lua_State* l) {
	socket_t sock = lua_touserdata(l, 1);
	nets_close(sock);
	return 0;
}

static int lnets_getraddr(lua_State* l) {
	socket_t sock = lua_touserdata(l, 1);
	lua_pushstring(l, sock->ip);
	lua_pushinteger(l, sock->port);
	return 2;
}

static const luaL_Reg funs[] = {
		{"listen", lnets_listen},
		{"accept", lnets_accept},
		{"connect", lnets_connect},
		{"read", lnets_read},
		{"write", lnets_write},
		{"close", lnets_close},
		{"getraddr", lnets_getraddr},
		{NULL, NULL}
};

LUA_API int luaopen_netlib(lua_State* l) {
	luaL_register(l, LUA_NET_LIBNAME, funs);
	return 1;
}
