
#include <stdio.h>

#include <mem.h>
#include <exception/sql_exception.h>

#include <db/dbconfig.h>
#include <db/dbrs.h>
#include <db/dbpst.h>

/**
 * Implementation of the PreparedStatement interface 
 */


/* ----------------------------------------------------------- Definitions */

#define T dbpst_t
struct dbpst_S {
	pop_t op;
	int parameterCount;
	dbrs_t resultSet;
	dbpst_delegate_t D;
};


/* ------------------------------------------------------- Private methods */

static void clearResultSet(T P) {
	if (P->resultSet)
		dbrs_free(&P->resultSet);
}

/* ----------------------------------------------------- Protected methods */

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T dbpst_new(dbpst_delegate_t D, pop_t op, int parameterCount) {
	T P;
	assert(D);
	assert(op);
	NEW(P);
	P->D = D;
	P->op = op;
	P->parameterCount = parameterCount;
	return P;
}


void dbpst_free(T *P) {
	assert(P && *P);
	clearResultSet((*P));
	(*P)->op->free(&(*P)->D);
	FREE(*P);
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif


/* ------------------------------------------------------------ Parameters */

void dbpst_setString(T P, int parameterIndex, const char *x) {
	assert(P);
	P->op->setString(P->D, parameterIndex, x);
}

void dbpst_setInt(T P, int parameterIndex, int x) {
	assert(P);
	P->op->setInt(P->D, parameterIndex, x);
}

void dbpst_setLLong(T P, int parameterIndex, long long x) {
	assert(P);
	P->op->setLLong(P->D, parameterIndex, x);
}

void dbpst_setDouble(T P, int parameterIndex, double x) {
	assert(P);
	P->op->setDouble(P->D, parameterIndex, x);
}

void dbpst_setBlob(T P, int parameterIndex, const void *x, int size) {
	assert(P);
	P->op->setBlob(P->D, parameterIndex, x, size);
}

void dbpst_setTimestamp(T P, int parameterIndex, time_t x) {
	assert(P);
	P->op->setTimestamp(P->D, parameterIndex, x);
}


/* -------------------------------------------------------- Public methods */


void dbpst_execute(T P) {
	assert(P);
	clearResultSet(P);
	P->op->execute(P->D);
}

dbrs_t dbpst_executeQuery(T P) {
	assert(P);
	clearResultSet(P);
	P->resultSet = P->op->executeQuery(P->D);
	if (!P->resultSet)
		THROW(sql_exception, "dbpst_executeQuery");
	return P->resultSet;
}

long long dbpst_rowsChanged(T P) {
	assert(P);
	return P->op->rowsChanged(P->D);
}


/* ------------------------------------------------------------ Properties */


int dbpst_getParameterCount(T P) {
	assert(P);
	return P->parameterCount;
}
