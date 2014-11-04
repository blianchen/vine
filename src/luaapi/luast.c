/*
 * luast.c
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#include <mem.h>
#include <exception.h>
#include <st/public.h>

//#include <stdint.h>

#include <luaapi/luast.h>

#define LUA_ST_LIB_NAME "st"

void dumpstk(lua_State* l) {
	int n = lua_gettop(l);
	int i;
	for (i=1; i<=n; i++) {
		int t = lua_type(l, i);
		switch (t) {
		case LUA_TNUMBER:
			printf("stack index %i is number %g", i, lua_tonumber(l, i));
			break;
		case LUA_TSTRING:
			printf("stack index %i is string %s", i, lua_tostring(l, i));
			break;
		case LUA_TBOOLEAN:
			printf("stack index %i is boolean %s", i, lua_toboolean(l, i)?"true":"false");
			break;
		default:
			printf("stack index %i type is %s", i, lua_typename(l, t));
			break;
		}
		printf("\n");
	}
}

//void st_context_init() {
//	if (st_init() < 0) {
////		THROW(st_exception, "st init error.");
//	}
//}

int stop_thread(lua_State* l) {
	st_thread_exit(NULL);
	return 0;
}

int msleep(lua_State* l) {
	int ms = luaL_checkinteger(l, 1);
//	printf("in msleep11111111111111!!!! %d\n", ms);
	st_usleep(ms * 1000);
//	printf("in msleep22222222222222!!!! %d\n", ms);
	return 0;
}

typedef struct {
	int type;
	int size;
	char* value;
} CallArg;
typedef struct {
	lua_State* l;
	int argsNum;
	CallArg* args;
} CallStackArgs;

static char* cstStr = "st.call_lua_start_fun()";
void* st_main_call_p(void* l) {
	luaL_dostring((lua_State*)l, cstStr);
	return NULL;
}

int call_lua_start_fun(lua_State* l) {
	lua_getglobal(l, "do_calc");
//	lua_pushcfunction(l, lcf);
	lua_pushnumber(l, 20);
	printf("in call_lua_main!!! lua_stack_size=%d\n", lua_gettop(l));
	dumpstk(l);
	lua_pcall(l, lua_gettop(l)-1, 0, 0);
	return 0;
}

int create_thread(lua_State* l) {
	dumpstk(l);

	int stkn = lua_gettop(l);

	CallStackArgs* callArgs = MALLOC(1024 * 2);	// init 2k
	callArgs->l = l;
	callArgs->argsNum = stkn;
	char* sp = ;

	CallArg* p = callArgs.args;
	int i;
	int type, v;
	for (i=0; i<stkn; i++) {
		type = lua_type(l, i);
		switch (type) {
		case LUA_TBOOLEAN:
			p->type = LUA_TBOOLEAN;
			p->size = 1;
			p->value = lua_toboolean(l, i);
			p++;
			break;
		case LUA_TNUMBER:
			p->type = LUA_TNUMBER;
			p->size = 1;
			p->value = luaL_checknumber(l, i);
			p++;
			break;
		case LUA_TSTRING:
			p->type = LUA_TSTRING;
			size_t sl;
			char* s = luaL_checklstring(l, i, &sl);
			p->size = sl;
			p->value = sl;
		}
	}
	size_t len;
	char* funName = luaL_checklstring(l, 1, &len);
	int ai = luaL_checkinteger(l, 2);


	if (st_thread_create(st_main_call_p, (void*) l, 0, 0) == NULL) {
		return -1;
	}
	return 1;
}

static const luaL_Reg funs[] = {
		{"create_thread", create_thread},
		{"stop_thread", stop_thread},
		{"msleep", msleep},
		{"call_lua_start_fun", call_lua_start_fun},
		{NULL, NULL}
};

LUA_API int lua_open_stlib(lua_State* l) {
	luaL_register(l, LUA_ST_LIB_NAME, funs);
	return 1;
}
