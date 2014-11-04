/*
 * luast.c
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#include <mem.h>
#include <exception.h>
#include <st/public.h>

#include <stdint.h>

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
//	char* value;
} CallArg;
typedef struct {
	lua_State* l;
	int argsNum;
	CallArg* args;
} CallStackArgs;

//static char* cstStr = "st.call_lua_start_fun(";
void* st_main_call_p(void* args) {
	CallStackArgs* ca = (CallStackArgs*)args;
//	st_thread_setspecific(0, ca);
	char cs[64];
	sprintf(cs, "st.call_lua_start_fun(%llu)", (uint64_t)ca);
	luaL_dostring(ca->l, cs);
	return NULL;
}

int call_lua_start_fun(lua_State* l) {
	printf("in call_lua_main!!! lua_stack_size=%d\n", lua_gettop(l));
	dumpstk(l);
	CallStackArgs* ca = (uint64_t)luaL_checknumber(l, 1);
	lua_pop(l, 1);

	int stkn = ca->argsNum;
	CallArg* p = ca->args;
	int i;
	int size;
	for (i=1; i<=stkn; i++) {
		switch (p->type) {
		case LUA_TBOOLEAN:
			lua_pushboolean(l, p->size);
			p++;
			break;
		case LUA_TNUMBER:
			size = p->size;
			p++;
			lua_pushnumber(l, *((lua_Number*)p));
			p = (CallArg*) ((char*)p + size);
			break;
		case LUA_TSTRING:
			size = p->size;
			p++;
			if (i == 1) {
				lua_getglobal(l, (char*)p);
			} else {
				lua_pushlstring(l, (char*)p, size);
			}
			p = (CallArg*) ((char*)p + size);
			break;
		default:
//			THROW(MemoryException, "throw exception!!!!!!!!!!!!");
			break;
		}
	}
//	dumpstk(l);
	lua_pcall(l, lua_gettop(l)-1, 0, 0);
	return 0;
}

#define INIT_ALLOC_SIZE 1024

int create_thread(lua_State* l) {
//	dumpstk(l);

	int stkn = lua_gettop(l);

	int alloc_size = INIT_ALLOC_SIZE;
	CallStackArgs* callArgs = MALLOC(INIT_ALLOC_SIZE);	// init 2k
	callArgs->l = l;
	callArgs->argsNum = stkn;
	callArgs->args = ((char*)callArgs) + sizeof(CallStackArgs);
	char* sp = (char*)callArgs->args;

	CallArg* p = callArgs->args;
	int i;
	int type;
	for (i=1; i<=stkn; i++) {
		if (sp - (char*)callArgs >= alloc_size) {
			alloc_size += INIT_ALLOC_SIZE;
			callArgs = REALLOC(callArgs, alloc_size);
		}
		type = lua_type(l, i);
		switch (type) {
		case LUA_TBOOLEAN:
			p->type = LUA_TBOOLEAN;
			p->size = lua_toboolean(l, i);
			p++;
			sp = p;
			break;
		case LUA_TNUMBER:
			p->type = LUA_TNUMBER;
			p->size = sizeof(lua_Number);
			sp += sizeof(p);
			*((lua_Number*)sp) = luaL_checknumber(l, i);
			sp += p->size;
			p = (CallArg*)sp;
			break;
		case LUA_TSTRING:
			p->type = LUA_TSTRING;
			size_t sl;
			const char* s = luaL_checklstring(l, i, &sl);
			p->size = sl + 1;
			sp += sizeof(p);
			memcpy(sp, s, p->size);
			sp += p->size;
			p = (CallArg*)sp;
			break;
		default:
//			THROW(MemoryException, "throw exception!!!!!!!!!!!!");
			break;
		}
	}

	if (st_thread_create(st_main_call_p, (void*) callArgs, 0, 0) == NULL) {
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
