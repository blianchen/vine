/*
 * vine.c
 *
 *  Created on: Dec 25, 2014
 *      Author: blc
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
//#include <lualib.h>

#include <luaapi/luautils.h>

#include <mem.h>
#include <logger.h>
#include <utils.h>
#include <str.h>
#include <exception/exception.h>
#include <st/st.h>

#include <vine.h>

//typedef struct config_t {
//	char* mainfile;
//	char* mainfun;
//} config_t;
//
//config_t config;

//static void* startLuaMain(void* arg) {
//	lua_State* ls = arg;
//
//	load_lua_file(ls, config.mainfile);		// lua file
//	call_lua_fun(ls, 0, 0);
//	lua_getglobal(ls, config.mainfun);		// lua function
//	call_lua_fun(ls, 0, 0);
//	return NULL;
//}
//
//static void loadconfig() {
//	FILE *cfgFile = fopen("config.ini", "r");
//	if (!cfgFile) {
//		LOG_WARN("****ERROR****:Do not find file config.ini");
//		exit(1);
//		return;
//	}
//	// 分析行
////#define LOG_CFG_ROW_LEN 256
//	char delims[] = "=";
//	while (!feof(cfgFile)) {
//		char strLine[256];
//		if (!fgets(strLine, 256, cfgFile))
//			break;
//		if (strLine[0] == '#' || str_trim(strLine)[0] == '\0') continue;
//		// 分割'='
//		char *name;
//		char *value;
//		name = str_trim(strtok(strLine, delims));
//		if (!name || strlen(name) == 0) continue;
//		value = str_trim(strtok(NULL, delims));
//		if (!value || strlen(value) == 0) continue;
//		if (strcmp(name, "mainfile") == 0) {
//			config.mainfile = str_dup(value);
//		} else if (strcmp(name, "mainfun") == 0) {
//			config.mainfun = str_dup(value);
//		} else {
//			LOG_DEBUG("Invalid config item(%s) in config.ini", name);
//		}
//	}
//	fclose(cfgFile);
//}

#include <intmap.h>
#include <ffid.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <exception/net_exception.h>

int main(int argc, char **argv) {
	exception_init();

	char *nodeName = NULL;

	/*
	 * parse argument
	 * -n nodeName
	 */
	int i;
	for (i = 1; argv[i] != NULL; i++) {
		if (argv[i][0] != '-') /* not an option? */
			break;

		switch (argv[i][1]) { /* option */
		case 'n':
			if (argv[i][2] != '\0') return -1;
			nodeName = argv[i + 1] != NULL ? argv[i + 1] : NULL;
			break;
		default:
			break;
		}
	}

	lua_State* ls = luaL_newstate();
	if (ls == NULL) {
		LOG_WARN("%s: cannot create state: not enough memory", argv[0]);
		return 1;
	}

	//// st
	st_set_eventsys(ST_EVENTSYS_ALT);

	if (st_init() < 0) {
//		THROW(st_exception, "st init error.");
	}

	st_rms_init(nodeName);


	//// lua main fun
	struct Smain *s = MALLOC(sizeof(struct Smain));
	s->argc = argc;
	s->argv = argv;
	s->main_l = ls;
	st_thread_create(pmain, s, 0, 0);

	st_thread_exit(NULL);

	// all thread exit
	lua_close(ls);
	FREE(s);

	return 0;
}
