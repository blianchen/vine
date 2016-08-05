#ifndef _LUA_HTTP_H_
#define _LUA_HTTP_H_

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <nets.h>

#include <http_parser.h>


#define HTTP_PARSER_CALLBACK_TABLE "_http_parser_cb_t_"

#define LUA_HTTP_LIBNAME "http"


struct string_ptr {
	char* str;
	size_t strlen;
	size_t size;
};

typedef struct http_data_s {
	http_parser parser;
	http_parser_settings settings;

	lua_State* L;
	socket_t socket;

	struct string_ptr url;
	struct string_ptr status_msg;
	struct string_ptr body;
	int http_major;
	int http_minor;
	int method;
	int status_code;
	int alive;
	int upgrade;

	int fields_num;
	enum { NONE=0, FIELD, VALUE } last_header_element;
	struct string_ptr fields[32];
	struct string_ptr values[32];

	// callback lua ref
	int on_messag_begin;
	int on_url;
	int on_status;
	int on_headers_complete;
	int on_upgrade;
	int on_body;
	int on_message_complete;
} *http_data_t;


LUA_API int luaopen_http(lua_State* l);

#endif
