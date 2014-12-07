/*
 * luautils.c
 *
 *  Created on: Dec 6, 2014
 *      Author: blc
 */

#include <stdio.h>

#include <mem.h>

#include "luautils.h"
//
//int luastack_to_valist(lua_State* l, va_list* ap, char* argstk, ...) {
//#define INIT_MALLOC_SIZE 1024
//	char* arg;
//	int argNum = lua_gettop(l);
//	if (argNum > 2) {
//		int memSize = INIT_MALLOC_SIZE;
//		arg = MALLOC(memSize);
//		argstk
//		va_start(*ap, argstk);
//
//		int i, type;
//		size_t strLen;
//		for (i=3; i<=argNum; i++) {
//			if (argstk - (char*)ap > INIT_MALLOC_SIZE - 16) {
//				memSize += INIT_MALLOC_SIZE;
////				args = REALLOC(args, memSize);
//			}
//			type = lua_type(l, i);
//			switch (type) {
//			case LUA_TNUMBER:
//				*argstk = luaL_checknumber(l, i);
////				args = &va_arg(*ap, lua_Number);
//				break;
//			case LUA_TSTRING:
//				*argstk = luaL_checklstring(l, i, &strLen);
////				args = &va_arg(*ap, char*);
//				break;
////			case LUA_TBOOLEAN:
////				*args = lua_toboolean(l, i);
////				args = &va_arg(*ap, int);
////				break;
//			default:
////				lua_pushstring(l, "error args");
//				return 0;
//				break;
//			}
//		}
////		va_start(*ap, argstk);
//	}
//	return 1;
//}


void dump_cstack(lua_State* l) {
	int n = lua_gettop(l);
	int i, t;
	for (i=1; i<=n; i++) {
		t = lua_type(l, i);
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

