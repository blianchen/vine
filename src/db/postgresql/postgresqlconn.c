
#include <db/dbconfig.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <libpq-fe.h>

#include <exception/sql_exception.h>
#include <uri.h>
#include <mem.h>

#include <db/dbrs.h>
#include <db/dbpst.h>
#include "../StringBuffer.h"
#include "../dbconn_delegate.h"
#include "postgresqlpst.h"
#include "postgresqlrs.h"

#include "postgresqlconn.h"


/**
 * Implementation of the Connection/Delegate interface for postgresql. 
 */

/* ----------------------------------------------------------- Definitions */

const struct cop_t postgresqlcops = {
        .name = "postgresql",
		.onconn = postgresqlconn_onconn,
        .onstop = postgresqlconn_onstop,
        .new = postgresqlconn_new,
		.connstate = postgresqlconn_connstate,
        .free = postgresqlconn_free,
		.getsocket = postgresqlconn_getsocket,
//        postgresqlconn_setQueryTimeout,
//        postgresqlconn_setMaxRows,
        .ping = postgresqlconn_ping,
        .beginTransaction = postgresqlconn_beginTransaction,
        .commit = postgresqlconn_commit,
        .rollback = postgresqlconn_rollback,
//        postgresqlconn_lastRowId,
//        postgresqlconn_rowsChanged,
        .execute = postgresqlconn_execute,
//        postgresqlconn_executeQuery,
		.getrs = postgresqlconn_getrs,
        .prepareStatement = postgresqlconn_prepareStatement,
        .getDbLastError = postgresqlconn_getLastError
};

#define T dbconn_delegate_t
struct T {
	uri_t url;
	PGconn *db;
	PGresult *res;
	int maxRows;
//	int timeout;
	ExecStatusType lastError;
	StringBuffer_T sb;
};

static uint32_t statementid = 0;
extern const struct rop_t postgresqlrops;
extern const struct pop_t postgresqlpops;


/* ------------------------------------------------------- Private methods */

static int doConnect(T C) {
	/* User */
	if (uri_getUser(C->url))
		StringBuffer_append(C->sb, "user='%s' ", uri_getUser(C->url));
	else if (uri_getParameter(C->url, "user"))
		StringBuffer_append(C->sb, "user='%s' ", uri_getParameter(C->url, "user"));
	else
		THROW(sql_exception, "no username specified in URL");
	/* Password */
	if (uri_getPassword(C->url))
		StringBuffer_append(C->sb, "password='%s' ", uri_getPassword(C->url));
	else if (uri_getParameter(C->url, "password"))
		StringBuffer_append(C->sb, "password='%s' ", uri_getParameter(C->url, "password"));
	else
		THROW(sql_exception, "no password specified in URL");
	/* Host */
	if (uri_getParameter(C->url, "unix-socket")) {
		if (uri_getParameter(C->url, "unix-socket")[0] != '/')
			THROW(sql_exception, "invalid unix-socket directory");
		StringBuffer_append(C->sb, "host='%s' ", uri_getParameter(C->url, "unix-socket"));
	} else if (uri_getHost(C->url)) {
		StringBuffer_append(C->sb, "host='%s' ", uri_getHost(C->url));
		/* Port */
		if (uri_getPort(C->url) > 0)
			StringBuffer_append(C->sb, "port=%d ", uri_getPort(C->url));
		else
			THROW(sql_exception, "no port specified in URL");
	} else
		THROW(sql_exception, "no host specified in URL");
	/* Database name */
	if (uri_getPath(C->url))
		StringBuffer_append(C->sb, "dbname='%s' ", uri_getPath(C->url) + 1);
	else
		THROW(sql_exception, "no database specified in URL");
	/* Options */
	StringBuffer_append(C->sb, "sslmode='%s' ",
			str_isEqual(uri_getParameter(C->url, "use-ssl"), "true") ? "require" : "disable");
	if (uri_getParameter(C->url, "connect-timeout")) {
		TRY
			StringBuffer_append(C->sb, "connect_timeout=%d ",
			str_parseInt(uri_getParameter(C->url, "connect-timeout")));
		ELSE
			THROW(sql_exception, "invalid connect timeout value");
		END_TRY;
	} else
		StringBuffer_append(C->sb, "connect_timeout=%d ", SQL_DEFAULT_TCP_TIMEOUT);
	if (uri_getParameter(C->url, "application-name"))
		StringBuffer_append(C->sb, "application_name='%s' ", uri_getParameter(C->url, "application-name"));
	/* Connect */
//	C->db = PQconnectdb(StringBuffer_toString(C->sb));
	C->db = PQconnectStart(StringBuffer_toString(C->sb));
	if (PQstatus(C->db) == CONNECTION_OK)
		return 1;
	return 0;
}


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T postgresqlconn_new(uri_t url) {
	T C;
	assert(url);
	NEW(C);
	C->url = url;
	C->sb = StringBuffer_create(STRLEN);
//	C->timeout = SQL_DEFAULT_TIMEOUT;
	if (!doConnect(C))
		postgresqlconn_free(&C);
	return C;
}

void postgresqlconn_free(T *C) {
	assert(C && *C);
	if ((*C)->res)
		PQclear((*C)->res);
	if ((*C)->db)
		PQfinish((*C)->db);
	StringBuffer_free(&(*C)->sb);
	FREE(*C);
}

int postgresqlconn_connstate(T C) {
	PostgresPollingStatusType pt = PQconnectPoll(C->db);
	while (pt == PGRES_POLLING_READING || pt == PGRES_POLLING_WRITING) {
		pt = PQconnectPoll(C->db);
	}
	if (pt == PGRES_POLLING_OK) {
		return 1;
	}
	return 0;
}

int postgresqlconn_getsocket(T C) {
	return PQsocket(C->db);
}

//void postgresqlconn_setQueryTimeout(T C, int ms) {
//	assert(C);
//	C->timeout = ms;
//	StringBuffer_set(C->sb, "SET statement_timeout TO %d;", C->timeout);
//	PGresult *res = PQexec(C->db, StringBuffer_toString(C->sb));
//	PQclear(res);
//}
//
//void postgresqlconn_setMaxRows(T C, int max) {
//	assert(C);
//	C->maxRows = max;
//}

int postgresqlconn_ping(T C) {
	assert(C);
	return (PQstatus(C->db) == CONNECTION_OK);
}

int postgresqlconn_beginTransaction(T C) {
	assert(C);
	return PQsendQuery(C->db, "BEGIN TRANSACTION;");
}

int postgresqlconn_commit(T C) {
	assert(C);
	return PQsendQuery(C->db, "COMMIT TRANSACTION;");
}

int postgresqlconn_rollback(T C) {
	assert(C);
	return PQsendQuery(C->db, "ROLLBACK TRANSACTION;");
}

//long long postgresqlconn_lastRowId(T C) {
//	assert(C);
//	return (long long) PQoidValue(C->res);
//}
//
//long long postgresqlconn_rowsChanged(T C) {
//	assert(C);
//	char *changes = PQcmdTuples(C->res);
//	return changes ? str_parseLLong(changes) : 0;
//}

dbrs_t postgresqlconn_getrs(T C) {
	PQclear(C->res);
	C->res = PQgetResult(C->db);
	if (!C->res) {
		return NULL;
	}
	ExecStatusType re = PQresultStatus(C->res);
	C->lastError = re;
	if (re == PGRES_EMPTY_QUERY || re == PGRES_COMMAND_OK || re == PGRES_TUPLES_OK) {
		return dbrs_new(postgresqlrs_new(C->res, C->maxRows), (rop_t)&postgresqlrops);
	} else {
		return NULL;
	}
}

int postgresqlconn_execute(T C, const char *sql, va_list ap) {
	va_list ap_copy;
	assert(C);
	va_copy(ap_copy, ap);
	StringBuffer_vset(C->sb, sql, ap_copy);
	va_end(ap_copy);
	return PQsendQuery(C->db, StringBuffer_toString(C->sb));
}


//dbrs_t postgresqlconn_executeQuery(T C, const char *sql, va_list ap) {
//        va_list ap_copy;
//	assert(C);
//        PQclear(C->res);
//        va_copy(ap_copy, ap);
//        StringBuffer_vset(C->sb, sql, ap_copy);
//        va_end(ap_copy);
//        C->res = PQexec(C->db, StringBuffer_toString(C->sb));
//        C->lastError = PQresultStatus(C->res);
//        if (C->lastError == PGRES_TUPLES_OK)
//                return dbrs_new(postgresqlrs_new(C->res, C->maxRows), (rop_t)&postgresqlrops);
//        return NULL;
//}

dbpst_t postgresqlconn_prepareStatement(T C, const char *sql, va_list ap) {
	char *name;
	int paramCount = 0;
	va_list ap_copy;
	assert(C);
	assert(sql);
	PQclear(C->res);
	va_copy(ap_copy, ap);
	StringBuffer_vset(C->sb, sql, ap_copy);
	va_end(ap_copy);
	paramCount = StringBuffer_prepare4postgres(C->sb);
	uint32_t t = ++statementid; // increment is atomic
	name = str_cat("%d", t);
	int ir = PQsendPrepare(C->db, name, StringBuffer_toString(C->sb), 0, NULL);
//        C->lastError = C->res ? PQresultStatus(C->res) : PGRES_FATAL_ERROR;
//        if (C->lastError == PGRES_EMPTY_QUERY || C->lastError == PGRES_COMMAND_OK || C->lastError == PGRES_TUPLES_OK)
	if (ir)
		return dbpst_new(postgresqlpst_new(C->db, C->maxRows, name, paramCount), (pop_t) &postgresqlpops, paramCount);
	return NULL;
}

const char *postgresqlconn_getLastError(T C) {
	assert(C);
	return C->res ? PQresultErrorMessage(C->res) : "unknown error";
}

void  postgresqlconn_onconn(T C) {
        // Not needed, postgresqlconn_free handle finalization
}

/* Postgres client library finalization */
void  postgresqlconn_onstop(T C) {
        // Not needed, postgresqlconn_free handle finalization
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

