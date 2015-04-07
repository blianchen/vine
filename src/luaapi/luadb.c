/*
 * luadb.c
 *
 *  Created on: Nov 21, 2014
 *      Author: blc
 */
#include <stdint.h>
#include <string.h>

#include <mem.h>
#include <logger.h>

#include <db/dbpool.h>
#include <db/dbconn.h>
#include <db/dbrs.h>

#include <luaapi/luadb.h>

#include <libpq-fe.h>

//SELECT pg_stat_get_backend_pid(s.backendid) AS procpid, pg_stat_get_backend_activity(s.backendid) AS current_query FROM (SELECT pg_stat_get_backend_idset() AS backendid) AS s;

//////////////////////////////// common /////////////////////
static int ldb_version(lua_State* l) {
	const char* v = dbpool_version();
	lua_pushstring(l, v);
	return 1;
}

static int ldb_isSupported(lua_State* l) {
	const char* url = luaL_checkstring(l, 1);
	if (dbconn_isSupported(url)) {
		lua_pushinteger(l, 1);
	} else {
		lua_pushinteger(l, 0);
	}
	return 1;
}


///////////////////////////// conntion poll ///////////////////////////////
static int ldb_pool_new(lua_State* l) {
	const char* url = luaL_checkstring(l, 1);	//url string
	uri_t uri = uri_new(url);
	dbpool_t pool = dbpool_new(uri);

	lua_pushlightuserdata(l, pool);
	return 1;
}

static int ldb_pool_setInitialConn(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	int num = luaL_checkinteger(l, 2);
	dbpool_setInitialConn(pool, num);
	return 0;
}

static int ldb_pool_setMaxConn(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	int num = luaL_checkinteger(l, 2);
	dbpool_setMaxConn(pool, num);
	return 0;
}

static int ldb_pool_setSqlTimeout(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	int num = luaL_checkinteger(l, 2);
	dbpool_setSqlTimeout(pool, num);
	return 0;
}

static int ldb_pool_start(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	dbpool_start(pool);
	return 0;
}

static int ldb_pool_stop(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	dbpool_stop(pool);
	return 0;
}

static int ldb_pool_free(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	dbpool_free(&pool);
	return 0;
}

static int ldb_pool_getConn(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	dbconn_t conn = dbpool_getConn(pool);
	lua_pushlightuserdata(l, conn);
	return 1;
}

static int ldb_pool_active(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	int n = dbpool_active(pool);
	lua_pushinteger(l, n);
	return 1;
}


//////////////////////////////  db conntion //////////////////////
static int ldb_conn_execute(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	const char* sql = luaL_checkstring(l, 2);

//	TRY
		dbconn_execute(conn, sql);
//	ELSE
//		LOG_DEBUG("DB execute error: %s.(%s)", sql, dbconn_getLastError(conn));
//	END_TRY
	return 0;
}

static int ldb_conn_executeQuery(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	const char* sql = luaL_checkstring(l, 2);

	dbrs_t rs = NULL;
//	TRY
		rs = dbconn_executeQuery(conn, sql);
//	ELSE
//		LOG_DEBUG("DB execute error: %s.(%s)", sql, dbconn_getLastError(conn));
//	END_TRY

	lua_pushlightuserdata(l, rs);
	return 1;
}

static int ldb_conn_close(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	dbconn_close(conn);
	return 0;
}

static int ldb_conn_beginTransaction(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	dbconn_beginTransaction(conn);
	return 0;
}

static int ldb_conn_commit(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	dbconn_commit(conn);
	return 0;
}

static int ldb_conn_rollback(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	dbconn_rollback(conn);
	return 0;
}

static int ldb_conn_prepareStatement(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	const char* sql = luaL_checkstring(l, 2);
	dbpst_t pst = dbconn_prepareStatement(conn, sql);
	lua_pushlightuserdata(l, pst);
	return 1;
}


///////////////////////////// db prepareStatement  ////////////////
static int ldb_pst_setString(lua_State* l) {
	dbpst_t pst = lua_touserdata(l, 1);
	int paramId = luaL_checkinteger(l, 2);
	const char* s = luaL_checkstring(l, 3);
	dbpst_setString(pst, paramId, s);
	return 0;
}

static int ldb_pst_setInt(lua_State* l) {
	dbpst_t pst = lua_touserdata(l, 1);
	int paramId = luaL_checkinteger(l, 2);
	int n = luaL_checkinteger(l, 3);
	dbpst_setInt(pst, paramId, n);
	return 0;
}

static int ldb_pst_setLLong(lua_State* l) {
	dbpst_t pst = lua_touserdata(l, 1);
	int paramId = luaL_checkinteger(l, 2);
	long long n = luaL_checknumber(l, 3);
	dbpst_setLLong(pst, paramId, n);
	return 0;
}

static int ldb_pst_setDouble(lua_State* l) {
	dbpst_t pst = lua_touserdata(l, 1);
	int paramId = luaL_checkinteger(l, 2);
	double n = luaL_checknumber(l, 3);
	dbpst_setDouble(pst, paramId, n);
	return 0;
}

static int ldb_pst_execute(lua_State* l) {
	dbpst_t pst = lua_touserdata(l, 1);
	dbpst_execute(pst);
	return 0;
}

static int ldb_pst_executeQuery(lua_State* l) {
	dbpst_t pst = lua_touserdata(l, 1);

	dbrs_t rs = NULL;
	rs = dbpst_executeQuery(pst);

	lua_pushlightuserdata(l, rs);
	return 1;
}


//////////////////////////////  db result ////////////////////
static int ldb_rs_next(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int n = dbrs_next(rs);
	lua_pushinteger(l, n);
	return 1;
}

static int ldb_rs_getColumnCount(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int n = dbrs_getColumnCount(rs);
	lua_pushinteger(l, n);
	return 1;
}

static int ldb_rs_getColumnName(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	const char* name = dbrs_getColumnName(rs, col);
	lua_pushstring(l, name);
	return 1;
}

static int ldb_rs_getColumnSize(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	int n = dbrs_getColumnSize(rs, col);
	lua_pushinteger(l, n);
	return 1;
}

static int ldb_rs_isnull(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	if (dbrs_isnull(rs, col)) {
		lua_pushboolean(l, 1);
	} else {
		lua_pushboolean(l, 0);
	}
	return 1;
}

///////////// get  ///
static int ldb_rs_getString(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	const char* v = dbrs_getString(rs, col);
	lua_pushstring(l, v);
	return 1;
}

static int ldb_rs_getStringByName(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	const char* name = luaL_checkstring(l, 2);
	const char* v = dbrs_getStringByName(rs, name);
	lua_pushstring(l, v);
	return 1;
}

static int ldb_rs_getInt(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	int v = dbrs_getInt(rs, col);
	lua_pushinteger(l, v);
	return 1;
}

static int ldb_rs_getIntByName(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	const char* name = luaL_checkstring(l, 2);
	int v = dbrs_getIntByName(rs, name);
	lua_pushinteger(l, v);
	return 1;
}

static int ldb_rs_getLLong(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	int64_t v = dbrs_getLLong(rs, col);
	lua_pushnumber(l, v);
	return 1;
}

static int ldb_rs_getLLongByName(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	const char* name = luaL_checkstring(l, 2);
	int64_t v = dbrs_getLLongByName(rs, name);
	lua_pushnumber(l, v);
	return 1;
}

static int ldb_rs_getDouble(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	double v = dbrs_getDouble(rs, col);
	lua_pushnumber(l, v);
	return 1;
}

static int ldb_rs_getDoubleByName(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	const char* name = luaL_checkstring(l, 2);
	double v = dbrs_getDoubleByName(rs, name);
	lua_pushnumber(l, v);
	return 1;
}


/////////////////////// help function //////
static int ldb_help_nosqlGet(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	int type = lua_type(l, 2);
	if (type != LUA_TSTRING) {
		LOG_DEBUG("key type(%d) must be string.", type);
		lua_pushnil(l);
		return 1;
	}
	const char* key = luaL_checkstring(l, 2);

	char sql[128] = {"GET "};
	strcat(sql, key);

	dbconn_t conn = dbpool_getConn(pool);
	dbrs_t rs = dbconn_executeQuery(conn, sql);
	const char* v = dbrs_getString(rs, 1);
	if (v)
		lua_pushstring(l, v);	// copy str
	else
		lua_pushnil(l);
	dbconn_close(conn);
	return 1;
}

static int ldb_help_nosqlSet(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	int type = lua_type(l, 2);
	if (type != LUA_TSTRING) {
		LOG_DEBUG("key type(%d) must be string.", type);
		lua_pushnil(l);
		return 1;
	}
	const char* key = luaL_checkstring(l, 2);
	char sql[128] = {"SET "};
	strcat(sql, key);

	char buf[24];
	int ki;
	double kn;
	type = lua_type(l, 3);
	switch (type) {
	case LUA_TSTRING:
		key = luaL_checkstring(l, 3);
		break;
	case LUA_TBOOLEAN:
		ki = lua_toboolean(l, 3);
		sprintf(buf, "%d", ki);
		key = buf;
		break;
	case LUA_TNUMBER:
		kn = luaL_checknumber(l, 3);
		sprintf(buf, "%lf", kn);
		key = buf;
		break;
	default:
		LOG_DEBUG("invalid value type(%d).", type);
		lua_pushinteger(l, -1);
		return 1;
	}
	key = buf;
	strcat(sql, " ");
	strcat(sql, key);

	dbconn_t conn = dbpool_getConn(pool);
	dbconn_execute(conn, sql);
	dbconn_close(conn);
	lua_pushinteger(l, 0);
	return 1;
}


static const luaL_Reg funs[] = {
		{"version", ldb_version},
		{"isSupported", ldb_isSupported},

		//////////////////// pool
		{"newPool", ldb_pool_new},
		{"setInitialConn", ldb_pool_setInitialConn},
		{"setMaxConn", ldb_pool_setMaxConn},
		{"setSqlTimeout", ldb_pool_setSqlTimeout},
		{"start", ldb_pool_start},
		{"stop", ldb_pool_stop},
		{"freePool", ldb_pool_free},
		{"getConntion", ldb_pool_getConn},
		{"active", ldb_pool_active},

		/////////////////// connection
		{"execute", ldb_conn_execute},
		{"executeQuery", ldb_conn_executeQuery},
		{"close", ldb_conn_close},
		{"beginTransaction", ldb_conn_beginTransaction},
		{"commit", ldb_conn_commit},
		{"rollback", ldb_conn_rollback},
		{"prepareStatement", ldb_conn_prepareStatement},

		/////////////////// prepare statement
		{"setString", ldb_pst_setString},
		{"setInt", ldb_pst_setInt},
		{"setLLong", ldb_pst_setLLong},
		{"setDouble", ldb_pst_setDouble},
		{"executePst", ldb_pst_execute},
		{"executePstQuery", ldb_pst_executeQuery},

		///////////////////// result
		{"next", ldb_rs_next},
		{"getColumnCount", ldb_rs_getColumnCount},
		{"getColumnName", ldb_rs_getColumnName},
		{"getColumnSize", ldb_rs_getColumnSize},
		{"isnull", ldb_rs_isnull},
		// get value,
		{"getString", ldb_rs_getString},
		{"getStringByName", ldb_rs_getStringByName},
		{"getInt", ldb_rs_getInt},
		{"getIntByName", ldb_rs_getIntByName},
		{"getLLong", ldb_rs_getLLong},
		{"getLLongByName", ldb_rs_getLLongByName},
		{"getDouble", ldb_rs_getDouble},
		{"getDoubleByName", ldb_rs_getDoubleByName},

		//////////////////// help function ///
		{"get", ldb_help_nosqlGet},
		{"set", ldb_help_nosqlSet},
		{NULL, NULL}
};

LUA_API int luaopen_db(lua_State* l) {
	luaL_register(l, LUA_DB_LIBNAME, funs);
	return 1;
}
