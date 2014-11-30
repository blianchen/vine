
#ifndef _DBCONN_DELEGATE_H_
#define _DBCONN_DELEGATE_H_

/**
 * This interface defines the <b>contract</b> for the concrete database 
 * implementation used for delegation in the Connection class.
 */ 

#define T dbconn_delegate_t
typedef struct T *T;

typedef struct cop_t {
	const char *name;
	// Event handler class methods
	void (*onconn)(T C);
	void (*onstop)(T C);
	// Methods
	T (*new)(uri_t url);
	int (*connstate)(T C);
	void (*free)(T *C);
	int (*getsocket)(T C);
//	void (*setQueryTimeout)(T C, int ms);
//	void (*setMaxRows)(T C, int max);
	int (*ping)(T C);
	int (*beginTransaction)(T C);
	int (*commit)(T C);
	int (*rollback)(T C);
//	long long (*lastRowId)(T C);
//	long long (*rowsChanged)(T C);
	int (*execute)(T C, const char *sql, va_list ap);
//	dbrs_t (*executeQuery)(T C, const char *sql, va_list ap);
	dbrs_t (*getrs)(T C);
	dbpst_t (*prepareStatement)(T C, const char *sql, va_list ap);
	const char *(*getDbLastError)(T C);
}*cop_t;

#undef T
#endif
