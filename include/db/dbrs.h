
#ifndef _DBRS_H_
#define _DBRS_H_

#include <time.h>

//<< Protected methods
#include "../../src/db/dbrs_delegate.h"
//>> End Protected methods


#define T dbrs_t
typedef struct resultset_s *T;


//<< Protected methods

/**
 * Create a new ResultSet.
 * @param D the delegate used by this ResultSet
 * @param op delegate operations
 * @return A new ResultSet object
 */
T dbrs_new(dbresultset_delegate_t D, rop_t op);


/**
 * Destroy a ResultSet and release allocated resources.
 * @param R A ResultSet object reference
 */
void dbrs_free(T *R);

//>> End Protected methods

/** @name Properties */
//@{

/**
 * Returns the number of columns in this ResultSet object.
 * @param R A ResultSet object
 * @return The number of columns
 */
int dbrs_getColumnCount(T R);


/**
 * Get the designated column's name.
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return Column name or NULL if the column does not exist. You 
 * should use the method ResultSet_getColumnCount() to test for 
 * the availability of columns in the result set.
 */
const char *dbrs_getColumnName(T R, int columnIndex);


/**
 * Returns column size in bytes. If the column is a blob then 
 * this method returns the number of bytes in that blob. No type 
 * conversions occur. If the result is a string (or a number 
 * since a number can be converted into a string) then return the 
 * number of bytes in the resulting string. 
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return Column data size
 * @exception SQLException If columnIndex is outside the valid range
 * @see SQLException.h
 */
long dbrs_getColumnSize(T R, int columnIndex);

//@}

/**
 * Moves the cursor down one row from its current position. A
 * ResultSet cursor is initially positioned before the first row; the
 * first call to this method makes the first row the current row; the
 * second call makes the second row the current row, and so on. When
 * there are not more available rows false is returned. An empty
 * ResultSet will return false on the first call to ResultSet_next().
 * @param R A ResultSet object
 * @return true if the new current row is valid; false if there are no
 * more rows
 * @exception SQLException If a database access error occurs
 */
int dbrs_next(T R);

/** @name Columns */
//@{

/**
 * Returns true if the value of the designated column in the current row of
 * this ResultSet object is SQL NULL, otherwise false. If the column value is 
 * SQL NULL, a Result Set returns the NULL pointer for string and blob values
 * and 0 for primitive data types. Use this method if you need to differ 
 * between SQL NULL and the value NULL/0.
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return True if column value is SQL NULL, otherwise false
 * @exception SQLException If a database access error occurs or
 * columnIndex is outside the valid range
 * @see SQLException.h
 */
int dbrs_isnull(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a C-string. If <code>columnIndex</code>
 * is outside the range [1..ResultSet_getColumnCount()] this
 * method throws an SQLException. <i>The returned string may only be 
 * valid until the next call to ResultSet_next() and if you plan to use
 * the returned value longer, you must make a copy.</i>
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return The column value; if the value is SQL NULL, the value
 * returned is NULL
 * @exception SQLException If a database access error occurs or
 * columnIndex is outside the valid range
 * @see SQLException.h
 */
const char *dbrs_getString(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a C-string. If <code>columnName</code>
 * is not found this method throws an SQLException. <i>The returned string
 * may only be valid until the next call to ResultSet_next() and if you plan
 * to use the returned value longer, you must make a copy.</i>
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @return The column value; if the value is SQL NULL, the value
 * returned is NULL
 * @exception SQLException If a database access error occurs or
 * columnName does not exist
 * @see SQLException.h
 */
const char *dbrs_getStringByName(T R, const char *columnName);


/**
 * Retrieves the value of the designated column in the current row of this 
 * ResultSet object as an int. If <code>columnIndex</code> is outside the 
 * range [1..ResultSet_getColumnCount()] this method throws an SQLException. 
 * In general, on both 32 and 64 bits architecture, <code>int</code> is 4 bytes
 * or 32 bits and <code>long long</code> is 8 bytes or 64 bits. A
 * <code>long</code> type is usually equal to <code>int</code> on 32 bits
 * architecture and equal to <code>long long</code> on 64 bits architecture.
 * However, the width of integer types are architecture and compiler dependent.
 * The above is usually true, but not necessarily.
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return The column value; if the value is SQL NULL, the value
 * returned is 0
 * @exception SQLException If a database access error occurs, columnIndex
 * is outside the valid range or if the value is NaN
 * @see SQLException.h
 */
int dbrs_getInt(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as an int. If <code>columnName</code> is
 * not found this method throws an SQLException.
 * In general, on both 32 and 64 bits architecture, <code>int</code> is 4 bytes
 * or 32 bits and <code>long long</code> is 8 bytes or 64 bits. A
 * <code>long</code> type is usually equal to <code>int</code> on 32 bits
 * architecture and equal to <code>long long</code> on 64 bits architecture.
 * However, the width of integer types are architecture and compiler dependent.
 * The above is usually true, but not necessarily.
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @return The column value; if the value is SQL NULL, the value
 * returned is 0
 * @exception SQLException If a database access error occurs, columnName
 * does not exist or if the value is NaN
 * @see SQLException.h
 */
int dbrs_getIntByName(T R, const char *columnName);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a long long. If <code>columnIndex</code>
 * is outside the range [1..ResultSet_getColumnCount()] this
 * method throws an SQLException.
 * In general, on both 32 and 64 bits architecture, <code>int</code> is 4 bytes
 * or 32 bits and <code>long long</code> is 8 bytes or 64 bits. A 
 * <code>long</code> type is usually equal to <code>int</code> on 32 bits 
 * architecture and equal to <code>long long</code> on 64 bits architecture.
 * However, the width of integer types are architecture and compiler dependent.
 * The above is usually true, but not necessarily.
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return The column value; if the value is SQL NULL, the value
 * returned is 0
 * @exception SQLException If a database access error occurs,
 * columnIndex is outside the valid range or if the value is NaN
 * @see SQLException.h
 */
long long dbrs_getLLong(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a long long. If <code>columnName</code>
 * is not found this method throws an SQLException.
 * In general, on both 32 and 64 bits architecture, <code>int</code> is 4 bytes
 * or 32 bits and <code>long long</code> is 8 bytes or 64 bits. A
 * <code>long</code> type is usually equal to <code>int</code> on 32 bits
 * architecture and equal to <code>long long</code> on 64 bits architecture.
 * However, the width of integer types are architecture and compiler dependent.
 * The above is usually true, but not necessarily.
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @return The column value; if the value is SQL NULL, the value
 * returned is 0
 * @exception SQLException If a database access error occurs, columnName
 * does not exist or if the value is NaN
 * @see SQLException.h
 */
long long dbrs_getLLongByName(T R, const char *columnName);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a double. If <code>columnIndex</code>
 * is outside the range [1..ResultSet_getColumnCount()] this
 * method throws an SQLException.
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return The column value; if the value is SQL NULL, the value
 * returned is 0.0
 * @exception SQLException If a database access error occurs, columnIndex
 * is outside the valid range or if the value is NaN
 * @see SQLException.h
 */
double dbrs_getDouble(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a double. If <code>columnName</code> is
 * not found this method throws an SQLException.
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @return The column value; if the value is SQL NULL, the value
 * returned is 0.0
 * @exception SQLException If a database access error occurs, columnName
 * does not exist or if the value is NaN
 * @see SQLException.h
 */
double dbrs_getDoubleByName(T R, const char *columnName);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a void pointer. If <code>columnIndex</code>
 * is outside the range [1..ResultSet_getColumnCount()] this method 
 * throws an SQLException. <i>The returned blob may only be valid until
 * the next call to ResultSet_next() and if you plan to use the returned
 * value longer, you must make a copy.</i> 
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @param size The number of bytes in the blob is stored in size 
 * @return The column value; if the value is SQL NULL, the value
 * returned is NULL
 * @exception SQLException If a database access error occurs or 
 * columnIndex is outside the valid range
 * @see SQLException.h
 */
const void *dbrs_getBlob(T R, int columnIndex, int *size);


/**
 * Retrieves the value of the designated column in the current row of
 * this ResultSet object as a void pointer. If <code>columnName</code>
 * is not found this method throws an SQLException. <i>The returned
 * blob may only be valid until the next call to ResultSet_next() and if 
 * you plan to use the returned value longer, you must make a copy.</i>
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @param size The number of bytes in the blob is stored in size 
 * @return The column value; if the value is SQL NULL, the value
 * returned is NULL
 * @exception SQLException If a database access error occurs or 
 * columnName does not exist
 * @see SQLException.h
 */
const void *dbrs_getBlobByName(T R, const char *columnName, int *size);


/**
 * Retrieves the value of the designated column in the current row of this
 * ResultSet object as a Unix timestamp. The returned value is expected to be
 * in the UTC timezone and represent seconds since the <strong>epoch</strong>
 * (January 1, 1970, 00:00:00 GMT).
 *
 * Even though the underlying database might support timestamp ranges before
 * the epoch and after '2038-01-19 03:14:07 UTC' it is safest not to assume or
 * use values outside this range. Especially on a 32-bits system.
 *
 * <i class="textinfo">SQLite</i> does not have temporal SQL data types per se
 * and using this method with SQLite assume the column value in the Result Set
 * to be either a numerical value representing a Unix Time in UTC which is
 * returned as-is or a <a href="http://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>
 * time string which is converted to a time_t value.
 * See also PreparedStatement_setTimestamp()
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return The column value as seconds since the epoch in the 
 * <i class="textinfo">UTC timezone</i>. If the value is SQL NULL, the
 * value returned is 0, i.e. January 1, 1970, 00:00:00 GMT
 * @exception SQLException If a database access error occurs, if 
 * <code>columnIndex</code> is outside the range [1..ResultSet_getColumnCount()]
 * or if the column value cannot be converted to a valid timestamp
 * @see SQLException.h PreparedStatement_setTimestamp
 */
time_t dbrs_getTimestamp(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of this
 * ResultSet object as a Unix timestamp. The returned value is expected to be
 * in the UTC timezone and represent seconds since the <strong>epoch</strong>
 * (January 1, 1970, 00:00:00 GMT).
 *
 * Even though the underlying database might support timestamp ranges before
 * the epoch and after '2038-01-19 03:14:07 UTC' it is safest not to assume or
 * use values outside this range. Especially on a 32-bits system.
 *
 * <i class="textinfo">SQLite</i> does not have temporal SQL data types per se
 * and using this method with SQLite assume the column value in the Result Set
 * to be either a numerical value representing a Unix Time in UTC which is 
 * returned as-is or a <a href="http://en.wikipedia.org/wiki/ISO_8601">ISO 8601</a>
 * time string which is converted to a time_t value.
 * See also PreparedStatement_setTimestamp()
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @return The column value as seconds since the epoch in the
 * <i class="textinfo">UTC timezone</i>. If the value is SQL NULL, the
 * value returned is 0, i.e. January 1, 1970, 00:00:00 GMT
 * @exception SQLException If a database access error occurs, if 
 * <code>columnName</code> is not found or if the column value cannot be 
 * converted to a valid timestamp
 * @see SQLException.h PreparedStatement_setTimestamp
 */
time_t dbrs_getTimestampByName(T R, const char *columnName);


/**
 * Retrieves the value of the designated column in the current row of this
 * ResultSet object as a Date, Time or DateTime. This method can be used to
 * retrieve the value of columns with the SQL data type, Date, Time, DateTime
 * or Timestamp. The returned <code>tm</code> structure follows the convention
 * for usage with mktime(3) where, tm_hour = hours since midnight [0-23],
 * tm_min = minutes after the hour [0-59], tm_sec = seconds after the minute
 * [0-60], tm_mday = day of the month [1-31] and tm_mon = months since January
 * <b class="textinfo">[0-11]</b>. tm_gmtoff is set to the offset from UTC in
 * seconds if the column value contains timezone information, otherwise 
 * tm_gmtoff is set to 0. <i>On systems without tm_gmtoff, (Solaris), the 
 * member, tm_wday is set to gmt offset instead as this property is ignored 
 * by mktime on input.</i> The exception to the above is <b>tm_year</b> which
 * contains the year literal and <i>not years since 1900</i> which is the 
 * convention. All other fields in the structure are set to zero. If the 
 * column type is DateTime or Timestamp all the fields mentioned above are 
 * set, if it is a Date or Time, only the relevant fields are set.
 *
 * @param R A ResultSet object
 * @param columnIndex The first column is 1, the second is 2, ...
 * @return A tm structure with fields for date and time. If the value
 * is SQL NULL, a zeroed tm structure is returned
 * @exception SQLException If a database access error occurs, if
 * <code>columnIndex</code> is outside the range [1..ResultSet_getColumnCount()] 
 * or if the column value cannot be converted to a valid SQL Date, Time or 
 * DateTime type
 * @see SQLException.h
 */
struct tm dbrs_getDateTime(T R, int columnIndex);


/**
 * Retrieves the value of the designated column in the current row of this
 * ResultSet object as a Date, Time or DateTime. This method can be used to
 * retrieve the value of columns with the SQL data type, Date, Time, DateTime
 * or Timestamp. The returned <code>tm</code> structure follows the convention
 * for usage with mktime(3) where, tm_hour = hours since midnight [0-23],
 * tm_min = minutes after the hour [0-59], tm_sec = seconds after the minute
 * [0-60], tm_mday = day of the month [1-31] and tm_mon = months since January
 * <b class="textinfo">[0-11]</b>. tm_gmtoff is set to the offset from UTC in
 * seconds if the column value contains timezone information, otherwise
 * tm_gmtoff is set to 0. <i>On systems without tm_gmtoff, (Solaris), the
 * member, tm_wday is set to gmt offset instead as this property is ignored
 * by mktime on input.</i> The exception to the above is <b>tm_year</b> which
 * contains the year literal and <i>not years since 1900</i> which is the
 * convention. All other fields in the structure are set to zero. If the
 * column type is DateTime or Timestamp all the fields mentioned above are
 * set, if it is a Date or Time, only the relevant fields are set.
 *
 * @param R A ResultSet object
 * @param columnName The SQL name of the column. <i>case-sensitive</i>
 * @return A tm structure with fields for date and time. If the value
 * is SQL NULL, a zeroed tm structure is returned
 * @exception SQLException If a database access error occurs, if 
 * <code>columnName</code> is not found or if the column value cannot be 
 * converted to a valid SQL Date, Time or DateTime type
 * @see SQLException.h
 */
struct tm dbrs_getDateTimeByName(T R, const char *columnName);

#undef T
#endif
