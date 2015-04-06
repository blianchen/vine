/*
 * nets.c
 *
 *  Created on: Dec 17, 2014
 *      Author: blc
 */

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <str.h>

#include <exception/net_exception.h>
#include <mem.h>
#include <nets.h>


static int read_net_address(const char *str, struct sockaddr_in *sin) {
	char host[128], *p;
	struct hostent *hp;
	unsigned short port;

	strcpy(host, str);
	if ((p = strchr(host, ':')) == NULL) {
		LOG_WARN("invalid address: %s\n", host);
		return 0;
	}
	*p++ = '\0';
	port = (unsigned short) atoi(p);
	if (port < 1) {
		LOG_WARN("invalid port: %s\n", p);
		return 0;
	}

	memset(sin, 0, sizeof(struct sockaddr_in));
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	if (host[0] == '\0') {
		sin->sin_addr.s_addr = INADDR_ANY;
		return 1;
	}
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == INADDR_NONE) {
		/* not dotted-decimal */
		if ((hp = gethostbyname(host)) == NULL) {
			LOG_WARN("can't resolve address: %s\n", host);
			return 0;
		}
		memcpy(&sin->sin_addr, hp->h_addr, hp->h_length);
	}
	return 1;
}

static int read_ip_address(const char* host, struct sockaddr_in *sin) {
	struct hostent *hp;
	if (host == NULL || host[0] == '\0') {
		sin->sin_addr.s_addr = INADDR_ANY;
		return 1;
	}
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == INADDR_NONE) {
		/* not dotted-decimal */
		if ((hp = gethostbyname(host)) == NULL) {
			LOG_WARN("can't resolve address: %s\n", host);
			return 0;
		}
		memcpy(&sin->sin_addr, hp->h_addr, hp->h_length);
	}
	return 1;
}

socket_t nets_listen(int port, const char* host, const char* options) {
	int sockfd;
	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		THROW(net_exception, "Create socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	int package = 0;
	int timeout = (int)ST_UTIME_NO_TIMEOUT;
	if (options) {
		//param 3:  options : reuseaddr=true,keepalive=true, package=1/2/4, timeout=3000
		int n = 1, ir;
		kv_option_t kv = parse_kv_option(options);
		while (kv) {
			if (str_isEqual(kv->name, "reuseaddr") && str_isEqual(kv->value, "true")) {
				ir = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n));
			} else
			if (str_isEqual(kv->name, "keepalive") && str_isEqual(kv->value, "true")) {
				ir = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&n, sizeof(n));
			} else if (str_isEqual(kv->name, "timeout")) {
				timeout = str_parseInt(kv->value) * 1000; //ms
			} else if (str_isEqual(kv->name, "package")) {
				package = str_parseInt(kv->value);
			}
			if (ir < 0) {
				close(sockfd);
				THROW(net_exception, "setsockopt error: %s(errno: %d)", getLastErrorText(), getLastError());
				return NULL;
			}
			kv = kv->next;
		}
		clean_kv_option(&kv);
	}

	//初始化
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	read_ip_address(host, &servaddr);
	servaddr.sin_port = htons(port);		//设置的端口为DEFAULT_PORT
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		close(sockfd);
		THROW(net_exception, "Bind socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}
	if (listen(sockfd, SOCKET_LISTEN_REQUEST_QUEUE) == -1) {
		close(sockfd);
		THROW(net_exception, "Listen socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	st_netfd_t st_srvfd;
	if ((st_srvfd = st_netfd_open_socket(sockfd)) == NULL) {
		close(sockfd);
		THROW(net_exception, "st_netfd_open_socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	socket_t serverSocket = MALLOC(sizeof(struct socket_s));
	serverSocket->port = port;
	serverSocket->ip = str_dup(inet_ntoa(servaddr.sin_addr));
	serverSocket->nfd = st_srvfd;
	serverSocket->buf = NULL;
	serverSocket->buflen = 0;
	serverSocket->package = package;
	serverSocket->timeout = timeout;

	return serverSocket;
}

socket_t nets_accept(socket_t serverSocket) {
	struct sockaddr_in cli_add;
	int n = sizeof(cli_add);
	st_netfd_t st_clifd = st_accept(serverSocket->nfd, (struct sockaddr *)&cli_add, &n, ST_UTIME_NO_TIMEOUT);
	if (st_clifd == NULL) {
		THROW(net_exception, "socket accept error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	socket_t clientSocket = MALLOC(sizeof(struct socket_s));
	clientSocket->port = ntohs(cli_add.sin_port);
	clientSocket->ip = str_dup(inet_ntoa(cli_add.sin_addr));
	clientSocket->nfd = st_clifd;
	clientSocket->package = serverSocket->package;
	clientSocket->timeout = serverSocket->timeout;
//	if (clientSocket->package > 0) {
		clientSocket->buf = MALLOC(SOCKET_DATA_BUF_SIZE);
		clientSocket->buflen = SOCKET_DATA_BUF_SIZE;
//	} else {
//		clientSocket->buf = NULL;
//		clientSocket->buflen = 0;
//	}

	return clientSocket;
}

socket_t nets_connect(const char* addr, const char* options) {
	struct sockaddr_in rmt_addr;
	if (!read_net_address(addr, &rmt_addr)) {
		return NULL;
	}

	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		THROW(net_exception, "socket create error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}
	st_netfd_t cli_nfd;
	if ((cli_nfd = st_netfd_open_socket(sock)) == NULL) {
		close(sock);
		return NULL;
	}

	int package = 0, timeout = (int)ST_UTIME_NO_TIMEOUT;
	if (options) {
		//param 3:  options : reuseaddr=true,keepalive=true, package=1/2/4, timeout=3000
		int n = 1, ir;
		kv_option_t kv = parse_kv_option(options);
		while (kv) {
			if (str_isEqual(kv->name, "reuseaddr") && str_isEqual(kv->value, "true")) {
				ir = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n));
			} else if (str_isEqual(kv->name, "keepalive") && str_isEqual(kv->value, "true")) {
				ir = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&n, sizeof(n));
			} else if (str_isEqual(kv->name, "timeout")) {
				timeout = str_parseInt(kv->value) * 1000; //ms
			} else if (str_isEqual(kv->name, "package")) {
				package = str_parseInt(kv->value);
			}
			if (ir < 0) {
				st_netfd_close(cli_nfd);
				THROW(net_exception, "setsockopt error: %s(errno: %d)", getLastErrorText(), getLastError());
				return NULL;
			}
			kv = kv->next;
		}
		clean_kv_option(&kv);
	}

	if (st_connect(cli_nfd, (struct sockaddr *) &rmt_addr, sizeof(rmt_addr), timeout) < 0) {
		st_netfd_close(cli_nfd);
		THROW(net_exception, "st_connect error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	socket_t clientSocket = MALLOC(sizeof(struct socket_s));
	clientSocket->port = rmt_addr.sin_port;
	clientSocket->ip = str_dup(inet_ntoa(rmt_addr.sin_addr));
	clientSocket->nfd = cli_nfd;
	clientSocket->package = package;
	clientSocket->timeout = timeout;
//	if (clientSocket->package > 0) {
		clientSocket->buf = MALLOC(SOCKET_DATA_BUF_SIZE);
		clientSocket->buflen = SOCKET_DATA_BUF_SIZE;
//	} else {
//		clientSocket->buf = NULL;
//		clientSocket->buflen = 0;
//	}

	return clientSocket;
}

int nets_read(socket_t soc) {
	int rn, n;
	if (soc->package == 0) {
		rn = st_read(soc->nfd, soc->buf, SOCKET_DATA_BUF_SIZE, soc->timeout);
	} else {
		unsigned char hb[soc->package];		// package header length = 1, 2, 4
		rn = st_read(soc->nfd, hb, soc->package, soc->timeout);
		int unread;
		while (rn < soc->package) {
			unread = soc->package - rn;
			n = st_read(soc->nfd, hb+rn, unread, soc->timeout);
			if (n <= 0) { // read error
				return n;
			}
			rn = rn + n;
		}
		unsigned int headerLen = leb2i(hb, soc->package);
		if (soc->buflen < headerLen) {
			REALLOC(soc->buf, headerLen);
			soc->buflen = headerLen;
		}
		rn = st_read(soc->nfd, soc->buf, headerLen, soc->timeout);
		while (rn < headerLen) {
			unread = headerLen - rn;
			n = st_read(soc->nfd, soc->buf+rn, unread, soc->timeout);
			if (n <= 0) { // read error
				return n;
			}
			rn = rn + n;
		}
	}
	return rn;
}

int nets_write(socket_t soc, char *data, int writeNum) {
	int wn;
	if (soc->package == 0) {
		wn = st_write(soc->nfd, data, writeNum, soc->timeout); //写入的字节数
	} else {
		unsigned char hb[soc->package];
		lei2b(hb, soc->package, writeNum);
		struct iovec iov[2];
		iov[0].iov_base = hb;
		iov[0].iov_len = soc->package;
		iov[1].iov_base = data;
		iov[1].iov_len = writeNum;
		wn = st_writev(soc->nfd, iov, 2, soc->timeout);
	}
	return wn;
}

void nets_close(socket_t soc) {
	st_netfd_close(soc->nfd);
	if (soc->buf) {
		FREE(soc->buf);
	}
	FREE(soc->ip);
	FREE(soc);
}
