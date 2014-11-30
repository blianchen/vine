/*
 * redisrs.h
 *
 *  Created on: Nov 28, 2014
 *      Author: blc
 */

#ifndef DB_REDIS_REDISRS_H_
#define DB_REDIS_REDISRS_H_

#include <hiredis/hiredis.h>

#define T dbresultset_delegate_t
T redisrs_new(redisReply *stmts);
void redisrs_free(T *R);
int redisrs_getColumnCount(T R);
const char *redisrs_getColumnName(T R, int columnIndex);
long redisrs_getColumnSize(T R, int columnIndex);
int redisrs_next(T R);
int redisrs_isnull(T R, int columnIndex);
const char *redisrs_getString(T R, int columnIndex);
const void *redisrs_getBlob(T R, int columnIndex, int *size);
#undef T

#endif /* DB_REDIS_REDISRS_H_ */
