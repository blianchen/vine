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


	//main function args
	int nargs = lua_gettop(l) - 1;
	lua_xmove(l, tl, nargs);
	//main function args num
	lua_pushinteger(tl, nargs);
	//thread lua_State ref
	lua_pushvalue(l, 1); /* copy lua_State ref to top */
	lua_xmove(l, tl, 1);

	/**
	 * tl stack
	 * stack[1] = thread main function
	 * stack[2 -> nargs+1] = thread main function args
	 * stack[nargs+1+1] = function args num
	 * stack[nargs+1+2] = thread lua_State ref, use to undef
	 */

	st_thread_t thread = st_thread_create(st_thread_callback_fun, (void*) tl, 0, ST_THREAD_STACK_SIZE);
	if (thread == NULL) {
		LOG_WARN("Create thread error.");
		lua_pushnil(l);
		return 1;
	}

	lua_pushlightuserdata(l, thread);
	return 1;
}

static int get_tid(lua_State *l) {
	st_thread_t thread;
	// if no param or nil is get current thread id
	if (lua_gettop(l) == 0 || lua_type(l, 1) == LUA_TNIL) {
		thread = st_thread_self();
	} else {
		thread = lua_touserdata(l, 1);
	}
	lua_pushnumber(l, st_get_tid(thread));
	return 1;
}

static int stusleep(lua_State* l) {
	int ms = luaL_checkinteger(l, 1);
	st_usleep(ms);
	return 0;
}

static int stustime(lua_State* l) {
	lua_pushnumber(l, st_utime());
	return 1;
}

static int stsend(lua_State *l) {
	st_thread_t thread;
	if (lua_type(l, 1) == LUA_TLIGHTUSERDATA) {
		thread = lua_touserdata(l, 1);
	} else {
		st_tid_t tid = luaL_checknumber(l, 1);
		if (ST_IS_INTERNAL(tid)) {
			thread = st_get_thread(tid);
		} else {	// to thread in another node
			//todo
		}
	}

	size_t len;
	const char *buf = luaL_checklstring(l, 2, &len);

	st_thread_msg_t msg = st_create_msg(buf, len);
	st_send_msg(thread, msg);
	return 0;
}

static int stsend_multi(lua_State *l) {
	luaL_checktype(l, 1, LUA_TTABLE);

	size_t buflen;
	const char *buf = luaL_checklstring(l, 2, &buflen);
	st_thread_msg_t msg = st_create_msg(buf, buflen);

	int len = lua_objlen(l, 1);
	int i;
	st_tid_t tid;
	st_thread_t thread;
	for (i=1; i<=len; i++) {
		lua_rawgeti(l, 1, i);
		tid = luaL_checknumber(l, -1);
		lua_pop(l, 1);

		if (ST_IS_INTERNAL(tid)) {
			thread = st_get_thread(tid);
		} else {	// to thread in another node
			//todo
		}
		st_send_msg(thread, msg);
	}
	return 0;
}

static int strecv(lua_State *l) {
	st_thread_msg_t msg = st_recv_msg();

	char *buf;
	int len = st_get_data(msg, &buf);

	lua_pushnumber(l, st_get_fromtid(msg));
	lua_pushlstring(l, buf, len);

	st_destroy_msg(msg);
	return 2;
}

static const luaL_Reg funs[] = {
		{"spawn", thread_create},
		{"tid", get_tid},
		{"usleep", stusleep},
		{"ustime", stustime},
		{"send", stsend},
		{"recv", strecv},
		{"send_multi", stsend_multi},
		{NULL, NULL}
};

LUA_API int luaopen_stlib(lua_State* l) {
	luaL_register(l, LUA_ST_LIBNAME, funs);
	return 1;
}

