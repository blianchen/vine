/*
 * threadmsg.c
 *
 *  Created on: Jan 23, 2015
 *      Author: blc
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <str.h>
#include <logger.h>
#include <intmap.h>
#include <hashmap.h>
#include <exception/net_exception.h>
#include <sc.h>

#include "common.h"

////////////// vpmd message codes ///////////////
/* Definitions of message codes */
/* Registration and queries */
#define EPMD_ALIVE2_REQ 'x'
#define EPMD_PORT2_REQ 'z'
#define EPMD_ALIVE2_RESP 'y'
#define EPMD_PORT2_RESP 'w'
#define EPMD_NAMES_REQ 'n'

/* Interactive client command codes */
#define EPMD_DUMP_REQ 'd'
#define EPMD_KILL_REQ 'k'
#define EPMD_STOP_REQ 's'

/////////////// nodes message codes ///////////////
#define NODE_CONN_REQ 'c'	// node connect
#define NODE_CONN_RESP 'r'
#define NODE_RPC_MSG 'm'	// send rpc msg
#define NODE_RPC_MSG_NAME 'e'	// send rpc msg by name
#define NODE_FINDID_REQ	'f'	// find a node info by id. only the the node be has the info will be return NODE_CONN_RESP message.

#define HOSTNAME_SIZE 128
#define INBUF_SIZE (2*MAXSYMLEN)
//#define OUTBUF_SIZE (2*MAXSYMLEN)

//// big endian
#define get_int16(s) ((((unsigned char*)  (s))[0] << 8) | \
                      (((unsigned char*)  (s))[1]))

#define put_int16(i, s) {((unsigned char*)(s))[0] = ((i) >> 8) & 0xff; \
                        ((unsigned char*)(s))[1] = (i)         & 0xff;}

#define get_uint64(s) ( (uint64_t)((((unsigned char*)  (s))[0])) << 56 | \
						(uint64_t)((((unsigned char*)  (s))[1])) << 48 | \
						(uint64_t)((((unsigned char*)  (s))[2])) << 40 | \
						(uint64_t)((((unsigned char*)  (s))[3])) << 32 | \
						(uint64_t)((((unsigned char*)  (s))[4])) << 24 | \
						(uint64_t)((((unsigned char*)  (s))[5])) << 16 | \
						(uint64_t)((((unsigned char*)  (s))[6])) << 8 | \
						(uint64_t)((((unsigned char*)  (s))[7])) )

#define put_uint64(i, s) {((unsigned char*)(s))[0] = ((i) >> 56) & 0xff; \
						((unsigned char*)(s))[1] = ((i) >> 48) & 0xff; \
						((unsigned char*)(s))[2] = ((i) >> 40) & 0xff; \
						((unsigned char*)(s))[3] = ((i) >> 32) & 0xff; \
						((unsigned char*)(s))[4] = ((i) >> 24) & 0xff; \
						((unsigned char*)(s))[5] = ((i) >> 16) & 0xff; \
						((unsigned char*)(s))[6] = ((i) >> 8) & 0xff; \
                        ((unsigned char*)(s))[7] = (i)         & 0xff;}



//static _st_thread_t *_st_send_msg_thread = NULL;	//send message thread, process all external node message
static uint16_t _st_vpmd_listen_port = 1511;

static int64map_t _st_nodeid_sock_map;
static int64map_t _st_nodeid_info_map;
static hashmap_t _st_nodeurl_info_map;

static char _this_hostname[HOSTNAME_SIZE];
static char *_st_this_node_name;
static uint64_t _st_this_node_id = 0;

static int _st_rpc_server_listen_port = 0; // current rpc server accept port, port is random allocation


_st_thread_msg_t *st_create_msg(const char *data, int len) {
	_st_thread_msg_t *msg = MALLOC(sizeof(_st_thread_msg_t) + len);
	msg->f_tid = ST_MAKE_TID(_ST_CURRENT_THREAD()->sid);
	msg->data = (char*)msg + sizeof(_st_thread_msg_t);
	msg->len = len;
	msg->refcnt = 0;
	memcpy(msg->data, data, len);  //data will be destroy with msg.
	return msg;
}

void st_destroy_msg(_st_thread_msg_t *msg) {
	if (msg->refcnt == 0) {
		FREE(msg);
	}
}

int st_msg_data(_st_thread_msg_t *msg, char **data) {
	*data = msg->data;
	return msg->len;
}

st_tid_t st_msg_fromtid(_st_thread_msg_t *msg) {
	return msg->f_tid;
}


void st_send_msg(_st_thread_t *thread, _st_thread_msg_t *msg) {
	_st_thread_msg_queue_t *mq = MALLOC(sizeof(_st_thread_msg_queue_t));
	msg->refcnt++;
	mq->msg = msg;
	ST_APPEND_LINK(mq, thread->msg_q);

	//Add the recv thread to runnable queue
	if (thread->state != _ST_ST_RUNNABLE) {
		thread->state = _ST_ST_RUNNABLE;
		_ST_ADD_RUNQ(thread);
	}
}

_st_thread_msg_t *st_recv_msg() {
	_st_thread_t *thread = _ST_CURRENT_THREAD();
	_st_thread_msg_queue_t *mq;
	while ((mq = thread->msg_q->next) == thread->msg_q) {
		thread->state = _ST_ST_MSG_WAIT;
		_ST_SWITCH_CONTEXT(thread);
	}
	ST_REMOVE_LINK(mq);
	_st_thread_msg_t *msg = mq->msg;
	FREE(mq);
	msg->refcnt--;
	return msg;
}

int st_has_msg(_st_thread_t *thread) {
	return thread->msg_q->next != thread->msg_q;
}


////////////////////// send and recv thread in nodes  /////////////////////////////
typedef struct {
	uint64_t id;
	uint16_t port;
	int nameLen;
	int hostLen;
	char *name;
	char *host;
	char *url;
//	_st_thread_t *thread;
} _rms_node_info;

/* rms = remote message server */
static void _start_vpmd(char *epmd);
static int _rms_start_server();
static int _rms_create_listen_socket();
static int _rms_register_node();
static void *_rms_register_node_loop(void *arg);
static void *_rms_server_accept_loop(void *arg);
static void *_rms_rcv_thread_loop(void *arg);
static st_netfd_t _rms_connect_to(char *addr, int port);
//static int _rms_write_message(st_netfd_t soc, _st_thread_msg_t *msg, uint64_t toid);
static int _rms_send_conn_req(st_netfd_t sock);
static int _rms_find_nodeinfo(st_tid_t tid, _rms_node_info **nodeinfo);
static void _rms_send_find_nodeid_req(st_tid_t tid);

static int length_str(char *x) {
	int i = 0;
	/*
	 * Using strlen is probably ok, but just to be sure, since we got UTF-8 strings, we do it ourselves.
	 * We assume null-terminated correctly encoded UTF-8.
	 */
	while (x[i])
		i++;
	return i;
}

static int copy_str(char *x, char *y) {
	int i = 0;
	/*
	 * Using strcpy() is probably ok, but just to be sure, since we got UTF-8 strings, we do it ourselves.
	 * We assume null-terminated correctly encoded UTF-8.
	 */
	while (1) {
		x[i] = y[i];
		if (y[i] == '\0')
			return i;
		i++;
	}
}

int st_send_msg_by_tid(st_tid_t tid, _st_thread_msg_t *msg) {
	/*
	 * to another node
	 */
	if (_st_nodeid_sock_map == NULL) {
		LOG_WARN("This node has not init rpc, can't send message in nodes.");
		return -1;
	}
	int nodeid = ST_NODEID(tid);
	st_netfd_t sock = int64map_get(_st_nodeid_sock_map, nodeid);
	if (sock == NULL) {
		_rms_node_info *nodeinfo = NULL;
		// find node info
		int loopn = 8;
		while (_rms_find_nodeinfo(nodeid, &nodeinfo) && loopn > 0) {
			loopn--;
		}
		if (nodeinfo == NULL) {
			LOG_WARN("Don't find nodeid:%lu", nodeid);
			return -1;
		}

		// create a connect to the node
		sock = _rms_connect_to(nodeinfo->host, nodeinfo->port);
		if (sock == NULL) return -1;	//error

		// send NODE_CONN_REQ
		_rms_send_conn_req(sock);

		// rcv loop
		st_thread_create(_rms_rcv_thread_loop, sock, 0, 0);
		int64map_put(_st_nodeid_sock_map, nodeid, sock);
	}

	unsigned char hb[19];
	hb[0] = NODE_RPC_MSG;
	put_int16(msg->len+16, hb+1);
	put_uint64(tid, hb+3);
	put_uint64(msg->f_tid, hb+11);
	struct iovec iov[2];
	iov[0].iov_base = hb;
	iov[0].iov_len = 19;
	iov[1].iov_base = msg->data;
	iov[1].iov_len = msg->len;
	return st_writev(sock, iov, 2, ST_UTIME_NO_TIMEOUT);
}

/*
 * nodeUrl = nodeName@hostName
 */
int st_send_msg_by_name(char *nodeUrl, char *threadName, _st_thread_msg_t *msg) {
	int nameLen = 0;
	char *nodeName = nodeUrl, *hostName = NULL;
	while (nodeUrl[nameLen]) {
		if (nodeUrl[nameLen] == '@') {
			hostName = nodeUrl + nameLen + 1;
			nodeUrl[nameLen] = '\0';
			nameLen++;
			break;
		}
		nameLen++;
	}
	if (hostName == NULL) {
		LOG_WARN("node format error. (%s)", nodeUrl);
		return -1;
	}

	_rms_node_info *nodeInfo;
	if (hashmap_get(_st_nodeurl_info_map, nodeUrl, (void*)&nodeInfo)) {
		// no find, connect to vpmd, request node info
		st_netfd_t vsock;
		while ( (vsock = _rms_connect_to(hostName, _st_vpmd_listen_port)) == NULL ) {
			st_usleep(100000);
		}

		char wbuf[3*MAXSYMLEN];
		put_int16(nameLen+1, wbuf);
		wbuf[2] = EPMD_PORT2_REQ;
		memcpy(wbuf+3, nodeName, nameLen);
		st_write(vsock, wbuf, nameLen+3, ST_UTIME_NO_TIMEOUT);

		int n = st_read(vsock, wbuf, 2+MAXSYMLEN, ST_UTIME_NO_TIMEOUT);
		if (n < 3) {
			// net error
			LOG_WARN("recv vpmd message error. ");
		}
		if (wbuf[0] != EPMD_PORT2_RESP || wbuf[1]) {
			// message error
			LOG_WARN("recv vpmd message error. ");
		}

		nodeInfo = MALLOC(sizeof(_rms_node_info));
		nodeInfo->nameLen = length_str(nodeName);
		nodeInfo->name = str_dup(nodeName);
		nodeInfo->hostLen = length_str(hostName);
		nodeInfo->host = str_dup(hostName);
		nodeInfo->url = str_dup(nodeUrl);
//		nodeInfo->thread = NULL;

		int pos = 2;
		nodeInfo->port = get_int16(wbuf+pos);
//		wbuf[4] = node->nodetype;
//		wbuf[5] = node->protocol;
//		put_int16(node->highvsn, wbuf + 6);
//		put_int16(node->lowvsn, wbuf + 8);
		pos += 6;
		int nl = get_int16(wbuf+pos);
		pos += nl + 2;
		int el = get_int16(wbuf+pos);
		pos += el + 2;
		nodeInfo->id = get_uint64(wbuf+pos);

		int64map_put(_st_nodeid_info_map, nodeInfo->id, nodeInfo);
		hashmap_put(_st_nodeurl_info_map, nodeInfo->url, nodeInfo);
	}

	st_netfd_t sock;
	sock = int64map_get(_st_nodeid_sock_map, nodeInfo->id);
	if (sock == NULL) {
		// create a connect to the node
		sock = _rms_connect_to(hostName, nodeInfo->port);
		if (sock == NULL) return 1;	//error

		// send NODE_CONN_REQ
		_rms_send_conn_req(sock);

		// rcv loop
		st_thread_create(_rms_rcv_thread_loop, sock, 0, 0);
		int64map_put(_st_nodeid_sock_map, nodeInfo->id, sock);
	}

	int ltn = length_str(threadName) + 1;
	int hdlen = 2 + ltn + 8;
	unsigned char hb[MAXSYMLEN + 32];
	hb[0] = NODE_RPC_MSG_NAME;
	put_int16(msg->len + hdlen, hb+1);
	put_int16(ltn, hb+3);
	strcpy((char*)hb+5, threadName);
	put_uint64(msg->f_tid, hb+5+ltn);
	struct iovec iov[2];
	iov[0].iov_base = hb;
	iov[0].iov_len = hdlen + 3;
	iov[1].iov_base = msg->data;
	iov[1].iov_len = msg->len;
	return st_writev(sock, iov, 2, ST_UTIME_NO_TIMEOUT);
}

static int _read_bytes(st_netfd_t soc, char *tobuf, int nbyte) {
	int rn, n;
	int unread;
	rn = st_read(soc, tobuf, nbyte, ST_UTIME_NO_TIMEOUT);
	while (rn < nbyte) {
		unread = nbyte - rn;
		n = st_read(soc, tobuf+rn, unread, ST_UTIME_NO_TIMEOUT);
		if (n <= 0) { // read error
			return n;
		}
		rn = rn + n;
	}
	return rn;
}

static st_netfd_t _rms_connect_to(char *addr, int port) {
	struct sockaddr_in rmt_addr;
	rmt_addr.sin_family = AF_INET;
	rmt_addr.sin_port = htons(port);
	rmt_addr.sin_addr.s_addr = inet_addr(addr);
	struct hostent *hp;
	if (rmt_addr.sin_addr.s_addr == INADDR_NONE) {
		/* not dotted-decimal */
		if ((hp = gethostbyname(addr)) == NULL) {
			LOG_WARN("can't resolve address: %s\n", addr);
			return NULL;
		}
		memcpy(&rmt_addr.sin_addr, hp->h_addr, hp->h_length);
	}

	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		LOG_WARN("socket create error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	int n = 1, ir;
	ir = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n));
	ir = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&n, sizeof(n));
	if (ir < 0) {
		close(sock);
		LOG_WARN("setsockopt error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}

	st_netfd_t cli_nfd;
	if ((cli_nfd = st_netfd_open_socket(sock)) == NULL) {
		close(sock);
		LOG_WARN("st_netfd_open_socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return NULL;
	}
	if (st_connect(cli_nfd, (struct sockaddr *) &rmt_addr, sizeof(rmt_addr), ST_UTIME_NO_TIMEOUT) < 0) {
		st_netfd_close(cli_nfd);
		LOG_WARN("st_connect to %s:%d error: %s(errno: %d)", addr, port, getLastErrorText(), getLastError());
		return NULL;
	}
	return cli_nfd;
}


static int _rms_send_conn_req(st_netfd_t sock) {
	char dataBuf[MAXSYMLEN * 2];
	int pos = 0, len;
	put_uint64(_st_this_node_id, dataBuf);
	pos += 8;
	len = length_str(_st_this_node_name);
	put_int16(len, dataBuf+pos);
	pos += 2;
	memcpy(dataBuf+pos, _st_this_node_name, len);
	pos += len;
	len = length_str(_this_hostname);
	put_int16(len, dataBuf+pos);
	pos += 2;
	memcpy(dataBuf+pos, _this_hostname, len);
	pos += len;
	put_int16(_st_rpc_server_listen_port, dataBuf+pos);
	pos += 2;

	char hb[3];
	hb[0] = NODE_CONN_REQ;
	put_int16(pos, hb+1);
	struct iovec iov[2];
	iov[0].iov_base = hb;
	iov[0].iov_len = 3;
	iov[1].iov_base = dataBuf;
	iov[1].iov_len = pos;
	return st_writev(sock, iov, 2, ST_UTIME_NO_TIMEOUT);
}

static void _rms_send_find_nodeid_req(uint64_t nodeid) {
	char hb[11];
	hb[0] = NODE_FINDID_REQ;
	put_int16(8, hb+1);
	put_uint64(nodeid, hb+3);

	st_netfd_t sock;
	uint64_t key;
	int64map_iterator *it = int64map_new_iterator(_st_nodeid_sock_map);
	while ((sock=int64map_next(it, &key))) {
		// NODE_FINDID_REQ
		st_write(sock, hb, 11, ST_UTIME_NO_TIMEOUT);
	}
	int64map_del_iterator(it);
}

static int _rms_find_nodeinfo(uint64_t nodeid, _rms_node_info **nodeinfo) {
	_rms_node_info *nodeInfo;
	uint64_t key;
	int64map_iterator *it = int64map_new_iterator(_st_nodeid_info_map);
	while ((nodeInfo=int64map_next(it, &key))) {
		if (key == nodeInfo->id) {
			*nodeinfo = nodeInfo;
			return 0;
		}
	}
	int64map_del_iterator(it);

	// in _st_nodeid_info_map, not find this tid. send search
	_rms_send_find_nodeid_req(nodeid);

	st_usleep(30000);	// sleep 30ms, and retry find
	it = int64map_new_iterator(_st_nodeid_info_map);
	while ((nodeInfo=int64map_next(it, &key))) {
		if (key == nodeInfo->id) {
			*nodeinfo = nodeInfo;
			return 0;
		}
	}
	int64map_del_iterator(it);

	return 1;
}


// This function must be call before other thread(include lua vm).
void st_rms_init(char *nodeName) {
	if (nodeName == NULL) {
		LOG_INFO("Not set node name, do not run rpc module.");
		return;	// not run rpc
	}
	_this_hostname[127] = '\0';
	gethostname(_this_hostname, 127);
	_st_this_node_name = str_dup(nodeName);
	_st_nodeid_sock_map = int64map_create(16);
	_st_nodeid_info_map = int64map_create(16);
	_st_nodeurl_info_map = hashmap_create();

	//1. start vpmd
	_start_vpmd(NULL);

	//2. start rpc server thread, accept rpc connect
	//3. send rpc server listen port and node name to vpmd
	//4. get nodeid from vpmd
	_rms_start_server();
}

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

static void _start_vpmd(char *epmd) {
	char *bindir = MALLOC(512);
	getcwd(bindir, 511);
//	erts_snprintf(bindir,512,"%s/erts-%s/bin",rootdir,tmpbuffer);

	char epmd_cmd[MAXPATHLEN + 100];
#ifdef __WIN32__
	char* arg1 = NULL;
#endif
	int result;

	if (!epmd) {
		epmd = epmd_cmd;
#ifdef __WIN32__
		erts_snprintf(epmd_cmd, sizeof(epmd_cmd), "%s" DIRSEP "vpmd", bindir);
		arg1 = "-daemon";
#else
		snprintf(epmd_cmd, sizeof(epmd_cmd), "\"%s" DIRSEP "vpmd\" -daemon", bindir);
		printf("++++++++++++++++++++ %s\n", epmd_cmd);
#endif
	}
#ifdef __WIN32__
	if (arg1 != NULL) {
		strcat(epmd, " ");
		strcat(epmd, arg1);
	}
	{
		wchar_t wcepmd[MAXPATHLEN+100];
		STARTUPINFOW start;
		PROCESS_INFORMATION pi;
		memset(&start, 0, sizeof (start));
		start.cb = sizeof (start);
		MultiByteToWideChar(CP_UTF8, 0, epmd, -1, wcepmd, MAXPATHLEN+100);

		if (!CreateProcessW(NULL, wcepmd, NULL, NULL, FALSE,
						CREATE_DEFAULT_ERROR_MODE | DETACHED_PROCESS,
						NULL, NULL, &start, &pi))
			result = -1;
		else
			result = 0;
	}
#else
	result = system(epmd);
#endif
	if (result == -1) {
//		fprintf(stderr, "Error spawning %s (error %d)\n", epmd_cmd, errno);
		exit(1);
	}
}

static int _rms_start_server() {
	int retryNum = 64;
	int sockfd;
	while ((sockfd = _rms_create_listen_socket()) < 0 && retryNum >0) {
		st_usleep(10000000);		// 10s retry
		retryNum--;
	}

	// get listen port
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	memset(&addr, 0, sizeof(addr));
	getsockname(sockfd, (struct sockaddr *)&addr, &addrlen);
	_st_rpc_server_listen_port = ntohs(addr.sin_port);
	LOG_INFO("Rpc server socket listen start, listen port is %d.", _st_rpc_server_listen_port);

	//send rpc server listen port and node name to vpmd, get nodeid from vpmd
	while (_rms_register_node() < 0) {
		st_usleep(10000000);		// 10s retry
	}

	st_netfd_t st_srvfd;
	if ((st_srvfd = st_netfd_open_socket(sockfd)) == NULL) {
		close(sockfd);
		THROW(net_exception, "st_netfd_open_socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return -1;
	}

	// start accpet thread
	st_thread_create(_rms_server_accept_loop, st_srvfd, 0, 0);
	return 0;
}

static int _rms_create_listen_socket() {
	int sockfd;
	if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		LOG_WARN("Create socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return -1;
	}
	int n = 1, ir;
	ir = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&n, sizeof(n));
	ir = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&n, sizeof(n));
	if (ir < 0) {
		close(sockfd);
		LOG_WARN("setsockopt error: %s(errno: %d)", getLastErrorText(), getLastError());
		return -1;
	}
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = 0;		// auto listen port
	if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		close(sockfd);
		LOG_WARN("Bind socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return -1;
	}
	if (listen(sockfd, 256) == -1) {
		close(sockfd);
		LOG_WARN("Listen socket error: %s(errno: %d)", getLastErrorText(), getLastError());
		return -1;
	}

	return sockfd;
}

static int _rms_register_node() {
	st_netfd_t cli_nfd = _rms_connect_to("127.0.0.1", _st_vpmd_listen_port);  // vpmd in localhost

	//send rpc server listen port and node name to vpmd
	int offset = 2;				//2byte header, offset from 2
	char wbuf[3*MAXSYMLEN]; 	/* Buffer for writing */
	wbuf[offset++] = EPMD_ALIVE2_REQ;
	put_int16(_st_rpc_server_listen_port, wbuf + offset);
	offset += 2;
	wbuf[offset++] = 77;	// 77 = normal Erlang node
	wbuf[offset++] = 0;		// 0 = tcp/ip-v4
	put_int16(5, wbuf + offset);
	offset += 2;
	put_int16(5, wbuf + offset);
	offset += 2;
	put_int16(length_str(_st_this_node_name), wbuf + offset);
	offset += 2;
	offset += copy_str(wbuf + offset, _st_this_node_name);
	put_int16(4, wbuf + offset);
	offset += 2;
	memcpy(wbuf + offset, "vpmd", 4);
	offset += 4;
	// packet len, 2 byte
	put_int16(offset-2, wbuf);
	if (st_write(cli_nfd, wbuf, offset, -1) != offset) {
		st_netfd_close(cli_nfd);
		LOG_WARN("st_write error: %s(errno: %d)", getLastErrorText(), getLastError());
		return -1;
	}

	//get nodeid from vpmd
	char rbuf[12];
	int rn = st_read(cli_nfd, rbuf, 12, -1);
	if (rn < 12 || rbuf[0] != EPMD_ALIVE2_RESP) {
		LOG_WARN("st_read error: %s(errno: %d)", getLastErrorText(), getLastError());
		st_netfd_close(cli_nfd);
		return -1;
	}
	_st_this_node_id = get_uint64(rbuf + 4);
	LOG_INFO("Rpc node register finish, node name = %s and node id = %lu.", _st_this_node_name, _st_this_node_id);

	// Create a thread to check the connection be closed.
	// If the connection be closed, this node unregister from vpmd.
	st_thread_create(_rms_register_node_loop, cli_nfd, 0, 0);
	return 0;
}

static void *_rms_register_node_loop(void *arg) {
	st_netfd_t cli_nfd = (st_netfd_t)arg;
	char rbuf[1];
	int rn;
	while (1) {
		rn = st_read(cli_nfd, rbuf, 1, -1);
		if (rn <= 0) {
			// connection error or close, re-register node
			LOG_WARN("connection error or close: %s(errno: %d)", getLastErrorText(), getLastError());
			st_netfd_close(cli_nfd);
			while (_rms_register_node() < 0) {
				st_usleep(10000000);		// 10s
			}
			return NULL;
		}
	}
	return NULL;
}

static void *_rms_server_accept_loop(void *arg) {
	st_netfd_t st_srvfd = arg;
	// accept loop
	while (1) {
		struct sockaddr_in cli_add;
		int n = sizeof(cli_add);
		st_netfd_t st_clifd = st_accept(st_srvfd, (struct sockaddr *)&cli_add, &n, ST_UTIME_NO_TIMEOUT);
		if (st_clifd == NULL) {
			LOG_WARN("socket accept error: %s(errno: %d)", getLastErrorText(), getLastError());
			st_netfd_close(st_srvfd);
			// socket error, current thread termination, try to recreate server
			st_usleep(5000000);		// 5s
			_rms_start_server();

			//TODO the already connected do not destroy
			return NULL;
		}

		// rpc connect thread
		st_thread_create(_rms_rcv_thread_loop, st_clifd, 0, 0);
	}
	return NULL;
}


/*
 * NODE_CONN_REQ
 * byte 1			message code, NODE_CONN_REQ
 * byte 2			data length
 * byte 8			node id
 * byte 2			nameLen
 * byte nameLen		node name
 * byte 2			hostLen
 * byte hostLen		host name
 * byte 2			listen port
 *
 * NODE_CONN_RESP
 * byte 1			message code, NODE_CONN_RESP
 * byte 1			success code
 * byte 8			node id
 * byte 2			data length
 * byte 2			nameLen
 * byte nameLen		node name
 * byte 2			hostLen
 * byte hostLen		host name
 * byte 2			listen port
 *      .........
 * byte 2			nameLen
 * byte nameLen		node name
 * byte 2			hostLen
 * byte hostLen		host name
 * byte 2			listen port
 *
 * NODE_RPC_MSG
 * byte 1			message code, NODE_RPC_MSG
 * byte 2			dataLen
 * byte 8			to thread id
 * byte 8			from thread id
 * byte dataLen		message data
 *
 * NODE_FINDID_REQ
 * byte 1			message code, NODE_FINDID_REQ
 * byte 2			dataLen(=8)
 * byte 8			find thread id
 */
static void *_rms_rcv_thread_loop(void *arg) {
	st_netfd_t soc = (st_netfd_t) arg;

	_rms_node_info *nodeInfo = NULL;
	_rms_node_info *nodeInfoIter;

	char readBuf[INBUF_SIZE];
	// read loop
	int msgcode, dataLen, rn;
	uint64_t toid, fromid;
	int buflen = INBUF_SIZE;
	char *data = MALLOC(buflen);
	while ((rn = _read_bytes(soc, readBuf, 3)) > 0) {
		msgcode = readBuf[0];
		dataLen = get_int16(readBuf+1);
		// message data
		if (dataLen > buflen) {
			REALLOC(data, dataLen);
			buflen = dataLen;
		}
		rn = _read_bytes(soc, data, dataLen);
		if (rn != dataLen) {
			LOG_WARN("error message data: %s(errno: %d)", getLastErrorText(), getLastError());
			break;
		}

		switch (msgcode) {
		case NODE_CONN_REQ: {
			if (nodeInfo) {
				FREE(nodeInfo->name);
				FREE(nodeInfo->host);
				FREE(nodeInfo->url);
				FREE(nodeInfo);
			}
			nodeInfo = MALLOC(sizeof(_rms_node_info));

			int pos = 0;
			nodeInfo->id = get_uint64(data);
			pos += 8;
			int charLen = get_int16(data+pos);
			pos += 2;
			nodeInfo->nameLen = charLen;
			nodeInfo->name = str_ndup(data+pos, charLen);
			pos += charLen;
			charLen = get_int16(data+pos);
			pos += 2;
			nodeInfo->hostLen = charLen;
			nodeInfo->host = str_ndup(data+pos, charLen);
			pos += charLen;
			nodeInfo->port = get_int16(data+pos);
			int nnl = nodeInfo->nameLen + nodeInfo->hostLen + 2;
			nodeInfo->url = MALLOC(nnl);
			snprintf(nodeInfo->url, nnl, "%s@%s", nodeInfo->name, nodeInfo->host);
			int64map_put(_st_nodeid_sock_map, nodeInfo->id, soc);
			int64map_put(_st_nodeid_info_map, nodeInfo->id, nodeInfo);
			hashmap_put(_st_nodeurl_info_map, nodeInfo->url, nodeInfo);

			//response,  NODE_CONN_RESP
			int writeNum = 0;	// out data byte size
			// node info loop
			uint64_t key;
			int64map_iterator *it = int64map_new_iterator(_st_nodeid_info_map);
			while ((nodeInfoIter=int64map_next(it, &key))) {
				writeNum += nodeInfoIter->nameLen + nodeInfoIter->hostLen + 4;
			}
			int64map_del_iterator(it);
			char *writeBuf = MALLOC(writeNum);
			int posw = 0;
			it = int64map_new_iterator(_st_nodeid_info_map);
			while ((nodeInfoIter=int64map_next(it, &key))) {
				put_int16(nodeInfoIter->nameLen, writeBuf+posw);
				memcpy(writeBuf+posw+2, nodeInfoIter->name, nodeInfoIter->nameLen);
				posw += 2 + nodeInfoIter->nameLen;
				put_int16(nodeInfoIter->hostLen, writeBuf+posw);
				memcpy(writeBuf+posw+2, nodeInfoIter->host, nodeInfoIter->hostLen);
				posw += 2 + nodeInfoIter->hostLen;
				put_int16(nodeInfoIter->port, writeBuf+posw);
				posw += 2;
				put_uint64(nodeInfoIter->id, writeBuf+posw);
				posw += 8;
			}
			int64map_del_iterator(it);
			// header
			unsigned char hb[12];
			hb[0] = NODE_CONN_RESP;
			put_int16(writeNum+9, hb+1);
			hb[3] = 0;	//success code	//TODO when 1, error message
			put_uint64(_st_this_node_id, hb+4); // write this node id
			struct iovec iov[2];
			iov[0].iov_base = hb;
			iov[0].iov_len = 12;
			iov[1].iov_base = writeBuf;
			iov[1].iov_len = writeNum;
			st_writev(soc, iov, 2, ST_UTIME_NO_TIMEOUT);
			break;
		}
		case NODE_CONN_RESP: {
			if (data[0] != 0) {		// connect request error
				//TODO log out error message
				goto connectEnd;
			}
			uint64_t fromNodeId = get_uint64(data+1);
			nodeInfo = int64map_get(_st_nodeid_info_map, fromNodeId);
//			if (nodeInfo->id != fromNodeId) {
//				LOG_WARN("connecting node id error, request id=%lu and response id=%lu", nodeInfo->id, fromNodeId);
//				goto connectEnd;
//			}

			int pn = 9;
			_rms_node_info *nitmp;
			while (pn < dataLen) {
				nodeInfoIter = MALLOC(sizeof(_rms_node_info));
				nodeInfoIter->nameLen = get_int16(data + pn);
				pn += 2;
				nodeInfoIter->name = str_ndup(data + pn, nodeInfoIter->nameLen);
				pn += nodeInfoIter->nameLen;
				nodeInfoIter->hostLen = get_int16(data + pn);
				pn += 2;
				nodeInfoIter->host = str_ndup(data + pn, nodeInfoIter->hostLen);
				pn += nodeInfoIter->hostLen;
				nodeInfoIter->port = get_int16(data + pn);
				pn += 2;
				nodeInfoIter->id = get_uint64(data + pn);
//				pn += 8;
				int nnl = nodeInfoIter->nameLen + nodeInfoIter->hostLen + 2;
				nodeInfoIter->url = MALLOC(nnl);
				snprintf(nodeInfoIter->url, nnl, "%s@%s", nodeInfoIter->name, nodeInfoIter->host);

				nitmp = int64map_get(_st_nodeid_info_map, nodeInfoIter->id);
				if (nitmp) {
					FREE(nitmp->name);
					FREE(nitmp->host);
					FREE(nitmp->url);
					FREE(nitmp);
				}
				int64map_put(_st_nodeid_info_map, nodeInfoIter->id, nodeInfoIter);
				hashmap_put(_st_nodeurl_info_map, nodeInfoIter->url, nodeInfoIter);
			}
			break;
		}
		case NODE_RPC_MSG: {
			// toid 8byte, fromid 8byte
			toid = get_uint64(data);
			fromid = get_uint64(data + 8);

			st_thread_msg_t msg = st_create_msg(data+16, dataLen-16);
			msg->f_tid = fromid;
			st_thread_t tothread = st_get_thread(toid);
			if (tothread)
				st_send_msg(tothread, msg);
			else
				LOG_WARN("thread no found, toid=%lu, fromid=%lu", toid, fromid);
			break;
		}
		case NODE_RPC_MSG_NAME: {
			// threadName tnlen byte, fromid 8byte
			int tnlen = get_int16(data);
			char *threadName = data + 2;
			toid = st_get_reg_tid(threadName);
			fromid = get_uint64(data + 2 + tnlen);

			st_thread_msg_t msg = st_create_msg(data + 10 + tnlen, dataLen - 10 - tnlen);
			msg->f_tid = fromid;
			st_thread_t tothread = st_get_thread(toid);
			if (tothread)
				st_send_msg(tothread, msg);
			else
				LOG_WARN("thread no found, toThreadName=%s, fromid=%lu", threadName, fromid);
			break;
		}
		case NODE_FINDID_REQ: {
			uint64_t findNid = get_uint64(data);//ST_NODEID(findTid);
			//response,  NODE_CONN_RESP
			int writeNum = 0;	// out data byte size
			// node info loop
			uint64_t key;
			int hasTid = 0;
			int64map_iterator *it = int64map_new_iterator(_st_nodeid_info_map);
			while ((nodeInfoIter=int64map_next(it, &key))) {
				writeNum += nodeInfoIter->nameLen + nodeInfoIter->hostLen + 4;
				if (nodeInfoIter->id == findNid) hasTid = 1;
			}
			int64map_del_iterator(it);
			if (!hasTid) {
				// if don't find tid, no response
				_rms_send_find_nodeid_req(findNid);
				break;
			}

			char *writeBuf = MALLOC(writeNum);
			it = int64map_new_iterator(_st_nodeid_info_map);
			int posw = 0;
			while ((nodeInfoIter=int64map_next(it, &key))) {
				put_int16(nodeInfoIter->nameLen, writeBuf+posw);
				memcpy(writeBuf+posw+2, nodeInfoIter->name, nodeInfoIter->nameLen);
				posw += 2 + nodeInfoIter->nameLen;
				put_int16(nodeInfoIter->hostLen, writeBuf+posw);
				memcpy(writeBuf+posw+2, nodeInfoIter->host, nodeInfoIter->hostLen);
				posw += 2 + nodeInfoIter->hostLen;
				put_int16(nodeInfoIter->port, writeBuf+posw);
				posw += 2;
				put_uint64(nodeInfoIter->id, writeBuf+posw);
				posw += 8;
			}
			int64map_del_iterator(it);
			// header
			unsigned char hb[4];
			hb[0] = NODE_CONN_RESP;
			put_int16(writeNum, hb+1);
			hb[3] = 0;	//success code	//TODO when 1, error message
			struct iovec iov[2];
			iov[0].iov_base = hb;
			iov[0].iov_len = 4;
			iov[1].iov_base = writeBuf;
			iov[1].iov_len = writeNum;
			st_writev(soc, iov, 2, ST_UTIME_NO_TIMEOUT);
			break;
		}
		default:
			LOG_WARN("error message code: %d", msgcode);
			break;
		}
	}
	// connect close or error
//	LOG_WARN("connection error: %s(errno: %d)", getLastErrorText(), getLastError());
	connectEnd:
	FREE(data);
	st_netfd_close(soc);
//	if (nodeInfo) {
		int64map_remove(_st_nodeid_sock_map, nodeInfo->id);
		int64map_remove(_st_nodeid_info_map, nodeInfo->id);
		hashmap_remove(_st_nodeurl_info_map, nodeInfo->url);
		FREE(nodeInfo->name);
		FREE(nodeInfo->host);
		FREE(nodeInfo->url);
		FREE(nodeInfo);
//	}
	return NULL;
}


uint64_t st_get_this_node_id() {
	return _st_this_node_id;
}

uint16_t st_get_vpmd_port() {
	return _st_vpmd_listen_port;
}

void st_set_vpmd_port(uint16_t port) {
	_st_vpmd_listen_port = port;
}

