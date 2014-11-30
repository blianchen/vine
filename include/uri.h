
#ifndef _URI_H_
#define _URI_H_

#ifdef __cplusplus
extern "C" {
#endif

int uri_decode(const char * src, int len, char * des);

/*
 * 未编码的字符
	0 1 2 3 4 5 6 7 8 9
	a b c d e f g h i j k l m n o p q r s t u v w x y z
	A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
	; / ? : @ & = + $ , #
	- _ . ! ~ * ' ( )
 */
int uri_encode(const char * src, int len, char * des);

char *uri_unescape(unsigned char *url);


/**
 * <b>URL</b> represent an immutable Uniform Resource Locator.
 * A Uniform Resource Locator (URL), is used to uniquely identify a
 * resource on the Internet. The URL is a compact text string with a
 * restricted syntax that consists of four main components:
 * <center><code>
 * &lt;protocol&gt;://&lt;authority&gt;&lt;path&gt;?&lt;query&gt;
 * </code></center>
 *
 * The &lt;protocol&gt; part is mandatory, the other components may or
 * may not be present in an URL string. For instance the
 * <code>file</code> protocol only use the path component while a
 * <code>http</code> protocol may use all components. Here is an
 * example where all components are used:
 *
 * <pre>
 * http://user:password@www.foo.bar:8080/document/index.csp?querystring#ref
 * </pre>
 *
 * The following URL components are automatically unescaped according to the escaping
 * mechanism defined in RFC 2396; <code>credentials</code>, <code>path</code> and parameter
 * <code>values</code>.
 *
 * An <i>IPv6 address</i> can be used for host as defined in
 * <a href="http://www.ietf.org/rfc/rfc2732.txt">RFC2732</a> by enclosing the
 * address in [brackets]. For instance, mysql://[2010:836B:4179::836B:4179]:3306/test
 *
 * For more information about the URL syntax and specification, see,
 * <a href="ftp://ftp.rfc-editor.org/in-notes/rfc2396.txt">RFC2396 -
 * Uniform Resource Identifiers (URI): Generic Syntax</a>
 */


#define T uri_t
typedef struct URI_S *T;

/**
 * Create a new URL object from the <code>url</code> parameter
 * string. The url string <i>must</i> start with a protocol
 * specifier, such as <code>http://</code> or <code>ftp://</code>
 * @param url A string specifying the URL
 * @return A URL object or NULL if the <code>url</code> parameter
 * cannot be parsed as an URL.
 */
T uri_new(const char *url);


/**
 * Factory method for building an URL object using a variable argument
 * list. <i>Important</i>: since the '%%' character is used as a format
 * specifier (e.g. %%s for string, %%d for integer and so on),
 * submitting an URL escaped string (i.e. a %%HEXHEX encoded string)
 * in the <code>url</code> parameter can produce undesired results. In
 * this case, use either the uri_new() method or uri_unescape() the
 * <code>url</code> parameter first.
 * @param url A string specifying the URL
 * @return A URL object or NULL if the <code>url</code> parameter
 * cannot be parsed as an URL.
 */
T uri_create(const char *url, ...) __attribute__((format (printf, 1, 2)));


/**
 * Destroy a URL object.
 * @param U A URL object reference
 */
void uri_free(T *U);


/** @name Properties */
//@{

/**
 * Get the protocol of the URL.
 * @param U An URL object
 * @return The protocol name
 */
const char *uri_getProtocol(T U);


/**
 * Get the user name from the authority part of the URL.
 * @param U An URL object
 * @return A username specified in the URL or NULL if not found
 */
const char *uri_getUser(T U);


/**
 * Get the password from the authority part of the URL.
 * @param U An URL object
 * @return A password specified in the URL or NULL if not found
 */
const char *uri_getPassword(T U);


/**
 * Get the hostname of the URL.
 * @param U An URL object
 * @return The hostname of the URL or NULL if not found
 */
const char *uri_getHost(T U);


/**
 * Get the port of the URL.
 * @param U An URL object
 * @return The port number of the URL or -1 if not specified
 */
int uri_getPort(T U);


/**
 * Get the path of the URL.
 * @param U An URL object
 * @return The path of the URL or NULL if not found
 */
const char *uri_getPath(T U);


/**
 * Get the query string of the URL.
 * @param U An URL object
 * @return The query string of the URL or NULL if not found
 */
const char *uri_getQueryString(T U);

/**
 * Returns an array of string objects with the names of the
 * parameters contained in this URL. If the URL has no parameters,
 * the method returns NULL. The last value in the array is NULL.
 * To print all parameter names and their values contained in this
 * URL, the following code can be used:
 * <pre>
 *   const char **params = uri_getParameterNames(U);
 *   if (params)
 *           for (int i = 0; params[i]; i++)
 *                   printf("%s = %s\n", params[i], uri_getParameter(U, params[i]));
 * </pre>
 * @param U An URL object
 * @return An array of string objects, each string containing the name
 * of a URL parameter; or NULL if the URL has no parameters
 */
const char **uri_getParameterNames(T U);

/**
 * Returns the value of a URL parameter as a string, or NULL if
 * the parameter does not exist. If you use this method with a
 * multi-valued parameter, the value returned is the first value found.
 * Lookup is <i>case-sensitive</i>.
 * @param U An URL object
 * @param name The parameter name to lookup
 * @return The parameter value or NULL if not found
 */
const char *uri_getParameter(T U, const char *name);

/**
 * Returns a string representation of this URL object
 * @param U An URL object
 * @return The URL string
 */
const char *uri_toString(T U);

#undef T

#ifdef __cplusplus
}
#endif

#endif
