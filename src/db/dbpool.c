#include <stdio.h>
#include <string.h>

#include <logger.h>
#include <mem.h>
#include <vector.h>
#include <exception/sql_exception.h>

#include <st/st.h>

#include <db/dbconfig.h>
#include <db/dbconn.h>
#include <db/dbpool.h>

/**
 * Implementation of the ConnectionPool interface
 */

/* ----------------------------------------------------------- Definitions */

#define T dbpool_t
struct dbpool_s {
	uri_t url;
	int filled;
//	int doSweep;
//	char *error;
	vector_t pool;
//	int sweepInterval;
	int maxConnections;
	volatile int stopped;
	int sqlTimeout;
//	int connectionTimeout;
	int initialConnections;
};

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif
void (*AbortHandler)(const char *error) = NULL;
#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

/* ------------------------------------------------------- Private methods */

static void drainPool(T P) {
	while (!vector_isEmpty(P->pool)) {
		dbconn_t con = vector_pop(P->pool);
		dbconn_free(&con);
	}
}

static int fillPool(T P) {
	int i;
	for (i = 0; i < P->initialConnections; i++) {
		dbconn_t con = dbconn_new(P);
		if (!con) {
			if (i > 0) {
				LOG_DEBUG("Failed to fill the pool with initial connections.\n");
				return 1;
			}
			return 0;
		}
		dbconn_setQueryTimeout(con, P->sqlTimeout);
		vector_push(P->pool, con);
	}
	return 1;
}

static int getActive(T P) {
	int i, n = 0, size = vector_size(P->pool);
	for (i = 0; i < size; i++)
		if (!dbconn_isAvailable(vector_get(P->pool, i)))
			n++;
	return n;
}

/* ------------------------------------------------------------ Properties */

uri_t dbpool_getURL(T P) {
	assert(P);
	return P->url;
}

void dbpool_setInitialConn(T P, int connections) {
	assert(P);
	assert(connections >= 0);
	P->initialConnections = connections;
}

int dbpool_getInitialConn(T P) {
	assert(P);
	return P->initialConnections;
}

void dbpool_setMaxConn(T P, int maxConnections) {
	assert(P);
	assert(P->initialConnections <= maxConnections);
	P->maxConnections = maxConnections;
}

int dbpool_getMaxConn(T P) {
	assert(P);
	return P->maxConnections;
}

void dbpool_setSqlTimeout(T P, int sqlTimeout) {
	assert(P);
	assert(sqlTimeout > 0);
	P->sqlTimeout = sqlTimeout;
}

int dbpool_getSqlTimeout(T P) {
	assert(P);
	return P->sqlTimeout;
}

//void dbpool_setConnTimeout(T P, int connectionTimeout) {
//	assert(P);
//	assert(connectionTimeout > 0);
//	P->connectionTimeout = connectionTimeout;
//}
//
//int dbpool_getConnTimeout(T P) {
//	assert(P);
//	return P->connectionTimeout;
//}

int dbpool_size(T P) {
	assert(P);
	return vector_size(P->pool);
}

int dbpool_active(T P) {
	int n = 0;
	assert(P);
	n = getActive(P);
	return n;
}

/* -------------------------------------------------------- Public methods */
T dbpool_new(uri_t url) {
	T P;
	assert(url);
	NEW(P);
	P->url = url;
	P->maxConnections = SQL_DEFAULT_MAX_CONNECTIONS;
	P->pool = vector_new(SQL_DEFAULT_MAX_CONNECTIONS);
	P->initialConnections = SQL_DEFAULT_INIT_CONNECTIONS;
//	P->connectionTimeout = SQL_DEFAULT_CONNECTION_TIMEOUT;
	P->sqlTimeout = SQL_DEFAULT_TIMEOUT;
	return P;
}

void dbpool_free(T *P) {
	vector_t pool;
	assert(P && *P);
	pool = (*P)->pool;
	if (!(*P)->stopped)
		dbpool_stop((*P));
	vector_free(&pool);
//	FREE((*P)->error);
	FREE(*P);
}

void dbpool_start(T P) {
	assert(P);
	P->stopped = 0;
	if (!P->filled) {
		P->filled = fillPool(P);
	}
	if (!P->filled)
		THROW(sql_exception, "Failed to start connection pool.");
}

void dbpool_stop(T P) {
	assert(P);
	P->stopped = 1;
	if (P->filled) {
		drainPool(P);
		P->filled = 0;
//		dbconn_onstop(P);
	}
}

dbconn_t dbpool_getConn(T P) {
	dbconn_t con = NULL;
	assert(P);
	int i, size = vector_size(P->pool);
	for (i = 0; i < size; i++) {
		con = vector_get(P->pool, i);
		if (dbconn_isAvailable(con) && dbconn_ping(con)) {
			dbconn_setAvailable(con, 0);
			return con;
		}
	}
	con = NULL;
	if (size < P->maxConnections) {		//TODO 在多线程时这个判断不精确，在connect等待时size不会增加
		con = dbconn_new(P);
		if (con) {
			dbconn_setAvailable(con, 0);
			vector_push(P->pool, con);
			return con;
		} else {
			LOG_DEBUG("Failed to create db connection.(%s)", uri_toString(P->url));
			return NULL;
		}
	}

	// pool have full, wait 32ms
	st_usleep(32 * 1000);
	return dbpool_getConn(P);	// in gcc, Tail Recursion  must be use -O3
}

void dbpool_returnConn(T P, dbconn_t conn) {
	assert(P);
	assert(conn);
	if (dbconn_isInTransaction(conn)) {
		TRY
			dbconn_rollback(conn);
		ELSE
			LOG_DEBUG("Failed to rollback transaction -- %s\n", Exception_frame.message);
		END_TRY;
	}
	dbconn_clear(conn);
	dbconn_setAvailable(conn, 1);
}

const char *dbpool_version(void) {
	return "db-pool, ver="VERSION;
}
