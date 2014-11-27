/*
 * serversocket.c
 *
 *  Created on: Nov 18, 2014
 *      Author: blc
 */

#include <luaapi/luanet.h>

#include <sys/ioctl.h>

static int read_net_address(const char *str, struct sockaddr_in *sin) {
	char host[128], *p;
	struct hostent *hp;
	unsigned short port;

	strcpy(host, str);
	if ((p = strchr(host, ':')) == NULL) {
		LOG_DEBUG("invalid address: %s\n", host);
		return -1;
	}
	*p++ = '\0';
	port = (unsigned short) atoi(p);
	if (port < 1) {
		LOG_DEBUG("invalid port: %s\n", p);
		return -1;
	}

	memset(sin, 0, sizeof(struct sockaddr_in));
	sin->sin_family = AF_INET;
	sin->sin_port = htons(port);
	if (host[0] == '\0') {
		sin->sin_addr.s_addr = INADDR_ANY;
		return 0;
	}
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == INADDR_NONE) {
		/* not dotted-decimal */
		if ((hp = gethostbyname(host)) == NULL) {
			LOG_DEBUG("can't resolve address: %s\n", host);
			return -1;
		}
		memcpy(&sin->sin_addr, hp->h_addr, hp->h_length);
	}
	return 0;
}

static int server_socket(lua_State* l) {
	size_t adrl;
	// gethostbyname
	const char* adr = luaL_checklstring(l, 1, &adrl);		//ip or domain name
	int port = luaL_checkinteger(l, 2);				//port

	int sock;
	if ((sock=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG_DEBUG("Create socket error: %s(errno: %d)", getLastErrorText(), getLastError());
//		THROW(NetException, "create socket error: %s(errno: %d)", getLastErrorText(), getLastError());
	}
	int n = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n)) < 0) {
		close(sock);
		LOG_DEBUG("setsockopt error: %s(errno: %d)", getLastErrorText(), getLastError());
//		THROW(NetException, "setsockopt error: %s(errno: %d)", getLastErrorText(), getLastError());
	}

	//初始化
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	servaddr.sin_port = htons(port);		//设置的端口为DEFAULT_PORT
	if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		close(sock);
		LOG_DEBUG("Bind socket error: %s(errno: %d)", getLastErrorText(), getLastError());
//		THROW(NetException, "bind socket error: %s(errno: %d)", getLastErrorText(), getLastError());
	}
	if (listen(sock, 256) == -1) {
		close(sock);
		LOG_DEBUG("Listen socket error: %s(errno: %d)", getLastErrorText(), getLastError());
//		THROW(NetException, "listen socket error: %s(errno: %d)", getLastErrorText(), getLastError());
	}

	st_netfd_t st_srvfd;
	if ((st_srvfd = st_netfd_open_socket(sock)) == NULL) {
		close(sock);
		LOG_DEBUG("st_netfd_open_socket error: %s(errno: %d)", getLastErrorText(), getLastError());
//		THROW(NetException, "st_netfd_open_socket error: %s(errno: %d)", getLastErrorText(), getLastError());
	}

	lua_pushlightuserdata(l, st_srvfd);		// return a server_socket
	return 1;
}

static int accept_socket(lua_State* l) {
	st_netfd_t st_srvfd = lua_touserdata(l, 1);		//a server_socket

	struct sockaddr_in cli_add;
	int n = sizeof(cli_add);
	st_netfd_t st_clifd = st_accept(st_srvfd, (struct sockaddr *)&cli_add, &n, ST_UTIME_NO_TIMEOUT);
	if (st_clifd == NULL) {

	}

	char* ip = inet_ntoa(cli_add.sin_addr);

	lua_pushlightuserdata(l, st_clifd);		// client socket
	lua_pushstring(l, ip);					// ip
	lua_pushinteger(l, cli_add.sin_port);	// port

	return 3;
}

static int client_socket(lua_State* l) {
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		//print_sys_error("socket");
	}
	st_netfd_t cli_nfd;
	if ((cli_nfd = st_netfd_open_socket(sock)) == NULL) {
		//print_sys_error("st_netfd_open_socket");
		close(sock);
	}

	lua_pushlightuserdata(l, cli_nfd);
	return 1;
}

static int connect_socket(lua_State* l) {
	st_netfd_t sock = lua_touserdata(l, 1);

	size_t addrLen;
	const char* addr = luaL_checklstring(l, 2, &addrLen);
	struct sockaddr_in rmt_addr;
	if (read_net_address(addr, &rmt_addr) == -1) {
		lua_pushinteger(l, -1);
	}

	st_utime_t timeout = ST_UTIME_NO_TIMEOUT;
	if (lua_gettop(l) > 2) {
		timeout = luaL_checknumber(l, 3);
		if (timeout <= 0) timeout = ST_UTIME_NO_TIMEOUT;
	}

	if (st_connect(sock, (struct sockaddr *) &rmt_addr, sizeof(rmt_addr), timeout) < 0) {
		st_netfd_close(sock);
		lua_pushinteger(l, -1);
//		lua_pushstring(l, getLastErrorText());
	} else {
		lua_pushinteger(l, 0);
	}
	return 1;
}

//static int getreadable(_st_netfd_t * netfd1) {
//	int n = 0;
//	while (1) {
//		ioctl(netfd1->osfd, FIONREAD, &n);
////		if (ioctl(netfd1.osfd, FIONREAD, &n) == -1 || n == 0) {
////		}
//		printf("############# %d \n", n);
//		sleep(1);
//	}
//}


static int read_socket(lua_State* l) {
	st_netfd_t sock = lua_touserdata(l, 1);

	char buf[512];
	int rn = st_read(sock, buf, sizeof(buf), ST_UTIME_NO_TIMEOUT);

	lua_pushinteger(l, rn);
	lua_pushlstring(l, buf, rn);
	return 2;
}

static int write_socket(lua_State* l) {
	st_netfd_t sock = lua_touserdata(l, 1);
	size_t size;
	const char* buf = luaL_checklstring(l, 2, &size);

	int resbyte = st_write(sock, buf, size, ST_UTIME_NO_TIMEOUT); //写入的字节数

	lua_pushinteger(l, resbyte);
	return 1;
}

static int close_socket(lua_State* l) {
	st_netfd_t fd = lua_touserdata(l, 1);
	st_netfd_close(fd);
	return 0;
}

static const luaL_Reg funs[] = {
		{"server_socket", server_socket},
		{"accept", accept_socket},
		{"socket", client_socket},
		{"connect", connect_socket},
		{"read", read_socket},
		{"write", write_socket},
		{"close", close_socket},
		{NULL, NULL}
};

LUA_API int luaopen_netlib(lua_State* l) {
	luaL_register(l, LUA_NET_LIB_NAME, funs);
	return 1;
}
