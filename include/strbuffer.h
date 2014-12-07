
#ifndef STRINGBUFFER_INCLUDED
#define STRINGBUFFER_INCLUDED
#include <stdarg.h>


/** 
 * A <b>String Buffer</b> implements a mutable sequence of characters.
 */


#define T str_buffer_t
typedef struct T *T;


/**
 * Constructs a string buffer so that it represents the same sequence of 
 * characters as the string argument; in other  words, the initial contents 
 * of the string buffer is a copy of the argument string. 
 * @param s the initial contents of the buffer
 * @return A new StringBuffer object
 */
T strbuffer_new(const char *s);


/**
 * Factory method, create an empty string buffer
 * @param hint The initial capacity of the buffer in bytes (hint > 0)
 * @return A new StringBuffer object
 * @exception AssertException if hint is less than or equal to 0
 * @exception MemoryException if allocation failed
 */
T strbuffer_create(int hint);


/**
 * Destroy a StringBuffer object and free allocated resources
 * @param S a StringBuffer object reference
 */
void strbuffer_free(T *S);


/**
 * The characters of the String argument are appended, in order, to the 
 * contents of this string buffer, increasing the length of this string 
 * buffer by the length of the arguments. 
 * @param S StringBuffer object
 * @param s A string with optional var args
 * @return A reference to this StringBuffer
 */
T strbuffer_append(T S, const char *s, ...) __attribute__((format (printf, 2, 3)));


/**
 * The characters of the String argument are appended, in order, to the 
 * contents of this string buffer, increasing the length of this string 
 * buffer by the length of the arguments. 
 * @param S StringBuffer object
 * @param s A string with optional var args
 * @param ap A variable argument list
 * @return A reference to this StringBuffer
 */
T strbuffer_vappend(T S, const char *s, va_list ap);


/**
 * Replace the content of this String Buffer with <code>s</code>. That is, the contents
 * of the string buffer is a copy of the argument string.
 * @param S StringBuffer object
 * @param s A string with optional var args
 * @return a reference to this StringBuffer
 * @exception MemoryException if allocation was used and failed
 */
T strbuffer_set(T S, const char *s, ...) __attribute__((format (printf, 2, 3)));


/**
 * Replace the content of this String Buffer with <code>s</code>. That is, the contents
 * of the string buffer is a copy of the argument string.
 * @param S StringBuffer object
 * @param s A string with optional var args
 * @param ap A variable argument list
 * @return a reference to this StringBuffer
 * @exception MemoryException if allocation was used and failed
 */
T strbuffer_vset(T S, const char *s, va_list ap);


/**
 * Returns the length (character count) of this string buffer.
 * @param S StringBuffer object
 * @return The length of the sequence of characters currently represented 
 * by this string buffer
 */
int strbuffer_length(T S);


/**
 * Clear the contents of the string buffer. I.e. set buffer length to 0.
 * @param S StringBuffer object
 * @return a reference to this StringBuffer
 */
T strbuffer_clear(T S);


/**
 * Converts to a string representing the data in this string buffer.
 * @param S StringBuffer object
 * @return A string representation of the string buffer 
 */
const char *strbuffer_toString(T S);


/**
 * Replace all occurences of <code>?</code> in this string buffer with <code>$n</code>.
 * Example: 
 * <pre>
 * strbuffer_T b = strbuffer_new("insert into host values(?, ?, ?);");
 * strbuffer_prepare4postgres(b) -> "insert into host values($1, $2, $3);"
 * </pre>
 * @param S StringBuffer object
 * @return The number of replacements that took place
 * @exception SQLException If there are more than 99 wild card '?' parameters
 */
int strbuffer_prepare4postgres(T S);


/**
 * Replace all occurences of <code>?</code> in this string buffer with <code>:n</code>.
 * Example: 
 * <pre>
 * strbuffer_T b = strbuffer_new("insert into host values(?, ?, ?);");
 * strbuffer_prepare4oracle(b) -> "insert into host values(:1, :2, :3);"
 * </pre>
 * @param S StringBuffer object
 * @return The number of replacements that took place
 * @exception SQLException If there are more than 99 wild card '?' parameters
 */
int strbuffer_prepare4oracle(T S);


/**
 * Remove (any) leading and trailing white space and semicolon [ \\t\\r\\n;]. Example
 * <pre>
 * strbuffer_T b = strbuffer_new("\t select a from b; \n");
 * strbuffer_trim(b) -> "select a from b"
 * </pre>
 * @param S StringBuffer object
 * @return a reference to this StringBuffer
 */
T strbuffer_trim(T S);


#undef T
#endif
