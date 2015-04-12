/*
 * vine.h
 *
 *  Created on: Dec 25, 2014
 *      Author: blc
 */

#ifndef _VINE_H_
#define _VINE_H_

#ifndef V_HIGH
#define V_HIGH 		1
#define V_LOW 		1
#define __D_TO_STR(s)	#s
#define _D_TO_STR(s)	__D_TO_STR(s)
#define VINE_RELEASE	"vine "_D_TO_STR(V_HIGH)"."_D_TO_STR(V_LOW)
#endif

#define NODE_TYPE	77	// 77 = normal node
#define PROT_TYPE 	0;	// 0 = tcp/ip-v4

#endif /* CORELIB_INCLUDE_VINE_H_ */
