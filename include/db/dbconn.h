#ifndef _DBCONNECTON_H_
#define _DBCONNECTON_H_

#include <uri.h>

#include <db/dbrs.h>

#define T dbconn_t
typedef struct dbconn_s *T;

/**
 * Create a new Connection.
 * @param pool The parent connection pool
 * @return A new Connection object or NULL on error
 */
T dbconn_new(void *pool);

/**
 * Destroy a Connection and release allocated resources.
 * @param C A Connection object reference
 */
void dbconn_free(T *conn);

/**
 * Set if this Connection is available and not already in use.
 * @param C A Connection object
 * @param isAvailable true if this Connection is available otherwise false
 */
void dbconn_setAvailable(T conn, int isAvailable);

/**
 * Get the availablity of this Connection.
 * @param C A Connection object
 * @return true If this Connection is available otherwise false
 */
int dbconn_isAvailable(T conn);

/**
 * Return true if this Connection is in a transaction that has not
 * been committed.
 * @param C A Connection object
 * @return true if this Connection is in a transaction otherwise false
 */
int dbconn_isInTransaction(T C);

/**
 * Sets the number of milliseconds the Connection should wait for a
 * SQL statement to finish if the database is busy. If the limit is
 * exceeded, then the <code>execute</code> methods will return
 * immediately with an error. The default timeout is <code>3
 * seconds</code>.
 * @param C A Connection object
 * @param ms The query timeout limit in milliseconds; zero means
 * there is no limit
 */
void dbconn_setQueryTimeout(T conn, int ms);

/**
 * Retrieves the number of milliseconds the Connection will wait for a
 * SQL statement object to execute.
 * @param C A Connection object
 * @return The query timeout limit in milliseconds; zero means there
 * is no limit
 */
int dbconn_getQueryTimeout(T conn);

///**
// * Sets the limit for the maximum number of rows that any ResultSet
// * object can contain. If the limit is exceeded, the excess rows
// * are silently dropped.
// * @param C A Connection object
// * @param max The new max rows limit; 0 means there is no limit
// */
//void Connection_setMaxRows(T C, int max);
//
///**
// * Retrieves the maximum number of rows that a ResultSet object
// * produced by this Connection object can contain. If this limit is
// * exceeded, the excess rows are silently dropped.
// * @param C A Connection object
// * @return The new max rows limit; 0 means there is no limit
// */
//int Connection_getMaxRows(T C);


/**
 * Returns this Connection URL
 * @param C A Connection object
 * @return This Connection URL
 * @see URL.h
 */
uri_t dbconn_getURL(T conn);

//@}

/**
 * Ping the database server and returns true if this Connection is
 * alive, otherwise false in which case the Connection should be closed.
 * @param C A Connection object
 * @return true if Connection is connected to a database server
 * otherwise false
 */
int dbconn_ping(T conn);

///**
// * Close any ResultSet and PreparedStatements in the Connection.
// * Normally it is not necessary to call this method, but for some
// * implementation (SQLite) it <i>may, in some situations,</i> be
// * necessary to call this method if a execution sequence error occurs.
// * @param C A Connection object
// */
void dbconn_clear(T C);

/**
 * Return connection to the connection pool. The same as calling
 * ConnectionPool_returnConnection() on a connection.
 * @param C A Connection object
 */
void dbconn_close(T conn);

/**
 * Start a transaction.
 * @param C A Connection object
 * @exception SQLException If a database error occurs
 * @see SQLException.h
 */
void dbconn_beginTransaction(T conn);

/**
 * Makes all changes made since the previous commit/rollback permanent
 * and releases any database locks currently held by this Connection
 * object.
 * @param C A Connection object
 * @exception SQLException If a database error occurs
 * @see SQLException.h
 */
void dbconn_commit(T conn);

/**
 * Undoes all changes made in the current transaction and releases any
 * database locks currently held by this Connection object. This method
 * will first call Connection_clear() before performing the rollback to
 * clear any statements in progress such as selects.
 * @param C A Connection object
 * @exception SQLException If a database error occurs
 * @see SQLException.h
 */
void dbconn_rollback(T conn);

///**
// * Returns the value for the most recent INSERT statement into a
// * table with an AUTO_INCREMENT or INTEGER PRIMARY KEY column.
// * @param C A Connection object
// * @return The value of the rowid from the last insert statement
// */
//long long Connection_lastRowId(T C);
//
///**
// * Returns the number of rows that was inserted, deleted or modified
// * by the last Connection_execute() statement. If used with a
// * transaction, this method should be called <i>before</i> commit is
// * executed, otherwise 0 is returned.
// * @param C A Connection object
// * @return The number of rows changed by the last (DIM) SQL statement
// */
//long long Connection_rowsChanged(T C);

/**
 * Executes the given SQL statement, which may be an INSERT, UPDATE,
 * or DELETE statement or an SQL statement that returns nothing, such
 * as an SQL DDL statement. Several SQL statements can be used in the
 * sql parameter string, each separated with the <i>;</i> SQL
 * statement separator character. <b>Note</b>, calling this method
 * clears any previous ResultSets associated with the Connection.
 * @param C A Connection object
 * @param sql A SQL statement
 * @exception SQLException If a database error occurs.
 * @see SQLException.h
 */
void dbconn_execute(T conn, const char *sql, ...) __attribute__((format (printf, 2, 3)));


/**
 * Executes the given SQL statement, which returns a single ResultSet
 * object. You may <b>only</b> use one SQL statement with this method.
 * This is different from the behavior of Connection_execute() which
 * executes all SQL statements in its input string. If the sql
 * parameter string contains more than one SQL statement, only the
 * first statement is executed, the others are silently ignored.
 * A ResultSet "lives" only until the next call to
 * Connection_executeQuery(), Connection_execute() or until the
 * Connection is returned to the Connection Pool. <i>This means that
 * Result Sets cannot be saved between queries</i>.
 * @param C A Connection object
 * @param sql A SQL statement
 * @return A ResultSet object that contains the data produced by the
 * given query.
 * @exception SQLException If a database error occurs.
 * @see ResultSet.h
 * @see SQLException.h
 */
dbrs_t dbconn_executeQuery(T conn, const char *sql, ...) __attribute__((format (printf, 2, 3)));


/**
 * Creates a PreparedStatement object for sending parameterized SQL
 * statements to the database. The <code>sql</code> parameter may
 * contain IN parameter placeholders. An IN placeholder is specified
 * with a '?' character in the sql string. The placeholders are
 * then replaced with actual values by using the PreparedStatement's
 * setXXX methods. Only <i>one</i> SQL statement may be used in the sql
 * parameter, this in difference to Connection_execute() which may
 * take several statements. A PreparedStatement "lives" until the
 * Connection is returned to the Connection Pool.
 * @param C A Connection object
 * @param sql A single SQL statement that may contain one or more '?'
 * IN parameter placeholders
 * @return A new PreparedStatement object containing the pre-compiled
 * SQL statement.
 * @exception SQLException If a database error occurs.
 * @see PreparedStatement.h
 * @see SQLException.h
 */
//PreparedStatement_T Connection_prepareStatement(T C, const char *sql, ...) __attribute__((format (printf, 2, 3)));


/**
 * This method can be used to obtain a string describing the last
 * error that occurred. Inside a CATCH-block you can also find
 * the error message directly in the variable Exception_frame.message.
 * It is recommended to use this variable instead since it contains both
 * SQL errors and API errors such as parameter index out of range etc,
 * while Connection_getLastError() might only show SQL errors
 * @param C A Connection object
 * @return A string explaining the last error
 */
const char *dbconn_getLastError(T C);


/** @name Class methods */
//@{

/**
 * <b>Class method</b>, test if the specified database system is
 * supported by this library. Clients may pass a full Connection URL,
 * for example using URL_toString(), or for convenience only the protocol
 * part of the URL. E.g. "mysql" or "sqlite".
 * @param url A database url string
 * @return true if supported otherwise false
 */
//int dbconn_isSupported(const char *url);

#undef T
#endif
