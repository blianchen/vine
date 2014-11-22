/*
 * luadb.c
 *
 *  Created on: Nov 21, 2014
 *      Author: blc
 */

#include <luaapi/luadb.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>

static int db_connect(lua_State* l) {
	redisContext* redc = redisConnectNonBlock("127.0.0.1", 6379);

	st_netfd_t nfd = st_netfd_open_socket(redc->fd);

	/* Wait until the socket becomes readable */
	if (st_netfd_poll(nfd, POLLOUT, -1) < 0)
		return 0;

	return 0;
}


static const luaL_Reg funs[] = {
		{"connect", db_connect},
		{NULL, NULL}
};

LUA_API int luaopen_dblib(lua_State* l) {
	luaL_register(l, LUA_DB_LIB_NAME, funs);
	return 1;
}
