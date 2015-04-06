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


//int64map_t *map = int64map_create(4);
//int64map_put(map, 1, "sss");
//int64map_put(map, 2, "ert");
//int64map_put(map, 3, "asddd");
//int64map_put(map, 4, "sss");
//int64map_put(map, 5, "ert");
//int64map_put(map, 6, "asddd");
//int64map_put(map, 7, "sss");
//int64map_put(map, 8, "ert");
//int64map_put(map, 9, "asddd");
//int64map_put(map, 3, "sss");
//int64map_put(map, 3, "ert");
//int64map_put(map, 3, "asddd");
//int64map_put(map, 13, "sss");
//int64map_put(map, 14, "ert");
//int64map_put(map, 15, "asddd");
//int64map_put(map, 16, "sss");
//int64map_put(map, 17, "ert");
//int64map_put(map, 18, "asddd");
//int64map_put(map, 19, "asddd");
//
//int size = int64map_size(map);
//uint64_t *keys = MALLOC();
//int64map_iterator *it = int64map_new_iterator(map);
//char *tmp;
//uint64_t key;
//while ((tmp=int64map_next(it, &key)) != NULL) {
//	printf("========iterator: key=%lu, val=%s \n", key, tmp);
//}
//int64map_del_iterator(it);
//
//printf("==========%s\n", (char*)int64map_get(map, 3));
//int64map_remove(map, 3);
//printf("==========%s\n", (char*)int64map_get(map, 3));
//	loadconfig();

//	ffid_t ffid = ffid_create(5);
//	int id, i;
//	for (i=0; i<5; i++) {
//		id = ffid_getid(ffid);
//		printf("get i=%i, id=%d \n", i, id);
//	}
//	ffid_releaseid(ffid, 3);
//	ffid_releaseid(ffid, 1);
//	ffid_releaseid(ffid, 4);
//	ffid_releaseid(ffid, 2);
//	ffid_releaseid(ffid, 0);
//	for (i=0; i<5; i++) {
//		id = ffid_getid(ffid);
//		printf("get i=%i, id=%d \n", i, id);
//	}

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
	st_thread_t mt = st_thread_create(lua_pmain, s, 0, 0);

	st_thread_exit(NULL);

	// all thread exit
	lua_close(ls);
	FREE(s);

	return 0;
}
