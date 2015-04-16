
#ifndef __ST_THREAD_H__
#define __ST_THREAD_H__

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <stdint.h>

#define ST_VERSION	    "1.9"
#define ST_VERSION_MAJOR    1
#define ST_VERSION_MINOR    9

/* Undefine this to remove the context switch callback feature. */
//#define ST_SWITCH_CB

#ifndef ETIME
#define ETIME ETIMEDOUT
#endif

#define ST_MAX_THREAD_NUM 0x4000	// 14 bit,  max thread num:16383

#ifndef ST_UTIME_NO_TIMEOUT
#define ST_UTIME_NO_TIMEOUT ((st_utime_t) -1LL)
#endif

#ifndef ST_UTIME_NO_WAIT
#define ST_UTIME_NO_WAIT 0
#endif

#define ST_EVENTSYS_DEFAULT 0
#define ST_EVENTSYS_SELECT  1
#define ST_EVENTSYS_POLL    2
#define ST_EVENTSYS_ALT     3

#ifdef __cplusplus
extern "C" {
#endif


// tid, use in thread message. add by blc
typedef uint64_t st_tid_t;  /* tid = sid(int14) + nodeid(int50);  nodeid = nodesid(int8) + vpmdid(int42) */
#define ST_MAKE_TID(sid)	( ((uint64_t)(sid) << 50) | st_get_this_node_id() )
#define ST_NODEID(tid) 		( (tid) & 0x3FFFFFFFFFFFF )
#define ST_SID(tid) 		( ((tid) >> 50) & 0x3fff )
#define ST_IS_INTERNAL(tid)	( ST_NODEID(tid) == st_get_this_node_id() )

typedef struct _st_thread_msg * st_thread_msg_t;

typedef unsigned long long st_utime_t;
typedef struct _st_thread * st_thread_t;
typedef struct _st_cond * st_cond_t;
typedef struct _st_mutex * st_mutex_t;
typedef struct _st_netfd * st_netfd_t;
#ifdef ST_SWITCH_CB
typedef void (*st_switch_cb_t)(void);
#endif

extern int st_init(void);
extern int st_getfdlimit(void);

extern int st_set_eventsys(int eventsys);
extern int st_get_eventsys(void);
extern const char *st_get_eventsys_name(void);

#ifdef ST_SWITCH_CB
extern st_switch_cb_t st_set_switch_in_cb(st_switch_cb_t cb);
extern st_switch_cb_t st_set_switch_out_cb(st_switch_cb_t cb);
#endif

extern st_thread_t st_thread_self(void);
extern void st_thread_exit(void *retval);
extern int st_thread_join(st_thread_t thread, void **retvalp);
extern void st_thread_interrupt(st_thread_t thread);
extern st_thread_t st_thread_create(void *(*start)(void *arg), void *arg, int joinable, int stack_size);
extern int st_randomize_stacks(int on);
extern int st_set_utime_function(st_utime_t (*func)(void));

extern st_utime_t st_utime(void);
extern st_utime_t st_utime_last_clock(void);
extern int st_timecache_set(int on);
extern time_t st_time(void);
extern int st_usleep(st_utime_t usecs);
extern int st_sleep(int secs);
extern st_cond_t st_cond_new(void);
extern int st_cond_destroy(st_cond_t cvar);
extern int st_cond_timedwait(st_cond_t cvar, st_utime_t timeout);
extern int st_cond_wait(st_cond_t cvar);
extern int st_cond_signal(st_cond_t cvar);
extern int st_cond_broadcast(st_cond_t cvar);
extern st_mutex_t st_mutex_new(void);
extern int st_mutex_destroy(st_mutex_t lock);
extern int st_mutex_lock(st_mutex_t lock);
extern int st_mutex_unlock(st_mutex_t lock);
extern int st_mutex_trylock(st_mutex_t lock);

extern int st_key_create(int *keyp, void (*destructor)(void *));
extern int st_key_getlimit(void);
extern int st_thread_setspecific(int key, void *value);
extern void *st_thread_getspecific(int key);

extern st_netfd_t st_netfd_open(int osfd);
extern st_netfd_t st_netfd_open_socket(int osfd);
extern void st_netfd_free(st_netfd_t fd);
extern int st_netfd_close(st_netfd_t fd);
extern int st_netfd_fileno(st_netfd_t fd);
extern void st_netfd_setspecific(st_netfd_t fd, void *value, void (*destructor)(void *));
extern void *st_netfd_getspecific(st_netfd_t fd);
extern int st_netfd_serialize_accept(st_netfd_t fd);
extern int st_netfd_poll(st_netfd_t fd, int how, st_utime_t timeout);

extern int st_poll(struct pollfd *pds, int npds, st_utime_t timeout);
extern st_netfd_t st_accept(st_netfd_t fd, struct sockaddr *addr, int *addrlen, st_utime_t timeout);
extern int st_connect(st_netfd_t fd, const struct sockaddr *addr, int addrlen, st_utime_t timeout);
extern ssize_t st_read(st_netfd_t fd, void *buf, size_t nbyte, st_utime_t timeout);
extern ssize_t st_read_fully(st_netfd_t fd, void *buf, size_t nbyte, st_utime_t timeout);
extern int st_read_resid(st_netfd_t fd, void *buf, size_t *resid, st_utime_t timeout);
extern ssize_t st_readv(st_netfd_t fd, const struct iovec *iov, int iov_size, st_utime_t timeout);
extern int st_readv_resid(st_netfd_t fd, struct iovec **iov, int *iov_size, st_utime_t timeout);
extern ssize_t st_write(st_netfd_t fd, const void *buf, size_t nbyte, st_utime_t timeout);
extern int st_write_resid(st_netfd_t fd, const void *buf, size_t *resid, st_utime_t timeout);
extern ssize_t st_writev(st_netfd_t fd, const struct iovec *iov, int iov_size, st_utime_t timeout);
extern int st_writev_resid(st_netfd_t fd, struct iovec **iov, int *iov_size, st_utime_t timeout);
extern int st_recvfrom(st_netfd_t fd, void *buf, int len, struct sockaddr *from, int *fromlen, st_utime_t timeout);
extern int st_sendto(st_netfd_t fd, const void *msg, int len, const struct sockaddr *to, int tolen, st_utime_t timeout);
extern int st_recvmsg(st_netfd_t fd, struct msghdr *msg, int flags, st_utime_t timeout);
extern int st_sendmsg(st_netfd_t fd, const struct msghdr *msg, int flags, st_utime_t timeout);
extern st_netfd_t st_open(const char *path, int oflags, mode_t mode);


//////////////// thread message, add by blc
extern st_thread_t st_get_thread(st_tid_t tid);
extern st_tid_t st_get_tid(st_thread_t thread);
extern int st_reg_tid(char *name, st_tid_t tid);
extern int st_unreg_tid(char *name);
extern char** st_get_reg_names();
extern st_tid_t st_get_reg_tid(char *name);

extern void st_send_msg(st_thread_t thread, st_thread_msg_t msg);
extern int st_send_msg_by_tid(st_tid_t tid, st_thread_msg_t msg);
extern int st_send_msg_by_name(char *nodeUrl, char *threadName, st_thread_msg_t msg);
extern st_thread_msg_t st_recv_msg();
extern int st_has_msg(st_thread_t thread);

extern st_thread_msg_t st_create_msg(const char *data, int len, st_tid_t fromTid);
extern void st_destroy_msg(st_thread_msg_t msg);
extern int st_msg_data(st_thread_msg_t msg, char **data);
extern st_tid_t st_msg_fromtid(st_thread_msg_t msg);

extern uint64_t st_get_this_node_id();
extern void st_rms_init();
extern uint16_t st_get_vpmd_port();
extern void st_set_vpmd_port(uint16_t port);


#ifdef DEBUG
extern void _st_show_thread_stack(st_thread_t thread, const char *messg);
extern void _st_iterate_threads(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* !__ST_THREAD_H__ */
