#ifndef _DBCONFIG_H_
#define _DBCONFIG_H_

#include <assert.h>

/* Version number of package */
#define VERSION "0.1"

/* Define to 1 to enable mysql */
//#define HAVE_LIBMYSQLCLIENT 1

/* Define to 1 to enable postgresql */
//#define HAVE_LIBPQ 1

/* Define to 1 if you have the <libpq-fe.h> header file. */
//#define HAVE_LIBPQ_FE_H 1

/* Define to 1 to enable sqlite3 */
//#define HAVE_LIBSQLITE3 1


/**
 * Standard millisecond timeout value for a database call.
 */
#define SQL_DEFAULT_TIMEOUT 3000

/**
 * The default maximum number of database connections
 */
#define SQL_DEFAULT_MAX_CONNECTIONS 10

/**
 * The initial number of database connections
 */
#define SQL_DEFAULT_INIT_CONNECTIONS 5

/**
 * Default Connection timeout in seconds, used by reaper to remove
 * inactive connections
 */
#define SQL_DEFAULT_CONNECTION_TIMEOUT 30

#endif
