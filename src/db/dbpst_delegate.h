
#ifndef _DBPST_DELEGATE_H_
#define _DBPST_DELEGATE_H_

#include <db/dbrs.h>

/**
 * This interface defines the <b>contract</b> for the concrete database 
 * implementation used for delegation in the PreparedStatement class.
 */ 

#define T dbpst_delegate_t
typedef struct T *T;

typedef struct pop_t {
	const char *name;
	void (*free)(T *P);
	void (*setString)(T P, int parameterIndex, const char *x);
	void (*setInt)(T P, int parameterIndex, int x);
	void (*setLLong)(T P, int parameterIndex, long long x);
	void (*setDouble)(T P, int parameterIndex, double x);
	void (*setTimestamp)(T P, int parameterIndex, time_t timestamp);
	void (*setBlob)(T P, int parameterIndex, const void *x, int size);
	int (*execute)(T P);
	dbrs_t (*getrs)(T P);
//	dbrs_t (*executeQuery)(T P);
	long long (*rowsChanged)(T P);
}*pop_t;

/**
 * Throws exception if parameterIndex is outside the parameterCount range.
 * @return parameterIndex - 1. In the API parameterIndex starts with 1,
 * internally it starts with 0.
 */
static inline int checkAndSetParameterIndex(int parameterIndex, int parameterCount) {
	int i = parameterIndex - 1;
	if (parameterCount <= 0 || i < 0 || i >= parameterCount)
		THROW(sql_exception, "Parameter index is out of range");
	return i;
}

#undef T
#endif
