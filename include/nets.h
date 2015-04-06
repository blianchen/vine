/*
 * nets.h
 *
 *  Created on: Dec 17, 2014
 *      Author: blc
 */

#ifndef _NETS_H_
#define _NETS_H_

#include <st/st.h>

#define SOCKET_DATA_BUF_SIZE 4096

#define SOCKET_LISTEN_REQUEST_QUEUE 256


typedef struct socket_s {
	st_netfd_t nfd;
//	int osfd;
	char* ip;
	char* buf;
	int buflen;
	int package;
	int timeout;
	int port;
} *socket_t;


socket_t nets_listen(int port, const char* host, const char* options);

socket_t nets_accept(socket_t serverSocket);

socket_t nets_connect(const char* addr, const char* options);

int nets_read(socket_t soc);

int nets_write(socket_t soc, char *data, int writeNum);

void nets_close(socket_t soc);

#endif /* NETS_H_ */
