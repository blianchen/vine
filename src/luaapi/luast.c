/*
 * luast.c
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#include <stdio.h>
#include <string.h>

#include <mem.h>
#include <exception.h>
#include <st/public.h>

#include <luaapi/luast.h>

#define LUA_ST_LIB_NAME "st"


//void st_context_init() {
//	if (st_init() < 0) {
////		THROW(st_exception, "st init error.");
//	}
//}

//int stop_thread(lua_State* l) {
//	st_thread_exit(NULL);
//	return 0;
//}

int msleep(lua_State* l) {
	int ms = luaL_checkinteger(l, 1);
	st_usleep(ms * 1000);
	return 0;
}

typedef struct {
	lua_State* l;
	char* luaStr;
} CallLuaStr;

void* st_main_call_p(void* args) {
	CallLuaStr* cls = (CallLuaStr*) args;
	printf("lua str == %s\n", cls->luaStr);
	luaL_dostring(cls->l, cls->luaStr);
	FREE(cls);
	return NULL;
}

#define INIT_ALLOC_SIZE 1024
#define MAX_DOUBLE_DIGIT_NUM 24

int create_thread(lua_State* l) {
	CallLuaStr* cls = MALLOC(INIT_ALLOC_SIZE);

	cls->l = lua_newthread(l);
//	cls->l = luaL_newstate();
//	luaL_openlibs(cls->l);
//	lua_open_stlib(cls->l);
//	luaL_loadfile(cls->l, "t.lua");
//	lua_pcall(cls->l, 0, 0, 0);
	cls->luaStr = (char*)cls + sizeof(CallLuaStr);

	int stkn = lua_gettop(l);

	int alloc_size = INIT_ALLOC_SIZE - sizeof(cls) - 1;
	int sp = 0;
	char df[MAX_DOUBLE_DIGIT_NUM];
	int tmpStrLen;
	const char* s; size_t sl;
	int i;
	int type;
	for (i = 1; i <= stkn; i++) {
		type = lua_type(l, i);
		switch (type) {
		case LUA_TBOOLEAN:
			if (sp + 6 >= alloc_size) {
				alloc_size += INIT_ALLOC_SIZE;
				cls = REALLOC(cls, alloc_size);
			}
			if (lua_toboolean(l, i)) {
				strcat(cls->luaStr, "true,");
				sp += 5;
			} else {
				strcat(cls->luaStr, "false,");
				sp += 6;
			}
			break;
		case LUA_TNUMBER:
			sprintf(df, "%f,", luaL_checknumber(l, i));
			tmpStrLen = strlen(df);
			if (sp + tmpStrLen >= alloc_size) {
				alloc_size += INIT_ALLOC_SIZE;
				cls = REALLOC(cls, alloc_size);
			}
			strcat(cls->luaStr, df);
			sp += tmpStrLen;
			break;
		case LUA_TSTRING:
			s = luaL_checklstring(l, i, &sl);
			if (sp + sl >= alloc_size) {
				alloc_size += INIT_ALLOC_SIZE;
				cls = REALLOC(cls, alloc_size);
			}

			if (i == 1) {
				strcpy(cls->luaStr, s);
				strcat(cls->luaStr, "(");
				sp++;
			} else {
				strcat(cls->luaStr, s);
				strcat(cls->luaStr, ",");
				sp++;
			}
			sp += sl;

			break;
		default:
//			THROW(STException, "throw exception!!!!!!!!!!!!");
			break;
		}
	}

	*(cls->luaStr + sp - 1) = '\0';
	strcat(cls->luaStr, ")");

	if (st_thread_create(st_main_call_p, (void*) cls, 0, 0) == NULL) {
		return -1;
	}
	return 1;
}


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


static const luaL_Reg funs[] = {
		{"create_thread", create_thread},
		{"msleep", msleep},
		{NULL, NULL}
};

LUA_API int lua_open_stlib(lua_State* l) {
	luaL_register(l, LUA_ST_LIB_NAME, funs);
	return 1;
}

