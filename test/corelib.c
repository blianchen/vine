/*
 ============================================================================
 Name        : ccore.c
 Author      : blc
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "crypto_test.h"
#include "core_test.h"

#include <exception/exception.h>
#include <mem.h>
#include <str.h>

#include <lua.h>

#include <luaapi/luautils.h>
#include <luaapi/luast.h>
#include <luaapi/luadb.h>
#include <luaapi/luanet.h>

#include <st/st.h>
#include <stdint.h>

#include <nets.h>

typedef struct config_t {
	char* mainfile;
	char* mainfun;
} config_t;

config_t config;

void* startLuaMain(void* arg) {
	lua_State* ls = arg;

	load_lua_file(ls, config.mainfile);		// lua file
	call_lua_fun(ls, 0, 0);
	lua_getglobal(ls, config.mainfun);	// lua function
	call_lua_fun(ls, 0, 0);
	return NULL;
}

void loadconfig() {
	FILE *cfgFile = fopen("config.ini", "r");
	if (!cfgFile) {
		LOG_WARN("****ERROR****:Do not find file config.ini");
		exit(1);
		return;
	}
	// 分析行
//#define LOG_CFG_ROW_LEN 256
	char delims[] = "=";
	while (!feof(cfgFile)) {
		char strLine[256];
		if (!fgets(strLine, 256, cfgFile))
			break;
		if (strLine[0] == '#' || str_trim(strLine)[0] == '\0') continue;
		// 分割'='
		char *name;
		char *value;
		name = str_trim(strtok(strLine, delims));
		if (!name || strlen(name) == 0) continue;
		value = str_trim(strtok(NULL, delims));
		if (!value || strlen(value) == 0) continue;
		if (strcmp(name, "mainfile") == 0) {
			config.mainfile = str_dup(value);
		} else if (strcmp(name, "mainfun") == 0) {
			config.mainfun = str_dup(value);
		} else {
			LOG_DEBUG("Invalid config item(%s) in config.ini", name);
		}
	}
	fclose(cfgFile);
}

int main(void) {
	puts("!!!!!!!!!!!!!!!!! UNIT TEST !!!!!!!!!!!!!!!!!!!!"); /* prints !!!Hello World!!! */




//	char bbbb[] = {"aa =123  , bb= 4_56 , cc = gh?jkk , "
//			"kk=vvv"};
//	kv_option_t op = parse_kv_option(bbbb);

	exception_init();

	loadconfig();

	lua_State* ls = luaL_newstate();
	open_lua_libs(ls);

	luaopen_stlib(ls);
	luaopen_netlib(ls);
	luaopen_dblib(ls);

	/*
	 * register a table to anchor lua coroutines reliably:
	 * {([int]ref) = [cort]}
	 */
	lua_pushlightuserdata(ls, &LUA_THREAD_GLOBAL_IDX);
	lua_newtable(ls);
	lua_rawset(ls, LUA_GLOBALSINDEX);

	//// st
	st_set_eventsys(ST_EVENTSYS_ALT);

	if (st_init() < 0) {
//		THROW(st_exception, "st init error.");
	}

	//// lua main fun
	st_thread_create(startLuaMain, ls, 0);

	st_thread_exit(NULL);
	lua_close(ls);

	return 0;


	SRunner* sr = srunner_create(make_crypto_suite());
	srunner_run_all(sr, CK_NORMAL);

	SRunner* sr1 = srunner_create(make_core_suite());
	srunner_run_all(sr1, CK_NORMAL);

	srunner_free(sr);
	srunner_free(sr1);

	return EXIT_SUCCESS;
}
