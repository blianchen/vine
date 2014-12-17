
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdarg.h>

#define LOG_MAX_CHAR_NUM 1024

// log输出级别
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_OFF   100

// log前缀
#define LOG_PREFIX_LEN 6		//WARN DEBUG INFO OFF
#define LOG_FIX_WARN   "WARN "
#define LOG_FIX_DEBUG  "DEBUG"
#define LOG_FIX_INFO   "INFO "
#define LOG_FIX_OFF    "OFF  "

// log日期
#define LOG_TIME_FMT " %y.%m.%d %H:%M:%S "
#define LOG_TIME_LEN 20		//yy.mm.dd hh:MM:ss 包括前后空格

#define LOG_FILE_NAME_MAX_LEN 64	//log文件名最大字符数

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MULTI_THREAD
#define LOCK_LOGGING() lockLoggingMutex()
#define UNLOCK_LOGGING() unlockLoggingMutex()
#else
#define LOCK_LOGGING()
#define UNLOCK_LOGGING()
#endif

////// log级别
//extern void log_set_level(const char *levelName);
//extern char* log_get_level();

//使用这组宏可以附加文件名和行号
#define LOG_WARN(msg, ...)  _log_warn(__func__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )
#define LOG_DEBUG(msg, ...) _log_debug(__func__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )
#define LOG_INFO(msg, ...)  _log_info(__func__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )


//// log输出函数，格式化字符跟printf相同
#define log_warn(msg, ...) _log_warn(NULL, NULL, 0, msg, ##__VA_ARGS__)
#define log_debug(msg, ...)  _log_debug(NULL, NULL, 0, msg, ##__VA_ARGS__)
#define log_info(msg, ...)  _log_info(NULL, NULL, 0, msg, ##__VA_ARGS__)

/////// debug info //////////////////////
void _log_warn(const char* fun, const char* file, int line, const char *msgstr, ...);
void _log_debug(const char* fun, const char* file, int line, const char *msgstr, ...);
void _log_info(const char* fun, const char* file, int line, const char *msgstr, ...);


//// 初始化，只在第一次使用时调用一次
void log_init();
//// 格式化，附加log级别和时间
void log_format(char *out, const char *prefix, const char *in, va_list args_tr, const char* fun, const char* file, int line);
//// 默认的log输出函数，输出到控制台
void log_out_console(const char *msg);
//// 文件输出
void log_out_file(const char *msg);
void log_out_file_close(void);

//// log输出函数指针
typedef void (*LogOutFun)(const char *);


// 输出函数名（LogOutFunMap.name）的最大字符数
#define LOG_OUT_FUN_NAME_LEN 20
#define LOG_CFG_ROW_LEN 512

typedef struct {
	char *name;
	LogOutFun fun;
} LogOutFunMap;

//// 输出函数的配置名和函数指针
static const LogOutFunMap log_out_fun_dic[] = {
		{"CONSOLE", log_out_console},
		{"FILE", log_out_file}
};


#ifdef MULTI_THREAD
int lockLoggingMutex();
int unlockLoggingMutex();
#endif

#ifdef __cplusplus
}
#endif

#endif
