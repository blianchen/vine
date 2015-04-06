/*
 * system compatible
 */
#ifndef _SC_H_
#define _SC_H_

#ifdef __WIN32__
#define DIRSEP "\\"
#define PATHSEP ";"
#define NULL_DEVICE "nul"
#define BINARY_EXT ""
#define DLL_EXT ".dll"
#else
#define PATHSEP ":"
#define DIRSEP "/"
#define NULL_DEVICE "/dev/null"
#define BINARY_EXT ""
#endif

#endif //_SC_H_
