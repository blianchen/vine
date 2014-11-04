
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

#ifdef __cplusplus
}
#endif

#endif
