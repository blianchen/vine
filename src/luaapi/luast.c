/*
 * luast.c
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#include <stdio.h>
#include <string.h>

#include <mem.h>
#include <exception/st_exception.h>
#include <logger.h>
#include <luaapi/luautils.h>

#include <luaapi/luast.h>


char LUA_THREAD_GLOBAL_IDX;

static int thread_create(lua_State* l) {
	/* save stack index */
	int base = lua_gettop(l);

	/* get global thread table */
	lua_pushlightuserdata(l, &LUA_THREAD_GLOBAL_IDX);
	lua_rawget(l, LUA_GLOBALSINDEX);

	lua_State *tl = lua_newthread(l);

	luaL_argcheck(l, lua_isfunction(l, 1) && !lua_iscfunction(l, 1), 1, "Lua function expected.");
	lua_pushvalue(l, 1); /* move function to top */
	lua_xmove(l, tl, 1); /* move function from L to NL */

	/* get ref */
	int ref = luaL_ref(l, -2);
	if (ref == LUA_NOREF) {
		lua_settop(l, base); /* restore main thread stack */
		lua_pushnil(l);
		return 1;
	}

	lua_settop(l, base); /* restore main thread stack */

	lua_pushinteger(l, ref);
	return 1;
}

static void* st_thread_callback_fun(void* arg) {
	lua_State* tl = (lua_State*) arg;
	int ref = lua_tointeger(tl, -1);
	int nargs = lua_tointeger(tl, -2);
	lua_pop(tl, 2);

	call_lua_fun(tl, nargs, 0);

	/* detach coroutine from GLOBAL table */
	lua_pushlightuserdata(tl, &LUA_THREAD_GLOBAL_IDX);
	lua_rawget(tl, LUA_GLOBALSINDEX);
	luaL_unref(tl, -1, ref);

	return NULL;
}

/**
 *
 */
static int thread_run(lua_State* L) {
	lua_pushlightuserdata(L, &LUA_THREAD_GLOBAL_IDX);
	lua_rawget(L, LUA_GLOBALSINDEX);
	lua_pushvalue(L, 1); /* copy lua_State ref to top */
	lua_gettable(L, -2);   // get the thread lua_state from table and push to stack top

	lua_State* tl = lua_tothread(L, -1);
	luaL_argcheck(L, tl, 1, "thread expected");

	lua_pop(L, 1);

	//main function args
	int nargs = lua_gettop(L) - 1;
	lua_xmove(L, tl, nargs);
	//main function args num
	lua_pushinteger(tl, nargs);
	//thread lua_State ref
	lua_pushvalue(L, 1); /* copy lua_State ref to top */
	lua_xmove(L, tl, 1);

	/**
	 * tl stack
	 * stack[1] = thread main function
	 * stack[2 -> nargs+1] = thread main function args
	 * stack[nargs+1+1] = function args num
	 * stack[nargs+1+2] = thread lua_State ref, use to undef
	 */

	if (st_thread_create(st_thread_callback_fun, (void*) tl, 0, ST_THREAD_STACK_SIZE) == NULL) {
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

LUA_API int luaopen_stlib(lua_State* l) {
	luaL_register(l, LUA_ST_LIBNAME, funs);
	return 1;
}

