/*
 * luast.h
 *
 *  Created on: Oct 31, 2014
 *      Author: blc
 */

#ifndef LUAAPI_LUAST_H_
#define LUAAPI_LUAST_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <st/st.h>


#ifndef offsetof
#define offsetof(type, identifier) ((size_t)&(((type *)0)->identifier))
#endif

#define ST_THREAD_STACK_SIZE (16*1024)

#define LUA_ST_LIBNAME "st"

//// lua线程表在全局环境中的索引
extern char LUA_THREAD_GLOBAL_IDX;

//void st_context_init();

LUA_API int luaopen_st(lua_State* l);


////////////////////////////////////////////////////////////
////  private  function
////////////////////////////////////////////////////////////
void dump_cstack(lua_State* l);

#endif /* LUAAPI_LUAST_H_ */
