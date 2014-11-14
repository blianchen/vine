/*
 * luast.c
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#include <stdio.h>
#include <string.h>

#include <mem.h>
#include <exception/STException.h>
#include <st/public.h>

#include <luaapi/luast.h>


static int thread_create(lua_State* L) {
	lua_State *NL = lua_newthread(L);
	luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1), 1, "Lua function expected");
	lua_pushvalue(L, 1); /* move function to top */
	lua_xmove(L, NL, 1); /* move function from L to NL */
	return 1;
}

static void* st_thread_callback_fun(void* arg) {
	lua_State* tl = (lua_State*) arg;
	int nargs = lua_tointeger(tl, -1);
	lua_pop(tl, 1);
	int r = lua_pcall(tl, nargs, 0, 0);
	return NULL;
}

static int thread_run(lua_State* L) {
	lua_State* tl = lua_tothread(L, 1);
	luaL_argcheck(L, tl, 1, "thread expected");
	int nargs = lua_gettop(L) - 1;
	lua_xmove(L, tl, nargs);
	lua_pushinteger(tl, nargs);

	if (st_thread_create(st_thread_callback_fun, (void*) tl, 0, 0) == NULL) {
		lua_pushinteger(L, -1);
	} else {
		lua_pushinteger(L, 0);
	}
	return 1;
}

static int msleep(lua_State* l) {
	int ms = luaL_checkinteger(l, 1);
	st_usleep(ms * 1000);
	return 0;
}

static int mstime(lua_State* l) {
	lua_pushnumber(l, st_utime());
	return 1;
}

static const luaL_Reg funs[] = {
		{"create_thread", thread_create},
		{"run_thread", thread_run},
		{"msleep", msleep},
		{"mstime", mstime},
		{NULL, NULL}
};

LUA_API int lua_open_stlib(lua_State* l) {
	luaL_register(l, LUA_ST_LIB_NAME, funs);
	return 1;
}


////////////////////////////////////////////////////////////
////  private  function
////////////////////////////////////////////////////////////
void dump_cstack(lua_State* l) {
	int n = lua_gettop(l);
	int i;
	for (i=1; i<=n; i++) {
		int t = lua_type(l, i);
		switch (t) {
		case LUA_TNUMBER:
			printf("Call cstack index %i is number %g", i, lua_tonumber(l, i));
			break;
		case LUA_TSTRING:
			printf("Call cstack index %i is string %s", i, lua_tostring(l, i));
			break;
		case LUA_TBOOLEAN:
			printf("Call cstack index %i is boolean %s", i, lua_toboolean(l, i)?"true":"false");
			break;
		default:
			printf("Call cstack index %i type is %s", i, lua_typename(l, t));
			break;
		}
		printf("\n");
	}
}

