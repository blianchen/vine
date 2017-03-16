
#include <luaapi/luaurl.h>

#include <http_parser.h>
#include <uri.h>


////////////////// a tool function for parse url  /////////////////////
static void push_url_part(lua_State* L, struct http_parser_url* u, int field, const char *url) {
	if (u->field_set & (1 << field)) {
		lua_pushlstring(L, url + u->field_data[field].off, u->field_data[field].len);
	} else {
		lua_pushnil(L);
	}
}

static int lparse_url(lua_State* L) {
	size_t t;
	const char *url = luaL_checklstring(L, 1, &t);

	struct http_parser_url u;
	http_parser_url_init(&u);
	int r = http_parser_parse_url(url, t, 0, &u);
	if (r != 0) {
		LOG_DEBUG("Parse url error.");
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L);
	lua_pushinteger(L, u.port);
	lua_setfield(L, -2, "port");
	push_url_part(L, &u, UF_SCHEMA, url);
	lua_setfield(L, -2, "schema");
	push_url_part(L, &u, UF_HOST, url);
	lua_setfield(L, -2, "host");
	push_url_part(L, &u, UF_PATH, url);
	lua_setfield(L, -2, "path");
	push_url_part(L, &u, UF_QUERY, url);
	lua_setfield(L, -2, "query");
	push_url_part(L, &u, UF_FRAGMENT, url);
	lua_setfield(L, -2, "fragment");
	push_url_part(L, &u, UF_USERINFO, url);
	lua_setfield(L, -2, "userinfo");
	return 1;  // a table. key: port, schema, host, path, query, fragment, userinfo
}


//////////////////////////////// uri object //////
static int luri_new(lua_State* L) {
	char* str = luaL_checkstring(L, 1);
	uri_t u = uri_new(str);
	lua_pushlightuserdata(L, u);
	return 1;
}

static int luri_free(lua_State* L) {
	uri_t u = lua_touserdata(L, 1);
	uri_free(u);
	return 0;
}


static const luaL_Reg funs[] = {
		// A tool function for parse url. The difference with uri, this function is not require clear mem.
		{"parseUrl", lparse_url},	// return a table. key: port, schema, host, path, query, fragment, userinfo
		{NULL, NULL}
};

LUA_API int luaopen_http(lua_State* L) {
	luaL_newlib(L, funs);
	return 1;
}
