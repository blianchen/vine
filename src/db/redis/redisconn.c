/*
 * redisconn.c
 *
 *  Created on: Nov 28, 2014
 *      Author: blc
 */

#include <stdio.h>

#include <exception/sql_exception.h>
#include <mem.h>
#include <uri.h>

#include <db/dbconfig.h>
#include <db/dbrs.h>
#include <db/dbpst.h>
#include "../dbconn_delegate.h"
#include "redisrs.h"
#include "redisconn.h"

#include <hiredis/hiredis.h>

const struct cop_t rediscops = {
        .name = "redis",
		.onconn = redisconn_onconn,
		.onstop = redisconn_onstop,
        .new = redisconn_new,
		.connstate = redisconn_connstate,
        .free = redisconn_free,
		.getsocket = redisconn_getsocket,
//        redisconn_setQueryTimeout,
//        redisconn_setMaxRows,
        .ping = redisconn_ping,
        .beginTransaction = redisconn_beginTransaction,
        .commit = redisconn_commit,
        .rollback = redisconn_rollback,
//        redisconn_lastRowId,
//        redisconn_rowsChanged,
        .execute = redisconn_execute,
//        .executeQuery = redisconn_executeQuery,
		.getrs = redisconn_getrs,
        .prepareStatement = redisconn_prepareStatement,
        .getDbLastError = redisconn_getLastError
};

#define T dbconn_delegate_t
struct T {
	uri_t url;
	redisContext *db;
	redisReply *res;
//	int maxRows;
	int timeout;
//	ExecStatusType lastError;
//	StringBuffer_T sb;
};

//static uint32_t statementid = 0;
extern const struct rop_t redisrops;
//extern const struct pop_t redispops;


/* ------------------------------------------------------- Private methods */

static int doConnect(T C) {
	const char* host = uri_getHost(C->url);
	if (!host) {
		LOG_DEBUG("no host specified in URL");
		return 0;
	}
	int port = uri_getPort(C->url);
	if (!port) {
		port = REDIS_DEFAULT_PORT;
	}
	C->db = redisConnectNonBlock(host, port);

	return 1;
}

/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

void redisconn_onconn(T C) {
	//AUTH password
	//SELECT index


	// process login to dbanme
	/* User */
//	if (uri_getUser(C->url))
//		StringBuffer_append(C->sb, "user='%s' ", uri_getUser(C->url));
//	else if (uri_getParameter(C->url, "user"))
//		StringBuffer_append(C->sb, "user='%s' ", uri_getParameter(C->url, "user"));
//	else
//		ERROR("no username specified in URL");
//
//	/* Password */
//	if (uri_getPassword(C->url))
//		StringBuffer_append(C->sb, "password='%s' ", uri_getPassword(C->url));
//	else if (uri_getParameter(C->url, "password"))
//		StringBuffer_append(C->sb, "password='%s' ", uri_getParameter(C->url, "password"));
//	else
//		ERROR("no password specified in URL");
//
//	/* Database name */
//	if (uri_getPath(C->url))
//		StringBuffer_append(C->sb, "dbname='%s' ", uri_getPath(C->url) + 1);
//	else
//		ERROR("no database specified in URL");

}

void redisconn_onstop(T C) {

}

T redisconn_new(uri_t url) {
	T C;
	assert(url);
	NEW(C);
	C->url = url;
//        C->sb = StringBuffer_create(STRLEN);
	C->timeout = SQL_DEFAULT_TIMEOUT;
	if (!doConnect(C))
		redisconn_free(&C);
	return C;
}

int redisconn_connstate(T C) {
	if ((C->db)->err != 0) {
		redisFree(C->db);
		return 0;
	}
	return 1;
}

void redisconn_free(T *C) {
	assert(C && *C);
	if ((*C)->res)
		freeReplyObject((*C)->res);
	if ((*C)->db)
		redisFree((*C)->db);
//        StringBuffer_free(&(*C)->sb);
	FREE(*C);
}

int redisconn_getsocket(T C) {
	return (C->db)->fd;
}


//void redisconn_setQueryTimeout(T C, int ms) {
//	assert(C);
//	C->timeout = ms;
//}


//void redisconn_setMaxRows(T C, int max) {
//	assert(C);
//        C->maxRows = max;
//}


int redisconn_ping(T C) {
	assert(C);
//	return (PQstatus(C->db) == CONNECTION_OK);
	return 1;
}


int redisconn_beginTransaction(T C) {
	assert(C);
	return redisAppendCommand(C->db, "MULTI");
}


int redisconn_commit(T C) {
	assert(C);
	return redisconn_execute(C, "EXEC", NULL);
}

int redisconn_rollback(T C) {
	assert(C);
	return redisconn_execute(C, "DISCARD", NULL);
}


//long long redisconn_lastRowId(T C) {
//        assert(C);
//        return (long long)PQoidValue(C->res);
//}
//
//
//long long redisconn_rowsChanged(T C) {
//        assert(C);
//        char *changes = PQcmdTuples(C->res);
//        return changes ? Str_parseLLong(changes) : 0;
//}


dbrs_t redisconn_getrs(T C) {
	/* Read until there is a reply */
	do {
		if (redisBufferRead(C->db) == REDIS_ERR)
			return NULL;
		if (redisGetReplyFromReader(C->db, &(C->res)) == REDIS_ERR)
			return NULL;
	} while (C->res == NULL);
	if ((C->res)->type != REDIS_REPLY_ERROR) {
		return NULL;
	}
	dbrs_t dbrs = dbrs_new(redisrs_new(C->res), (rop_t) &redisrops);
	return dbrs;
}

int redisconn_execute(T C, const char *sql, va_list ap) {
	assert(C);
	if (C->res) {
		freeReplyObject(C->res);
		C->res = NULL;
	}
	if (ap) {
		redisvAppendCommand(C->db, sql, ap);
	} else {
		redisAppendCommand(C->db, sql);
	}

	int wdone = 0;
	/* Write until done */
	do {
		if (redisBufferWrite(C->db, &wdone) == REDIS_ERR)
			return REDIS_ERR;
	} while (!wdone);

	return wdone;
}


//dbrs_t redisconn_executeQuery(T C, const char *sql, va_list ap) {
//	va_list ap_copy;
//	assert(C);
//	freeReplyObject((*C)->res);
////	PQclear(C->res);
////	va_copy(ap_copy, ap);
////	StringBuffer_vset(C->sb, sql, ap_copy);
////	va_end(ap_copy);
////	C->res = PQexec(C->db, StringBuffer_toString(C->sb));
//////	C->lastError = PQresultStatus(C->res);
////	if (C->lastError == PGRES_TUPLES_OK)
////		return ResultSet_new(redisconn_new(C->res), (rop_t) & redisrops);
//	return NULL;
//}

dbpst_t redisconn_prepareStatement(T C, const char *sql, va_list ap) {
	LOG_WARN("redis not support prepareStatement");
	return NULL;
}


const char *redisconn_getLastError(T C) {
	assert(C);
	if (C->res && C->res->type == REDIS_REPLY_ERROR) {
		return C->res->str;
	} else {
		return "unknown error";
	}
}

/* Postgres client library finalization */
//void  redisconn_onstop(void) {
//        // Not needed, redisconn_free handle finalization
//}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

