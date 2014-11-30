/*
 * redisconn.h
 *
 *  Created on: Nov 28, 2014
 *      Author: blc
 */

#ifndef DB_REDIS_REDISCONN_H_
#define DB_REDIS_REDISCONN_H_

#define T dbconn_delegate_t
T redisconn_new(uri_t url);
int redisconn_connstate(T C);
void redisconn_free(T *C);
int redisconn_getsocket(T C);
//void redisconn_setQueryTimeout(T C, int ms);
//void redisconn_setMaxRows(T C, int max);
int redisconn_ping(T C);
int redisconn_beginTransaction(T C);
int redisconn_commit(T C);
int redisconn_rollback(T C);
//long long redisconn_lastRowId(T C);
//long long redisconn_rowsChanged(T C);
int redisconn_execute(T C, const char *sql, va_list ap);
dbrs_t redisconn_getrs(T C);
//dbrs_t redisconn_executeQuery(T C, const char *sql, va_list ap);
dbpst_t redisconn_prepareStatement(T C, const char *sql, va_list ap);
const char *redisconn_getLastError(T C);
/* Event handlers */
void  redisconn_onconn(T C);
void  redisconn_onstop(T C);
#undef T

#endif /* DB_REDIS_REDISCONN_H_ */
