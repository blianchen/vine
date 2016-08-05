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


/*
@@ LUA_PROMPT is the default prompt used by stand-alone Lua.
@@ LUA_PROMPT2 is the default continuation prompt used by stand-alone Lua.
** CHANGE them if you want different prompts. (You can also change the
** prompts dynamically, assigning to globals _PROMPT/_PROMPT2.)
*/
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "

/*
@@ LUA_MAXINPUT is the maximum length for an input line in the
@* stand-alone interpreter.
** CHANGE it if you need longer lines.
*/
#define LUA_MAXINPUT	512



//// lua线程表在全局环境中的索引
//extern char LUA_THREAD_GLOBAL_IDX;

struct Smain {
	lua_State* main_l;
	char **argv;
	int argc;
	int status;
};

//void open_lua_libs (lua_State *l);

//void load_lua_file(lua_State* l, char* file);

void call_lua_fun(lua_State* l, int nargs, int nresults);


void* pmain(void *arg);

void dump_cstack(lua_State* l);

#endif /* LUAAPI_LUAUTILS_H_ */
