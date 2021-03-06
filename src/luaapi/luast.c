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


static st_tid_t getTidFromLua(lua_State* l, int index) {
//	lua_rawgeti(l, index, 1);
//	uint64_t nodeid = luaL_checkinteger(l, -1);
//	lua_pop(l, 1);
//	lua_rawgeti(l, index, 2);
//	int sid = luaL_checkinteger(l, -1);
//	lua_pop(l, 1);
//	return ST_MAKE_TID_NID(sid, nodeid);
	return luaL_checkinteger(l, index);
}

static void pushTidToLua(lua_State* l, st_tid_t tid) {
//	lua_newtable(l);
//	lua_pushinteger(l, 1);
//	lua_pushinteger(l, ST_NODEID(tid));
//	lua_rawset(l, -3);
//	lua_pushinteger(l, 2);
//	lua_pushinteger(l, ST_SID(tid));
//	lua_rawset(l, -3);
	lua_pushinteger(l, tid);
}

static void* st_thread_callback_fun(void* arg) {
	lua_State* tl = (lua_State*) arg;
	int ref = lua_tointeger(tl, -1);
	int nargs = lua_tointeger(tl, -2);
	lua_pop(tl, 2);

	call_lua_fun(tl, nargs, 0);

	/* detach coroutine from GLOBAL table */
//	lua_pushlightuserdata(tl, &LUA_THREAD_GLOBAL_IDX);
//	lua_rawget(tl, LUA_GLOBALSINDEX);
	lua_getglobal(tl, LUA_THREAD_GLOBAL_TABLE);
	luaL_unref(tl, -1, ref);

	return NULL;
}

/**
 * spawn(function/functionName, ...)
 */
static int stThreadCreate(lua_State* L) {
	/* save stack index */
	int base = lua_gettop(L);

	/* get global thread table */
	lua_getglobal(L, LUA_THREAD_GLOBAL_TABLE);

	lua_State *tl = lua_newthread(L);

	if (lua_type(L, 1) == LUA_TSTRING) {	// function name
		size_t sn = 0;
		const char *pf = luaL_checklstring(L, 1, &sn);
		sn++;
		char *full = MALLOC(sn);
		memcpy(full, pf, sn);
		char *funName = strtok(full, ".");
		lua_getglobal(L, funName);
		do {
			funName = strtok(NULL, ".");
			if (funName) {
				lua_pushstring(L, funName);
				lua_rawget(L, -2);
				lua_remove(L, -2);
			}
		} while (funName);
		FREE(full);
	} else if (lua_isfunction(L, 1) && !lua_iscfunction(L, 1)) {
		lua_pushvalue(L, 1); /* move function to top */
	} else {
		LOG_WARN("first arg type(%s) error, must be string or lua function.", lua_typename(L, lua_type(L, 1)));
		lua_pushnil(L);
		return 1;
	}
	lua_xmove(L, tl, 1); /* pop and move function from L to NL */

	/* get ref */
	int ref = luaL_ref(L, -2);
	if (ref == LUA_NOREF) {
		lua_settop(L, base); /* restore main thread stack */
		lua_pushnil(L);
		return 1;
	}

	lua_settop(L, base); /* restore main thread stack */


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
	 * stack[nargs+1+2] = thread lua_State ref, use to unref
	 */

	st_thread_t thread = st_thread_create(st_thread_callback_fun, (void*) tl, 0, ST_THREAD_STACK_SIZE);
	if (thread == NULL) {
		LOG_WARN("Create thread error.");
		lua_pushnil(L);
		return 1;
	}

	pushTidToLua(L, st_get_tid(thread));
//	printf("++++++++++++++++++++++ create: {%lu,%d} \n", st_get_this_node_id(), st_get_sid(thread));
	return 1;
}

//get current thread id
static int stGetTid(lua_State *l) {
	st_thread_t thread = st_thread_self();
//	lua_pushnumber(l, st_get_tid(thread));
	pushTidToLua(l, st_get_tid(thread));
	return 1;
}

static int stUsleep(lua_State* l) {
	int ms = luaL_checkinteger(l, 1);
	st_usleep(ms);
	return 0;
}

static int stUstime(lua_State* L) {
	lua_pushinteger(L, st_utime());
	return 1;
}

static int stSend(lua_State *L) {	//send(toTid, msg [,fromTid]), send(node, threadName, msg [,fromTid])
	if (lua_isinteger(L, 1)) {
		st_tid_t tid = getTidFromLua(L, 1); //luaL_checkinteger(l, 1);
		size_t len;
		const char *buf = luaL_checklstring(L, 2, &len);
		st_tid_t fromTid;
		if (lua_gettop(L) > 2) {
			fromTid = getTidFromLua(L, 3);
		} else {
			fromTid = st_get_tid(st_thread_self());
		}
		st_thread_msg_t msg = st_create_msg(buf, len, fromTid);
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
	else if (lua_isstring(L, 1)) {
		char *nodeUrl = luaL_checkstring(L, 1);
		char *threadName = luaL_checkstring(L, 2);
		size_t len;
		const char *buf = luaL_checklstring(L, 3, &len);
		st_tid_t fromTid = 0;
		if (lua_gettop(L) > 3) {
			fromTid = getTidFromLua(L, 4);
		} else {
			fromTid = st_get_tid(st_thread_self());
		}
		st_thread_msg_t msg = st_create_msg(buf, len, fromTid);
		st_send_msg_by_name(nodeUrl, threadName, msg);
	} else {
		LOG_WARN("parameter error.");
	}
	return 0;
}

static int stSendMulti(lua_State *L) { // send_multi(tidsTable, msg [,fromId])
	luaL_checktype(L, 1, LUA_TTABLE);

	size_t buflen;
	const char *buf = luaL_checklstring(L, 2, &buflen);
	st_tid_t fromTid = 0;
	if (lua_gettop(L) > 2) {
		fromTid = getTidFromLua(L, 3);
	} else {
		fromTid = st_get_tid(st_thread_self());
	}
	st_thread_msg_t msg = st_create_msg(buf, buflen, fromTid);

	int len = lua_rawlen(L, 1);
	st_tid_t *nodeTids = CALLOC(len, sizeof(st_tid_t));
	int i;
	st_tid_t tid;
	st_thread_t thread;
	for (i=1; i<=len; i++) {
		lua_rawgeti(L, 1, i);
		tid = getTidFromLua(L, -1);
		lua_pop(L, 1);

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

static int stRecv(lua_State *L) {
	st_thread_msg_t msg = st_recv_msg();

	char *buf;
	int len = st_msg_data(msg, &buf);

	pushTidToLua(L, st_msg_fromtid(msg));
	lua_pushlstring(L, buf, len);

	st_destroy_msg(msg);
	return 2;
}

static int stTostringTid(lua_State *L) {
	char buf[128];
	uint64_t tid = getTidFromLua(L, 1);
	int n = sprintf(buf, "{%lu, %lu}", ST_NODEID(tid), ST_SID(tid));
	lua_pushlstring(L, buf, n);
	return 1;
}

static int stHasMsg(lua_State *l) {
	st_tid_t tid = getTidFromLua(l, 1);
	st_thread_t thread = st_get_thread(tid);
	if (thread) {
		lua_pushboolean(l, st_has_msg(thread));
	} else {
		lua_pushboolean(l, 0);
	}
	return 1;
}

static int stIsInternalTid(lua_State *l) {
	st_tid_t tid = getTidFromLua(l, 1);
	lua_pushboolean(l, ST_IS_INTERNAL(tid));
	return 1;
}

static int stRegTid(lua_State *l) {
	char *name = luaL_checkstring(l, 1);
	st_tid_t tid = getTidFromLua(l, 2);
//	printf("++++++++++++++++++++++ stRegTid: %s={%lu,%d} \n", name, ST_NODEID(tid), ST_SID(tid));
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
	pushTidToLua(l, tid);
	return 1;
}

static const luaL_Reg funs[] = {
		{"spawn", stThreadCreate},
		{"tid", stGetTid},
		{"usleep", stUsleep},
		{"ustime", stUstime},
		{"send", stSend},
		{"recv", stRecv},
		{"tostring", stTostringTid},
		{"send_multi", stSendMulti},
		{"has_msg", stHasMsg},
		{"is_internal_tid", stIsInternalTid},
		{"register", stRegTid},
		{"unregister", stUnregTid},
		{"registeredNames", stGetRegNames},
		{"getRegisterTid", stGetRegTid},
		{NULL, NULL}
};

LUA_API int luaopen_st(lua_State* L) {
	luaL_newlib(L, funs);

	//register a global table to save lua thread.
	lua_newtable(L);
	lua_setglobal(L, LUA_THREAD_GLOBAL_TABLE);

	return 1;
}

