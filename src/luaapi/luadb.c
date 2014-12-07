/*
 * luadb.c
 *
 *  Created on: Nov 21, 2014
 *      Author: blc
 */

#include <exception/sql_exception.h>
#include <mem.h>
#include <logger.h>

#include <db/dbpool.h>
#include <db/dbconn.h>
#include <db/dbrs.h>
#include "luautils.h"

#include <luaapi/luadb.h>

#include <libpq-fe.h>

//SELECT pg_stat_get_backend_pid(s.backendid) AS procpid, pg_stat_get_backend_activity(s.backendid) AS current_query FROM (SELECT pg_stat_get_backend_idset() AS backendid) AS s;

static int db_connect(lua_State* l) {
	PGconn* conn = PQconnectStart("hostaddr=127.0.0.1 port=5432 dbname=test user=dbuser password=dbuser application_name=corelib");
	int fd = PQsocket(conn);

	st_netfd_t nfd = st_netfd_open_socket(fd);
	st_netfd_setspecific(nfd, conn, NULL);

	/* Wait until the socket becomes writable */
	if (st_netfd_poll(nfd, POLLOUT, -1) < 0)
		return 0;

	PostgresPollingStatusType status = 0;
	while (1) {
		status = PQconnectPoll(conn);
		switch (status) {
		case PGRES_POLLING_WRITING:
		case PGRES_POLLING_READING:
			continue;
		case PGRES_POLLING_FAILED:
			lua_pushinteger(l, -1);
			PQfinish(conn);
			return 1;
		case PGRES_POLLING_OK:
			lua_pushlightuserdata(l, nfd);
			return 1;
		default:
			lua_pushinteger(l, -1);
			return 1;
		}
	}
	return 1;
}

static int db_command(lua_State* l) {
	st_netfd_t nfd = lua_touserdata(l, 1);
	PGconn* conn = st_netfd_getspecific(nfd);

	int r = PQsendQuery(conn, "select * from tb");
	if (r == 0) {
		lua_pushinteger(l, -1);
		return 1;
	}

	/* Wait until the socket becomes readable */
	if (st_netfd_poll(nfd, POLLIN, -1) < 0) {
		lua_pushinteger(l, -1);
		return 1;
	}

//	do {
//		r = PQconsumeInput(conn);
//	} while (r == 0);
//
//	do {
//		r = PQisBusy(conn);
//	} while (r == 1);

	PGresult* res = PQgetResult(conn);

	ExecStatusType rst = PQresultStatus(res);
	if (rst == PGRES_BAD_RESPONSE || rst == PGRES_NONFATAL_ERROR || rst == PGRES_FATAL_ERROR) {
		lua_pushinteger(l, -1);
		return 1;
	}
//	int n = PQntuples(res);
	char* s = PQgetvalue(res, 1, 1);

	lua_pushstring(l, s);
	return 1;
}

static int db_redis_connect(lua_State* l) {
	redisContext* redc = redisConnectNonBlock("127.0.0.1", 6379);

	st_netfd_t nfd = st_netfd_open_socket(redc->fd);
	st_netfd_setspecific(nfd, redc, NULL);

	/* Wait until the socket becomes writable */
	if (st_netfd_poll(nfd, POLLOUT, -1) < 0)
		return 0;

	if (redc->err != 0) {
		redisFree(redc);
		lua_pushinteger(l, -1);
	} else {
		lua_pushlightuserdata(l, nfd);
	}
	return 1;
}

static int db_redis_command(lua_State* l) {
	st_netfd_t nfd = lua_touserdata(l, 1);
	redisContext* c = st_netfd_getspecific(nfd);

	int wdone = 0;
	redisReply *r = NULL;

	redisAppendCommand(c, "get key");

	/* Write */
	if (redisBufferWrite(c, &wdone) == REDIS_ERR)
		return REDIS_ERR;

	if (st_netfd_poll(nfd, POLLIN, -1) < 0)
		return 0;

	/* Read until there is a reply */
	do {
		if (redisBufferRead(c) == REDIS_ERR)
			return REDIS_ERR;
		if (redisGetReplyFromReader(c, &r) == REDIS_ERR)
			return REDIS_ERR;
	} while (r == NULL);

	lua_pushstring(l, r->str);
	return 1;
}



/////////////////////////////////////////////////////////////////////
static int ldb_pool_new(lua_State* l) {
	size_t len;
	const char* url = luaL_checklstring(l, 1, &len);	//url string
	int initconn = 0;
	if (lua_gettop(l) > 1) {
		initconn = luaL_checkinteger(l, 2);		// init conntion num in pool
	}
	uri_t uri = uri_new(url);
	dbpool_t pool = dbpool_new(uri, initconn);

	dbpool_start(pool);

	lua_pushlightuserdata(l, pool);
	return 1;
}

static int ldb_pool_free(lua_State* l) {
	dbpool_t pool = lua_touserdata(l, 1);
	dbpool_stop(pool);
	dbpool_free(pool);
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

//////////////////////////////  db conntion ////////////////////
static int ldb_conn_execute(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	size_t len;
	const char* sql = luaL_checklstring(l, 2, &len);

	TRY
		dbconn_execute(conn, sql);
	ELSE
		LOG_DEBUG("DB execute error: %s.(%s)", sql, dbconn_getLastError(conn));
	END_TRY
	return 0;
}

static int ldb_conn_executeQuery(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);
	size_t len;
	const char* sql = luaL_checklstring(l, 2, &len);

	dbrs_t rs = NULL;
	TRY
		rs = dbconn_executeQuery(conn, sql);
	ELSE
		LOG_DEBUG("DB execute error: %s.(%s)", sql, dbconn_getLastError(conn));
	END_TRY

	lua_pushlightuserdata(l, rs);
	return 1;
}

static int ldb_conn_close(lua_State* l) {
	dbconn_t conn = lua_touserdata(l, 1);

	TRY
		dbconn_close(conn);
	ELSE
		LOG_DEBUG("DB execute error: (%s)", dbconn_getLastError(conn));
	END_TRY

	return 0;
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
	char* name = dbrs_getColumnName(rs, col);
	lua_pushstring(l, name);
	return 1;
}

static int ldb_rs_getString(lua_State* l) {
	dbrs_t rs = lua_touserdata(l, 1);
	int col = luaL_checkinteger(l, 2);
	char* v = dbrs_getString(rs, col);
	lua_pushstring(l, v);
	return 1;
}


static const luaL_Reg funs[] = {
		{"newPool", ldb_pool_new},
		{"freePool", ldb_pool_free},
		{"getConntion", ldb_pool_getConn},
		{"active", ldb_pool_active},

		{"execute", ldb_conn_execute},
		{"executeQuery", ldb_conn_executeQuery},
		{"close", ldb_conn_close},

		{"next", ldb_rs_next},
		{"getColumnCount", ldb_rs_getColumnCount},
		{"getColumnName", ldb_rs_getColumnName},
		{"getString", ldb_rs_getString},
		{NULL, NULL}
};

LUA_API int luaopen_dblib(lua_State* l) {
	luaL_register(l, LUA_DB_LIB_NAME, funs);
	return 1;
}
