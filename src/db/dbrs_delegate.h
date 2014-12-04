
#ifndef _DBRS_DELEGATE_H_
#define _DBRS_DELEGATE_H_

#include <exception/sql_exception.h>

/**
 * This interface defines the <b>contract</b> for the concrete database 
 * implementation used for delegation in the ResultSet class.
 */ 

#define T dbresultset_delegate_t
typedef struct T *T;

typedef struct rop_t {
	const char *name;
	void (*free)(T *R);
	int (*getColumnCount)(T R);
	const char *(*getColumnName)(T R, int columnIndex);
	long (*getColumnSize)(T R, int columnIndex);
	int (*next)(T R);
	int (*isnull)(T R, int columnIndex);
	const char *(*getString)(T R, int columnIndex);
	const void *(*getBlob)(T R, int columnIndex, int *size);
	time_t (*getTimestamp)(T R, int columnIndex);
	struct tm *(*getDateTime)(T R, int columnIndex, struct tm *tm);
}*rop_t;

/**
 * Throws exception if columnIndex is outside the columnCount range.
 * @return columnIndex - 1. In the API, columnIndex starts with 1,
 * internally it starts with 0.
 */
static inline int checkAndSetColumnIndex(int columnIndex, int columnCount) {
        int i = columnIndex - 1;
        if (columnCount <= 0 || i < 0 || i >= columnCount)
                THROW(sql_exception, "Column index is out of range");
        return i;
}

#undef T
#endif
