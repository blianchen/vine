/*
 * luahttp.c
 *
 *  Created on: Jul 20, 2016
 *      Author: blc
 */

#include <string.h>

#include <luaapi/luahttp.h>

#include <mem.h>
#include <logger.h>
#include <str.h>
#include <luaapi/luautils.h>


#define STRING_CHUNK_SIZE 128

static void update_string_ptr(struct string_ptr* s, const char* at, size_t len) {
	if (!s->str) {
		s->str = MALLOC(STRING_CHUNK_SIZE);
		s->size = STRING_CHUNK_SIZE;
		s->strlen = 0;
	}
	int cl = s->strlen + len;
	if (cl > s->size) {
		REALLOC(s->str, cl);
	}
	memcpy(s->str + s->strlen, at, len);
	s->strlen = cl;
}

static void clear_http_data(http_data_t hp) {
	if (hp->url.str) {
		FREE(hp->url.str);
	}
	if (hp->status_msg.str) {
		FREE(hp->status_msg.str);
	}
	int i;
	for (i=0; i<32; i++) {
		if (hp->fields[i].str) {
			FREE(hp->fields[i].str);
		}
		if (hp->values[i].str) {
			FREE(hp->values[i].str);
		}
	}
	FREE(hp);
}

static char* get_http_method_name(int m) {
	switch (m) {
		case HTTP_DELETE:
			return "delete";
			break;
		case HTTP_HEAD:
			return "head";
			break;
		case HTTP_POST:
			return "post";
			break;
		case HTTP_PUT:
			return "PUT";
			break;
		case HTTP_CONNECT:
			return "connect";
			break;
		case HTTP_OPTIONS:
			return "options";
			break;
		case HTTP_TRACE:
			return "trace";
			break;
		default:
			return "get";
	}
}

static http_data_t init_parser(socket_t socket, enum http_parser_type type) {
	http_data_t hp = CALLOC(1, sizeof(struct http_data_s));
	hp->parser.data = hp;
	hp->socket = socket;
	http_parser_init(&hp->parser, type);

	hp->on_messag_begin = LUA_REFNIL;
	hp->on_url = LUA_REFNIL;
	hp->on_status = LUA_REFNIL;
	hp->on_headers_complete = LUA_REFNIL;
	hp->on_upgrade = LUA_REFNIL;
	hp->on_body = LUA_REFNIL;
	hp->on_message_complete = LUA_REFNIL;

	return hp;
}


//////////////////////////////////////// parser call back ///////////////////////////////////////
static int on_message_begin(http_parser* parser) {
	http_data_t hp = parser->data;
	if (hp->on_messag_begin != LUA_REFNIL) {
		lua_getglobal(hp->L, HTTP_PARSER_CALLBACK_TABLE);
		lua_rawgeti(hp->L, -1, hp->on_messag_begin);
		lua_pushlightuserdata(hp->L, hp);
		call_lua_fun(hp->L, 1, 0);
	}
	return 0;
}

static int on_url(http_parser* parser, const char *at, size_t len) {
	http_data_t hp = parser->data;
	if (hp->on_url != LUA_REFNIL) {
		update_string_ptr(&hp->url, at, len);

		lua_getglobal(hp->L, HTTP_PARSER_CALLBACK_TABLE);
		lua_rawgeti(hp->L, -1, hp->on_url);
		lua_pushlightuserdata(hp->L, hp);
		lua_pushlstring(hp->L, hp->url.str, hp->url.strlen);
		call_lua_fun(hp->L, 2, 0);
	}
	return 0;
}

// function void on_status(string)
static int on_status(http_parser* parser, const char *at, size_t len) {
	http_data_t hp = parser->data;
	if (hp->on_status != LUA_REFNIL) {
		update_string_ptr(&hp->status_msg, at, len);

		lua_getglobal(hp->L, HTTP_PARSER_CALLBACK_TABLE);
		lua_rawgeti(hp->L, -1, hp->on_status);
		lua_pushlightuserdata(hp->L, hp);
		lua_pushlstring(hp->L, hp->status_msg.str, hp->status_msg.strlen);
		call_lua_fun(hp->L, 2, 0);
	}
	return 0;
}

static int on_header_field(http_parser* parser, const char *at, size_t len) {
	http_data_t hp = parser->data;
	if (hp->last_header_element == VALUE) {
		hp->fields_num++;
	}
	update_string_ptr(&hp->fields[hp->fields_num], at, len);
	hp->last_header_element = FIELD;
	return 0;
}

static int on_header_value(http_parser* parser, const char *at, size_t len) {
	http_data_t hp = parser->data;
	update_string_ptr(&hp->values[hp->fields_num], at, len);
	hp->last_header_element = VALUE;
	return 0;
}

// function void on_headers_complete(schema, method, status_code, alive, fields_table)
static int on_headers_complete(http_parser* parser) {
	http_data_t hp = parser->data;

	hp->http_major = parser->http_major;
	hp->http_minor = parser->http_minor;
	hp->method = parser->method;
	hp->status_code = parser->status_code;
	hp->alive = http_should_keep_alive(parser);
	hp->upgrade = parser->upgrade;

	if (hp->on_headers_complete != LUA_REFNIL) {
		lua_getglobal(hp->L, HTTP_PARSER_CALLBACK_TABLE);
		lua_rawgeti(hp->L, -1, hp->on_headers_complete);
		lua_pushlightuserdata(hp->L, hp);
		call_lua_fun(hp->L, 1, 0);
	}
	return 0;
}

static int on_body(http_parser* parser, const char *at, size_t len) {
	http_data_t hp = parser->data;
	if (hp->on_body != LUA_REFNIL) {
		update_string_ptr(&hp->body, at, len);

		lua_getglobal(hp->L, HTTP_PARSER_CALLBACK_TABLE);
		lua_rawgeti(hp->L, -1, hp->on_body);
		lua_pushlightuserdata(hp->L, hp);
		lua_pushlstring(hp->L, hp->body.str, hp->body.strlen);
		call_lua_fun(hp->L, 2, 0);
	}
	return 0;
}

static int on_message_complete(http_parser* parser) {
	http_data_t hp = parser->data;
	if (hp->on_message_complete != LUA_REFNIL) {
		lua_getglobal(hp->L, HTTP_PARSER_CALLBACK_TABLE);
		lua_rawgeti(hp->L, -1, hp->on_message_complete);
		lua_pushlightuserdata(hp->L, hp);
		call_lua_fun(hp->L, 1, 0);
	}
	// todo close net
	return 0;
}
//////////////////////////////////////// parser call back end ///////////////////////////////////////



static int linit_parser(lua_State* L) {  // init(socket, type);  type: "request", "response", "both"
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	socket_t socket = lua_touserdata(L, 1);
	size_t size;
	const char* typeStr = luaL_checklstring(L, 2, &size);
	if (size < 3) {
		LOG_DEBUG("Parameter [type] error, acceptable string: request, response, both.");
		lua_pushnil(L);
		return 1;
	}

	enum http_parser_type type;
	if (typeStr[2] == 'q' || typeStr[2] == 'Q') {
		type = HTTP_REQUEST;
	} else if (typeStr[2] == 's' || typeStr[2] == 'S') {
		type = HTTP_RESPONSE;
	} else if (typeStr[2] == 't' || typeStr[2] == 'T') {
		type = HTTP_BOTH;
	} else {
		LOG_DEBUG("Parameter [type] error, acceptable string: request, response, both.");
		lua_pushnil(L);
		return 1;
	}

	http_data_t hp = init_parser(socket, type);
	hp->L = L;
	lua_pushlightuserdata(L, hp);
	return 1;
}

static int lset_on_message_begin(lua_State* L) {
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	http_data_t hp = lua_touserdata(L, 1);
	lua_getglobal(L, HTTP_PARSER_CALLBACK_TABLE);
	lua_insert(L, 2);
	int ref = luaL_ref(L, 2);
	hp->on_messag_begin = ref;
	hp->settings.on_message_begin = on_message_begin;
	return 0;
}

static int lset_on_url(lua_State* L) { // callback function void on_url(string)
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	http_data_t hp = lua_touserdata(L, 1);
	lua_getglobal(L, HTTP_PARSER_CALLBACK_TABLE);
	lua_insert(L, 2);
	int ref = luaL_ref(L, 2);
	hp->on_url = ref;
	hp->settings.on_url = on_url;
	return 0;
}

static int lset_on_status(lua_State* L) { // callback function void on_status(string)
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	http_data_t hp = lua_touserdata(L, 1);
	lua_getglobal(L, HTTP_PARSER_CALLBACK_TABLE);
	lua_insert(L, 2);
	int ref = luaL_ref(L, 2);
	hp->on_status = ref;
	hp->settings.on_status = on_status;
	return 0;
}

static int lset_on_headers_complete(lua_State* L) {
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	http_data_t hp = lua_touserdata(L, 1);
	lua_getglobal(L, HTTP_PARSER_CALLBACK_TABLE);
	lua_insert(L, 2);
	int ref = luaL_ref(L, 2);
	hp->on_headers_complete = ref;
	hp->settings.on_headers_complete = on_headers_complete;
	hp->settings.on_header_field = on_header_field;
	hp->settings.on_header_value = on_header_value;
	return 0;
}

static int lget_http_version(lua_State* L) { // return  major, minor
	http_data_t hp = lua_touserdata(L, 1);
	lua_pushinteger(L, hp->http_major);
	lua_pushinteger(L, hp->http_minor);
	return 2;
}

static int lget_http_method(lua_State* L) {
	http_data_t hp = lua_touserdata(L, 1);
	char* method = get_http_method_name(hp->method);
	lua_pushstring(L, method);
	return 1;
}

static int lget_http_status_code(lua_State* L) {
	http_data_t hp = lua_touserdata(L, 1);
	lua_pushinteger(L, hp->status_code);
	return 1;
}

static int lget_http_upgrade(lua_State* L) {
	http_data_t hp = lua_touserdata(L, 1);
	lua_pushboolean(L, hp->upgrade);
	return 1;
}

static int lget_http_alive(lua_State* L) {
	http_data_t hp = lua_touserdata(L, 1);
	lua_pushboolean(L, hp->alive);
	return 1;
}

static int lget_http_headers(lua_State* L) { //return a table
	http_data_t hp = lua_touserdata(L, 1);
	// header field
	lua_newtable(hp->L);
	int i;
	for (i=0; i<hp->fields_num; i++) {
		lua_pushlstring(hp->L, hp->fields[i].str, hp->fields[i].strlen);
		lua_pushlstring(hp->L, hp->values[i].str, hp->values[i].strlen);
		lua_rawset(hp->L, -3);
	}
	return 1;
}

static int lset_on_body(lua_State* L) { // callback function void on_body(string)
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	http_data_t hp = lua_touserdata(L, 1);
	lua_getglobal(L, HTTP_PARSER_CALLBACK_TABLE);
	lua_insert(L, 2);
	int ref = luaL_ref(L, 2);
	hp->on_body = ref;
	hp->settings.on_body = on_body;
	return 0;
}

static int lset_on_message_complete(lua_State* L) {
	if (lua_gettop(L) != 2) {
		LOG_DEBUG("Parameter error, Two parameters are required.");
		return 0;
	}
	http_data_t hp = lua_touserdata(L, 1);
	lua_getglobal(L, HTTP_PARSER_CALLBACK_TABLE);
	lua_insert(L, 2);
	int ref = luaL_ref(L, 2);
	hp->on_message_complete = ref;
	hp->settings.on_message_complete = on_message_complete;
	return 0;
}

static int lparser_execute(lua_State* L) {
	http_data_t hp = lua_touserdata(L, 1);
	int rn;
	size_t nparsed;
	while ((rn = nets_read(hp->socket)) > 0) {
		nparsed = http_parser_execute(&hp->parser, &hp->settings, hp->socket->buf, rn);

		if (!hp->parser.upgrade && nparsed != rn) {
			lua_pushstring(L, http_errno_description(hp->parser.http_errno));
			return 1;
		}
	}
	//parse success(=0) or socket error(<0)
	if (rn < 0) {
		lua_pushstring(L, getLastErrorText());
		return 1;
	}
	lua_pushinteger(L, 0);
	return 1;
}

static int lclear_parser(lua_State* L) {
	http_data_t hp = lua_touserdata(L, 1);
	clear_http_data(hp);
	return 0;
}


static const luaL_Reg funs[] = {
		{"create", linit_parser},							// parser = create(socket, type)
		{"setMessageBegin", lset_on_message_begin},			// setMessageBegin(parser, callbackFun); 	callback function void on_message_begin(parser)
		{"setUrl", lset_on_url},							// setUrl(parser, callbackFun); 			callback function void on_url(parser, string)
		{"setStatus", lset_on_status},						// setStatus(parser, callbackFun); 			callback function void on_status(parser, string)
		{"setHeadersComplete", lset_on_headers_complete},	// setHeadersComplete(parser, callbackFun); callback function void on_headers_complete(parser)
		{"setBody", lset_on_body},							// setBody(parser, callbackFun); 			callback function void on_body(parser, string)
		{"setMessageComplete", lset_on_message_complete},	// setMessageComplete(parser, callbackFun); callback function void on_message_complete(parser)
		{"execute", lparser_execute},						// execute(parser)
		{"clear", lclear_parser},
		// get data function in on_headers_complete
		{"getHttpVersion", lget_http_version},				// major, minor = getHttpVersion(parser)
		{"getHttpMethod", lget_http_method},
		{"getHttpStatusCode", lget_http_status_code},
		{"getHttpUpgrade", lget_http_upgrade},
		{"getHttpAlive", lget_http_alive},
		{"getHttpHeaders", lget_http_headers},
		{NULL, NULL}
};

LUA_API int luaopen_http(lua_State* L) {
	luaL_newlib(L, funs);

	lua_newtable(L);
	lua_setglobal(L, HTTP_PARSER_CALLBACK_TABLE);

	return 1;
}
