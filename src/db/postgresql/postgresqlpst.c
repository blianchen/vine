

#include <db/dbconfig.h>

#include <stdio.h>
#include <string.h>
#include <libpq-fe.h>

#include <mem.h>
#include <exception/sql_exception.h>
#include <timeutil.h>
#include <str.h>

//#include "system/Time.h"
#include <db/dbpst.h>
#include <db/dbrs.h>
#include "../dbpst_delegate.h"
#include "postgresqlrs.h"
#include "postgresqlpst.h"


/**
 * Implementation of the PreparedStatement/Delegate interface for postgresql.
 * All parameter values are sent as text except for blobs. Postgres ignore
 * paramLengths for text parameters and it is therefor set to 0, except for blob.
 */

/* ----------------------------------------------------------- Definitions */

const struct pop_t postgresqlpops = {
        "postgresql",
        postgresqlpst_free,
        postgresqlpst_setString,
        postgresqlpst_setInt,
        postgresqlpst_setLLong,
        postgresqlpst_setDouble,
        postgresqlpst_setTimestamp,
        postgresqlpst_setBlob,
        postgresqlpst_execute,
//        postgresqlpst_executeQuery,
		postgresqlpst_getrs,
        postgresqlpst_rowsChanged
};

typedef struct param_t {
        char s[65];
} *param_t;
#define T dbpst_delegate_t
struct T {
	int maxRows;
//        int lastError;
	char *stmt;
	PGconn *db;
	PGresult *res;
	int paramCount;
	char **paramValues;
	int *paramLengths;
	int *paramFormats;
	param_t params;
};

extern const struct Rop_T postgresqlrops;


/* ----------------------------------------------------- Protected methods */

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T postgresqlpst_new(PGconn *db, int maxRows, char *stmt, int paramCount) {
	T P;
	assert(db);
	assert(stmt);
	NEW(P);
	P->db = db;
	P->stmt = stmt;
	P->maxRows = maxRows;
	P->paramCount = paramCount;
//	P->lastError = PGRES_COMMAND_OK;
	if (P->paramCount) {
		P->paramValues = CALLOC(P->paramCount, sizeof(char *));
		P->paramLengths = CALLOC(P->paramCount, sizeof(int));
		P->paramFormats = CALLOC(P->paramCount, sizeof(int));
		P->params = CALLOC(P->paramCount, sizeof(struct param_t));
	}
	return P;
}


void postgresqlpst_free(T *P) {
	char stmt[STRLEN];
	assert(P && *P);
	/* NOTE: there is no C API function for explicit statement
	 * deallocation (postgres-8.1.x) - the DEALLOCATE statement
	 * has to be used. The postgres documentation mentiones such
	 * function as a possible future extension */
	snprintf(stmt, STRLEN, "DEALLOCATE \"%s\";", (*P)->stmt);
	PQclear(PQexec((*P)->db, stmt));
	PQclear((*P)->res);
	FREE((*P)->stmt);
	if ((*P)->paramCount) {
		FREE((*P)->paramValues);
		FREE((*P)->paramLengths);
		FREE((*P)->paramFormats);
		FREE((*P)->params);
	}
	FREE(*P);
}

void postgresqlpst_setString(T P, int parameterIndex, const char *x) {
	assert(P);
	int i = checkAndSetParameterIndex(parameterIndex, P->paramCount);
	P->paramValues[i] = (char *) x;
	P->paramLengths[i] = 0;
	P->paramFormats[i] = 0;
}

void postgresqlpst_setInt(T P, int parameterIndex, int x) {
	assert(P);
	int i = checkAndSetParameterIndex(parameterIndex, P->paramCount);
	snprintf(P->params[i].s, 64, "%d", x);
	P->paramValues[i] = P->params[i].s;
	P->paramLengths[i] = 0;
	P->paramFormats[i] = 0;
}

void postgresqlpst_setLLong(T P, int parameterIndex, long long x) {
	assert(P);
	int i = checkAndSetParameterIndex(parameterIndex, P->paramCount);
	snprintf(P->params[i].s, 64, "%lld", x);
	P->paramValues[i] = P->params[i].s;
	P->paramLengths[i] = 0;
	P->paramFormats[i] = 0;
}

void postgresqlpst_setDouble(T P, int parameterIndex, double x) {
	assert(P);
	int i = checkAndSetParameterIndex(parameterIndex, P->paramCount);
	snprintf(P->params[i].s, 64, "%lf", x);
	P->paramValues[i] = P->params[i].s;
	P->paramLengths[i] = 0;
	P->paramFormats[i] = 0;
}

void postgresqlpst_setTimestamp(T P, int parameterIndex, time_t x) {
	assert(P);
	int i = checkAndSetParameterIndex(parameterIndex, P->paramCount);
	P->paramValues[i] = time_toString(x, P->params[i].s);
	P->paramLengths[i] = 0;
	P->paramFormats[i] = 0;
}

void postgresqlpst_setBlob(T P, int parameterIndex, const void *x, int size) {
	assert(P);
	int i = checkAndSetParameterIndex(parameterIndex, P->paramCount);
	P->paramValues[i] = (char *) x;
	P->paramLengths[i] = (x) ? size : 0;
	P->paramFormats[i] = 1;
}

int postgresqlpst_execute(T P) {
	assert(P);
	PQclear(P->res);
	return PQsendQueryPrepared(P->db, P->stmt, P->paramCount, (const char **) P->paramValues, P->paramLengths, P->paramFormats, 0);
//	P->lastError = P->res ? PQresultStatus(P->res) : PGRES_FATAL_ERROR;
//	if (P->lastError != PGRES_COMMAND_OK)
//		THROW(sql_exception, "%s", PQresultErrorMessage(P->res));
}

dbrs_t postgresqlpst_getrs(T P) {
	P->res = PQgetResult(P->db);
	if (!P->res) {
		return NULL;
	}
	ExecStatusType re = PQresultStatus(P->res);
//	P->lastError = re;
	if (re == PGRES_EMPTY_QUERY || re == PGRES_COMMAND_OK || re == PGRES_TUPLES_OK) {
		return dbrs_new(postgresqlrs_new(P->res, P->maxRows), (rop_t) &postgresqlrops);
	} else {
		return NULL;
	}
}

//dbrs_t postgresqlpst_executeQuery(T P) {
//	assert(P);
//	PQclear(P->res);
//	P->res = PQexecPrepared(P->db, P->stmt, P->paramCount, (const char **) P->paramValues, P->paramLengths,
//			P->paramFormats, 0);
//	P->lastError = P->res ? PQresultStatus(P->res) : PGRES_FATAL_ERROR;
//	if (P->lastError == PGRES_TUPLES_OK)
//		return dbrs_new(postgresqlrs_new(P->res, P->maxRows), (rop_t) &postgresqlrops);
//	THROW(sql_exception, "%s", PQresultErrorMessage(P->res));
//	return NULL;
//}

long long postgresqlpst_rowsChanged(T P) {
	assert(P);
	char *changes = PQcmdTuples(P->res);
	return changes ? str_parseLLong(changes) : 0;
}


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

