/*
 * redisrs.c
 *
 *  Created on: Nov 28, 2014
 *      Author: blc
 */

#include <stdio.h>
#include <assert.h>

#include <logger.h>
#include <mem.h>
#include <timeutil.h>

#include "../dbrs_delegate.h"
#include "redisrs.h"


const struct rop_t redisrops = {
	.name = "redis",
	.free = redisrs_free,
	.getColumnCount = redisrs_getColumnCount,
	.getColumnName = redisrs_getColumnName,
	.getColumnSize = redisrs_getColumnSize,
	.next = redisrs_next,
	.isnull = redisrs_isnull,
	.getString = redisrs_getString,
	.getBlob = redisrs_getBlob,
	.getTimestamp = NULL, // getTimestamp is handled in ResultSet
	.getDateTime = NULL  // getDateTime is handled in ResultSet
};

#define T dbresultset_delegate_t
struct T {
//        int maxRows;
        int currentRow;
        int columnCount;
        int rowCount;
        redisReply *res;
};

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T redisrs_new(redisReply *res) {
	T R;
	assert(res);
	NEW(R);
	R->res = res;
//	R->maxRows = maxRows;
	R->currentRow = -1;
	R->columnCount = 1;		//redis always 1
	if ((R->res)->type == REDIS_REPLY_ARRAY) {
		R->rowCount = (R->res)->elements;
	} else {
		R->rowCount = 1;
	}
	return R;
}

void redisrs_free(T *R) {
	assert(R && *R);
	FREE(*R);
}

int redisrs_getColumnCount(T R) {
	assert(R);
	return R->columnCount;
}

const char *redisrs_getColumnName(T R, int columnIndex) {
	assert(R);
	LOG_DEBUG("redis not implement.");
	return NULL;
}

long redisrs_getColumnSize(T R, int columnIndex) {
	// always return 1 column
	if ((R->res)->type == REDIS_REPLY_ARRAY) {
		return R->res->elements;
	} else if ((R->res)->type == REDIS_REPLY_STRING) {
		return R->res->len;
	} else if ((R->res)->type == REDIS_REPLY_INTEGER) {
		return sizeof((R->res)->integer);
	} else {
		return 1;
	}
}

int redisrs_next(T R) {
	assert(R);
	return (!(R->currentRow++ >= (R->rowCount - 1)));
}

int redisrs_isnull(T R, int columnIndex) {
	assert(R);
	int ir = 0;
	if (R->res->type == REDIS_REPLY_NIL) {
		ir = 1;
	} else if (R->res->type == REDIS_REPLY_ARRAY) {
		if (R->res->element[R->currentRow]->type == REDIS_REPLY_NIL) {
			ir = 1;
		}
	}
	return ir;
}

//static char* _getValue(redisReply* r) {
//	if (r->type == REDIS_REPLY_STRING) {
//		return r->str;
//	}
//}

const char *redisrs_getString(T R, int columnIndex) {
	assert(R);
	if ((R->res)->type == REDIS_REPLY_STRING) {
		return (R->res)->str;
	}
	else if ((R->res)->type == REDIS_REPLY_ARRAY) {
		return (R->res)->element[R->currentRow]->str;
	}
	return NULL;
}


/*
 * As a "hack" to avoid extra allocation and complications by using PQunescapeBytea()
 * we instead unescape the buffer retrieved via PQgetvalue 'in-place'. This should
 * be safe as unescape will only modify internal bytes in the buffer and not change
 * the buffer pointer. See also unescape_bytea() above.
 */
const void *redisrs_getBlob(T R, int columnIndex, int *size) {
//        assert(R);
//        int i = checkAndSetColumnIndex(columnIndex, R->columnCount);
//        if (PQgetisnull(R->res, R->currentRow, i))
//                return NULL;
//        return unescape_bytea((uchar_t*)PQgetvalue(R->res, R->currentRow, i), PQgetlength(R->res, R->currentRow, i), size);
	return NULL;
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif
