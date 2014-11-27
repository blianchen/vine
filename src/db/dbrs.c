
#include <stdio.h>

#include <mem.h>
#include <str.h>
#include <exception/sql_exception.h>
#include <utils.h>

#include <db/dbconfig.h>
#include <db/dbrs.h>
#include "dbrs_delegate.h"

/**
 * Implementation of the ResultSet interface 
 */

/* ----------------------------------------------------------- Definitions */
#define T dbrs_t
struct resultset_s {
        rop_t op;
        dbresultset_delegate_t D;
};


/* ------------------------------------------------------- Private methods */
static inline int getIndex(T R, const char *name) {
	int i;
	int columns = dbrs_getColumnCount(R);
	for (i = 1; i <= columns; i++)
		if (str_isByteEqual(name, dbrs_getColumnName(R, i)))
			return i;
	THROW(sql_exception, "Invalid column name '%s'", name ? name : "null");
	return -1;
}



/* ----------------------------------------------------- Protected methods */
#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif

T dbrs_new(dbresultset_delegate_t D, rop_t op) {
	T R;
	assert(D);
	assert(op);
	NEW(R);
	R->D = D;
	R->op = op;
	return R;
}

void dbrs_free(T *R) {
	assert(R && *R);
	(*R)->op->free(&(*R)->D);
	FREE(*R);
}

#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif

/* ------------------------------------------------------------ Properties */

int dbrs_getColumnCount(T R) {
	assert(R);
	return R->op->getColumnCount(R->D);
}

const char *dbrs_getColumnName(T R, int columnIndex) {
	assert(R);
	return R->op->getColumnName(R->D, columnIndex);
}

long dbrs_getColumnSize(T R, int columnIndex) {
	assert(R);
	return R->op->getColumnSize(R->D, columnIndex);
}

/* -------------------------------------------------------- Public methods */

int dbrs_next(T R) {
	return R ? R->op->next(R->D) : 0;
}

int dbrs_isnull(T R, int columnIndex) {
	assert(R);
	return R->op->isnull(R->D, columnIndex);
}

/* --------------------------------------------------------------- Columns */

const char *dbrs_getString(T R, int columnIndex) {
	assert(R);
	return R->op->getString(R->D, columnIndex);
}

const char *dbrs_getStringByName(T R, const char *columnName) {
	assert(R);
	return dbrs_getString(R, getIndex(R, columnName));
}

int dbrs_getInt(T R, int columnIndex) {
	assert(R);
	const char *s = R->op->getString(R->D, columnIndex);
	return s ? str_parseInt(s) : 0;
}

int dbrs_getIntByName(T R, const char *columnName) {
	assert(R);
	return dbrs_getInt(R, getIndex(R, columnName));
}

long long dbrs_getLLong(T R, int columnIndex) {
	assert(R);
	const char *s = R->op->getString(R->D, columnIndex);
	return s ? str_parseLLong(s) : 0;
}

long long dbrs_getLLongByName(T R, const char *columnName) {
	assert(R);
	return dbrs_getLLong(R, getIndex(R, columnName));
}

double dbrs_getDouble(T R, int columnIndex) {
	assert(R);
	const char *s = R->op->getString(R->D, columnIndex);
	return s ? str_parseDouble(s) : 0.0;
}

double dbrs_getDoubleByName(T R, const char *columnName) {
	assert(R);
	return dbrs_getDouble(R, getIndex(R, columnName));
}

const void *dbrs_getBlob(T R, int columnIndex, int *size) {
	assert(R);
	const void *b = R->op->getBlob(R->D, columnIndex, size);
	if (!b)
		*size = 0;
	return b;
}

const void *dbrs_getBlobByName(T R, const char *columnName, int *size) {
	assert(R);
	return dbrs_getBlob(R, getIndex(R, columnName), size);
}


/* --------------------------------------------------------- Date and Time */

//time_t dbrs_getTimestamp(T R, int columnIndex) {
//	assert(R);
//	time_t t = 0;
//	if (R->op->getTimestamp) {
//		t = R->op->getTimestamp(R->D, columnIndex);
//	} else {
//		const char *s = dbrs_getString(R, columnIndex);
//		if (s && *(s))
//			t = Time_toTimestamp(s);
//	}
//	return t;
//}
//
//time_t dbrs_getTimestampByName(T R, const char *columnName) {
//	assert(R);
//	return dbrs_getTimestamp(R, getIndex(R, columnName));
//}
//
//struct tm dbrs_getDateTime(T R, int columnIndex) {
//	assert(R);
//	struct tm t = { .tm_year = 0 };
//	if (R->op->getDateTime) {
//		R->op->getDateTime(R->D, columnIndex, &t);
//	} else {
//		const char *s = dbrs_getString(R, columnIndex);
//		if (s && *(s))
//			Time_toDateTime(s, &t);
//	}
//	return t;
//}
//
//struct tm dbrs_getDateTimeByName(T R, const char *columnName) {
//	assert(R);
//	return dbrs_getDateTime(R, getIndex(R, columnName));
//}

