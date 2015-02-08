/*
 * threadmsg.c
 *
 *  Created on: Jan 23, 2015
 *      Author: blc
 */

#include <string.h>

#include "common.h"


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

int st_get_data(_st_thread_msg_t *msg, char **data) {
	*data = msg->data;
	return msg->len;
}

st_tid_t st_get_fromtid(_st_thread_msg_t *msg) {
	return msg->f_tid;
}



void st_send_msg(_st_thread_t *thread, _st_thread_msg_t *msg) {
	_st_thread_msg_queue_t *mq = MALLOC(sizeof(_st_thread_msg_queue_t));
	msg->refcnt++;
	msg->f_tid = ST_MAKE_TID(_ST_CURRENT_THREAD()->sid);
	mq->msg = msg;
	ST_APPEND_LINK(mq, thread->msg_q);

	//Add the recv thread to runnable queuel
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
