/*
 * dbconnection.c
 *
 *  Created on: Nov 24, 2014
 *      Author: blc
 */

#include <stdio.h>
#include <time.h>

#include <exception/sql_exception.h>
#include <vector.h>
#include <mem.h>
#include <timeutil.h>
#include <str.h>
#include <logger.h>

#include <st/st.h>

#include <db/dbconfig.h>
#include <db/dbpool.h>
#include <db/dbpst.h>
#include "dbconn_delegate.h"
#include <db/dbconn.h>

#ifdef HAVE_HIREDIS
extern const struct cop_t rediscops;
#endif
#ifdef HAVE_LIBMYSQLCLIENT
extern const struct cop_t mysqlcops;
#endif
#ifdef HAVE_LIBPQ
extern const struct cop_t postgresqlcops;
#endif
#ifdef HAVE_LIBSQLITE3
extern const struct cop_t sqlite3cops;
#endif
#ifdef HAVE_ORACLE
extern const struct cop_t oraclesqlcops;
#endif

static const struct cop_t *cops[] = {
#ifdef HAVE_HIREDIS
		&rediscops,
#endif
#ifdef HAVE_LIBMYSQLCLIENT
        &mysqlcops,
#endif
#ifdef HAVE_LIBPQ
        &postgresqlcops,
#endif
#ifdef HAVE_LIBSQLITE3
        &sqlite3cops,
#endif
#ifdef HAVE_ORACLE
        &oraclesqlcops,
#endif
        NULL
};

#define T dbconn_t
struct dbconn_s {
	cop_t op;
	uri_t url;
//	int maxRows;
	int timeout;
	int isAvailable;
	vector_t prepared;
	int isInTransaction;
	time_t lastAccessedTime;
	dbrs_t resultSet;
	dbconn_delegate_t D;
	st_netfd_t nfd;
	dbpool_t parent;
};

/* ------------------------------------------------------- Private methods */

static cop_t getOp(const char *protocol) {
	int i;
	for (i = 0; cops[i]; i++) {
		if (str_startsWith(protocol, cops[i]->name))
			return (cop_t) cops[i];
	}
	return NULL;
}

static int setDelegate(T C) {
	C->op = getOp(uri_getProtocol(C->url));
	if (!C->op) {
		LOG_WARN("database protocol '%s' not supported", uri_getProtocol(C->url));
		return 0;
	}
	return 1;
}

static void freePrepared(T C) {
	while (! vector_isEmpty(C->prepared)) {
//		PreparedStatement_T ps = Vector_pop(C->prepared);
//		PreparedStatement_free(&ps);
	}
}


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif


dbconn_t dbconn_new(void *pool) {
	dbconn_t C;
	assert(pool);
	NEW(C);
	C->parent = pool;
	C->isAvailable = 1;
	C->isInTransaction = 0;
	C->prepared = vector_new(4);
	C->timeout = SQL_DEFAULT_TIMEOUT;
	C->url = dbpool_getURL(pool);
	C->lastAccessedTime = time_now();
	if (!setDelegate(C)) {
		return NULL;
	}
	C->D = C->op->new(C->url);
	if (C->D == NULL) {
		dbconn_free(&C);
		return NULL;
	}

	//// st thread
	C->nfd = st_netfd_open_socket((C->op)->getsocket(C->D));
	/* Wait until the socket becomes writable */
	if (st_netfd_poll(C->nfd, POLLOUT, -1) < 0) {
		dbconn_free(&C);
		return NULL;
	}

	if (!(C->op)->connstate(C->D)) {
		dbconn_free(&C);
		return NULL;
	}

	(C->op)->onconn(C->D);

	return C;
}

void dbconn_free(T *C) {
	assert(C && *C);
	dbconn_clear((*C));
	vector_free(&(*C)->prepared);
	if ((*C)->D)
		(*C)->op->free(&(*C)->D);
	FREE(*C);
}


void dbconn_setAvailable(T C, int isAvailable) {
	assert(C);
	C->isAvailable = isAvailable;
	C->lastAccessedTime = time_now();
}

int dbconn_isAvailable(T C) {
	assert(C);
	return C->isAvailable;
}

time_t dbconn_getLastAccessedTime(T C) {
	assert(C);
	return C->lastAccessedTime;
}

int dbconn_isInTransaction(T C) {
	assert(C);
	return (C->isInTransaction > 0);
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


/* ------------------------------------------------------------ Properties */

void dbconn_setQueryTimeout(T C, int ms) {
	assert(C);
	assert(ms >= 0);
	C->timeout = ms;
//	C->op->setQueryTimeout(C->D, ms);
	//// 超时改用 st 库的超时
}

int dbconn_getQueryTimeout(T C) {
	assert(C);
	return C->timeout;
}


//void dbconn_setMaxRows(T C, int max) {
//	assert(C);
//	C->maxRows = max;
//	C->op->setMaxRows(C->D, max);
//}
//
//
//int dbconn_getMaxRows(T C) {
//	assert(C);
//	return C->maxRows;
//}

uri_t dbconn_getURL(T C) {
	assert(C);
	return C->url;
}

/* -------------------------------------------------------- Public methods */

int dbconn_ping(T C) {
	assert(C);
	return C->op->ping(C->D);
}


void dbconn_clear(T C) {
	assert(C);
	if (C->resultSet)
		dbrs_free(&C->resultSet);
//	if (C->maxRows)
//		dbconn_setMaxRows(C, 0);
	if (C->timeout != SQL_DEFAULT_TIMEOUT)
		dbconn_setQueryTimeout(C, SQL_DEFAULT_TIMEOUT);
	freePrepared(C);
}

void dbconn_close(T C) {
	assert(C);
	dbpool_returnConn(C->parent, C);
}

void dbconn_beginTransaction(T C) {
	assert(C);
	if (!C->op->beginTransaction(C->D))
		THROW(sql_exception, "%s", dbconn_getLastError(C));
	C->isInTransaction++;
}

void dbconn_commit(T C) {
	assert(C);
	if (C->isInTransaction)
		C->isInTransaction = 0;
	// Even if we are not in a transaction, call the delegate anyway and propagate any errors
	if (!C->op->commit(C->D))
		THROW(sql_exception, "%s", dbconn_getLastError(C));
}


void dbconn_rollback(T C) {
	assert(C);
	if (C->isInTransaction) {
		// Clear any pending resultset statements first
		dbconn_clear(C);
		C->isInTransaction = 0;
	}
	// Even if we are not in a transaction, call the delegate anyway and propagate any errors
	if (!C->op->rollback(C->D))
		THROW(sql_exception, "%s", dbconn_getLastError(C));
}


//long long dbconn_lastRowId(T C) {
//        assert(C);
//        return C->op->lastRowId(C->D);
//}
//
//
//long long dbconn_rowsChanged(T C) {
//        assert(C);
//        return C->op->rowsChanged(C->D);
//}

void dbconn_execute(T C, const char *sql, ...) {
	assert(C);
	assert(sql);
	if (C->resultSet)
		dbrs_free(&C->resultSet);
	va_list ap;
	va_start(ap, sql);
	int success = C->op->execute(C->D, sql, ap);
	va_end(ap);
	if (!success)
		THROW(sql_exception, "%s", dbconn_getLastError(C));

	if (st_netfd_poll(C->nfd, POLLIN, C->timeout) < 0)
		THROW(sql_exception, "%s", dbconn_getLastError(C));

	C->resultSet = C->op->getrs(C->D);

	if (!C->resultSet)
		THROW(sql_exception, "%s", dbconn_getLastError(C));
}

dbrs_t dbconn_executeQuery(T C, const char *sql, ...) {
	assert(C);
	assert(sql);
	if (C->resultSet)
		dbrs_free(&C->resultSet);
	va_list ap;
	va_start(ap, sql);
	int success = C->op->execute(C->D, sql, ap);
	va_end(ap);
	if (!success)
		THROW(sql_exception, "%s", dbconn_getLastError(C));

	if (st_netfd_poll(C->nfd, POLLIN, C->timeout) < 0)
		THROW(sql_exception, "%s", dbconn_getLastError(C));

	C->resultSet = C->op->getrs(C->D);

	if (!C->resultSet)
		THROW(sql_exception, "%s", dbconn_getLastError(C));
	return C->resultSet;
}

dbpst_t dbconn_prepareStatement(T C, const char *sql, ...) {
	assert(C);
	assert(sql);
	va_list ap;
	va_start(ap, sql);
	dbpst_t p = C->op->prepareStatement(C->D, sql, ap);
	va_end(ap);
	if (p)
		vector_push(C->prepared, p);
	else
		THROW(sql_exception, "%s", dbconn_getLastError(C));
	return p;
}

const char *dbconn_getLastError(T C) {
	assert(C);
	const char *s = C->op->getDbLastError(C->D);
	return STR_DEF(s) ? s : "?";
}


int dbconn_isSupported(const char *url) {
	return (url ? (getOp(url) != NULL) : 0);
}
