/*
 * luautils.h
 *
 *  Created on: Dec 6, 2014
 *      Author: blc
 */

#ifndef LUAAPI_LUAUTILS_H_
#define LUAAPI_LUAUTILS_H_

#include <stdarg.h>

#include <lua.h>

//int luastack_to_valist(lua_State* l, va_list* ap, char* argstk, ...);

void dump_cstack(lua_State* l);

#endif /* LUAAPI_LUAUTILS_H_ */
