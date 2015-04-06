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

static int stThreadCreate(lua_State* l) {
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
	 * stack[nargs+1+2] = thread lua_State ref, use to unref
	 */

	st_thread_t thread = st_thread_create(st_thread_callback_fun, (void*) tl, 0, ST_THREAD_STACK_SIZE);
	if (thread == NULL) {
		LOG_WARN("Create thread error.");
		lua_pushnil(l);
		return 1;
	}

//	lua_pushlightuserdata(l, thread);
	lua_pushnumber(l, st_get_tid(thread));	//return tid
	return 1;
}

//get current thread id
static int stGetTid(lua_State *l) {
	st_thread_t thread = st_thread_self();
	lua_pushnumber(l, st_get_tid(thread));
	return 1;
}

static int stUsleep(lua_State* l) {
	int ms = luaL_checkinteger(l, 1);
	st_usleep(ms);
	return 0;
}

static int stUstime(lua_State* l) {
	lua_pushnumber(l, st_utime());
	return 1;
}

static int stSend(lua_State *l) {
	if (lua_type(l, 1) == LUA_TNUMBER) {
		st_tid_t tid = luaL_checknumber(l, 1);
		size_t len;
		const char *buf = luaL_checklstring(l, 2, &len);
		st_thread_msg_t msg = st_create_msg(buf, len);
		if (ST_IS_INTERNAL(tid)) {
			st_thread_t thread = st_get_thread(tid);
			if (thread)
				st_send_msg(thread, msg);
			else
				LOG_WARN("thread no found, id=%lu", tid);
		} else {	// to thread in another node
			st_send_msg_by_tid(tid, msg);
		}
	}
	else if (lua_type(l, 1) == LUA_TSTRING) {
		char *nodeUrl = luaL_checkstring(l, 1);
		char *threadName = luaL_checkstring(l, 2);
		size_t len;
		const char *buf = luaL_checklstring(l, 3, &len);
		st_thread_msg_t msg = st_create_msg(buf, len);
		st_send_msg_by_name(nodeUrl, threadName, msg);
	} else {
		LOG_WARN("parameter error.");
	}
	return 0;
}

static int stSendMulti(lua_State *l) {
	luaL_checktype(l, 1, LUA_TTABLE);

	size_t buflen;
	const char *buf = luaL_checklstring(l, 2, &buflen);
	st_thread_msg_t msg = st_create_msg(buf, buflen);

	int len = lua_objlen(l, 1);
	st_tid_t *nodeTids = CALLOC(len, sizeof(st_tid_t));
	int i;
	st_tid_t tid;
	st_thread_t thread;
	for (i=1; i<=len; i++) {
		lua_rawgeti(l, 1, i);
		tid = luaL_checknumber(l, -1);
		lua_pop(l, 1);

		// first send internal thread.
		if (ST_IS_INTERNAL(tid)) {
			thread = st_get_thread(tid);
			if (thread)
				st_send_msg(thread, msg);
			else
				LOG_WARN("thread no found, id=%lu", tid);
			return 0;
		} else {
			nodeTids[i] = tid;
		}
	}
	// send msg in nodes
	for (i=1; i<=len; i++) {
		if (nodeTids[i]) {
			st_send_msg_by_tid(nodeTids[i], msg);
		}
	}
	return 0;
}

static int stRecv(lua_State *l) {
	st_thread_msg_t msg = st_recv_msg();

	char *buf;
	int len = st_msg_data(msg, &buf);

	lua_pushnumber(l, st_msg_fromtid(msg));
	lua_pushlstring(l, buf, len);

	st_destroy_msg(msg);
	return 2;
}

static int stHasMsg(lua_State *l) {
	st_tid_t tid = luaL_checknumber(l, 1);
	st_thread_t thread = st_get_thread(tid);
	if (thread) {
		lua_pushboolean(l, st_has_msg(thread));
	} else {
		lua_pushboolean(l, 0);
	}
	return 1;
}

static int stIsInternalTid(lua_State *l) {
	st_tid_t tid = luaL_checknumber(l, 1);
	lua_pushboolean(l, ST_IS_INTERNAL(tid));
	return 1;
}

static int stRegTid(lua_State *l) {
	char *name = luaL_checkstring(l, 1);
	st_tid_t tid = luaL_checknumber(l, 2);
	if (st_reg_tid(name, tid)) {
		lua_pushboolean(l, 0);
	} else {
		lua_pushboolean(l, 1);
	}
	return 1;
}
static int stUnregTid(lua_State *l) {
	char *name = luaL_checkstring(l, 1);
	st_unreg_tid(name);
	return 0;
}
static int stGetRegNames(lua_State *l) {
	lua_newtable(l);
	char **names = st_get_reg_names();
	int i = 0;
	while (names[i]) {
		lua_pushinteger(l, i+1);
		lua_pushstring(l, names[i]);
		lua_rawset(l, -3);
		i++;
	}
	return 1;	// a table
}
static int stGetRegTid(lua_State *l) {
	char *name = luaL_checkstring(l, 1);
	st_tid_t tid = st_get_reg_tid(name);
	lua_pushnumber(l, tid);
	return 1;
}

static const luaL_Reg funs[] = {
		{"spawn", stThreadCreate},
		{"tid", stGetTid},
		{"usleep", stUsleep},
		{"ustime", stUstime},
		{"send", stSend},
		{"recv", stRecv},
		{"send_multi", stSendMulti},
		{"has_msg", stHasMsg},
		{"is_internal_tid", stIsInternalTid},
		{"register", stRegTid},
		{"unregister", stUnregTid},
		{"registeredNames", stGetRegNames},
		{"getRegisterTid", stGetRegTid},
		{NULL, NULL}
};

LUA_API int luaopen_stlib(lua_State* l) {
	luaL_register(l, LUA_ST_LIBNAME, funs);
	return 1;
}

