/*
 * redisconn.c
 *
 *  Created on: Nov 28, 2014
 *      Author: blc
 */

#include <stdio.h>

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
//        redisconn_setMaxRows,
        .ping = redisconn_ping,
        .beginTransaction = redisconn_beginTransaction,
        .commit = redisconn_commit,
        .rollback = redisconn_rollback,
//        redisconn_lastRowId,
//        redisconn_rowsChanged,
        .execute = redisconn_execute,
		.getrs = redisconn_getrs,
        .prepareStatement = redisconn_prepareStatement,
        .getDbLastError = redisconn_getLastError
};

#define T dbconn_delegate_t
struct T {
	uri_t url;
	redisContext *db;
	redisReply *res;
	int timeout;
};

extern const struct rop_t redisrops;


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
	if ((C->db)->err != 0) {
		LOG_DEBUG("connect to redis error: url=%s, errno=%d, errstr=%s.", C->url, (C->db)->err, (C->db)->errstr);
		redisFree(C->db);
		return 0;
	}

	C->db->flags &= ~REDIS_CONNECTED;

	return 1;
}

/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

void redisconn_onconn(T C) {
	//AUTH password
	//SELECT index
}

void redisconn_onstop(T C) {

}

T redisconn_new(uri_t url) {
	T C;
	assert(url);
	NEW(C);
	C->url = url;
	C->timeout = SQL_DEFAULT_TIMEOUT;
	if (!doConnect(C))
		redisconn_free(&C);
	return C;
}

int redisconn_connstate(T C) {
	if ((C->db)->err != 0) {
		LOG_DEBUG("connect to redis error: url=%s, errno=%d, errstr=%s.", C->url, (C->db)->err, (C->db)->errstr);
		return 0;
	}
	C->db->flags |= REDIS_CONNECTED;
	return 1;
}

void redisconn_free(T *C) {
	assert(C && *C);
	if ((*C)->res)
		freeReplyObject((*C)->res);
	if ((*C)->db)
		redisFree((*C)->db);
	FREE(*C);
}

int redisconn_getsocket(T C) {
	return (C->db)->fd;
}

int redisconn_ping(T C) {
	assert(C);
	//TODO redisconn_ping
	return 1;
}

int redisconn_beginTransaction(T C) {
	assert(C);
	return redisconn_execute(C, "MULTI");
}

int redisconn_commit(T C) {
	assert(C);
	return redisconn_execute(C, "EXEC");
}

int redisconn_rollback(T C) {
	assert(C);
	return redisconn_execute(C, "DISCARD");
}

dbrs_t redisconn_getrs(T C) {
	/* Read until there is a reply */
	do {
		if (redisBufferRead(C->db) == REDIS_ERR)
			return NULL;
		if (redisGetReplyFromReader(C->db, (void **)&(C->res)) == REDIS_ERR)
			return NULL;
	} while (C->res == NULL);
	if ((C->res)->type == REDIS_REPLY_ERROR) {
		return NULL;
	}
	dbrs_t dbrs = dbrs_new(redisrs_new(C->res), (rop_t) &redisrops);
	return dbrs;
}

int redisconn_execute(T C, const char *sql) {
	assert(C);
	if (C->res) {
		freeReplyObject(C->res);
		C->res = NULL;
	}
	redisAppendCommand(C->db, sql);

	int wdone = 0;
	/* Write until done */
	do {
		if (redisBufferWrite(C->db, &wdone) == REDIS_ERR)
			return REDIS_ERR;
	} while (!wdone);

	return wdone;
}

dbpst_t redisconn_prepareStatement(T C, const char *sql) {
	LOG_WARN("redis not support prepareStatement.");
	return NULL;
}

const char *redisconn_getLastError(T C) {
	assert(C);
	if (C->res && C->res->type == REDIS_REPLY_ERROR) {
		return C->res->str;
	} else if (C->db->err != 0) {
		return C->db->errstr;
	} else {
		return "unknown error";
	}
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

