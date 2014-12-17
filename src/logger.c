/*
 * logger.c
 *
 *  Created on: 2014年9月7日
 *      Author: blc
 */

#include <logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <str.h>
#include <utils.h>

#ifdef MULTI_THREAD
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#endif

static int log_level;
static LogOutFun log_out_fun = NULL;

///////////////// log 配置文件中内容 ////////////////////
//log函数名
static char log_out_fun_name[LOG_OUT_FUN_NAME_LEN];
//log级别名
static char log_level_name[LOG_PREFIX_LEN];
//log文件名
static char log_file_name[LOG_FILE_NAME_MAX_LEN];



void log_set_level(const char *levelName) {
	strcpy(log_level_name, levelName);

	if (strcmp(levelName, "DEBUG") == 0) {
		log_level = LOG_LEVEL_DEBUG;
	} else if (strcmp(levelName, "WARN") == 0) {
		log_level = LOG_LEVEL_WARN;
	} else if (strcmp(levelName, "INFO") == 0) {
		log_level = LOG_LEVEL_INFO;
	} else if (strcmp(levelName, "OFF") == 0) {
		log_level = LOG_LEVEL_OFF;
	} else {
		// 设置的输出级别有问题
		printf("设置的log输出级别有问题。 %s \n", levelName);
		log_level = LOG_LEVEL_OFF;
	}
}
char* log_get_level() {
	return log_level_name;
}

void log_set_out_fun(const char *funName) {
	strcpy(log_out_fun_name, funName);

	int i;
	for (i=0; i<sizeof(log_out_fun_dic)/sizeof(LogOutFunMap); i++) {
		if (strcmp(log_out_fun_dic[i].name, funName) == 0) {
			log_out_fun = log_out_fun_dic[i].fun;
			return;
		}
	}
	// 设置的输出函数名有问题，改成控制台输出
	printf("设置的log输出函数名有问题。 %s \n", funName);
	log_level = LOG_LEVEL_OFF;
	log_out_fun = log_out_console;
}
char* log_get_out_fun() {
	return log_out_fun_name;
}

void _log_warn(const char* fun, const char* file, int line, const char *msgstr, ...) {
	if (LOG_LEVEL_WARN < log_level || !msgstr) return;
	if (!log_out_fun) {
		log_init();
	}
	char buf[LOG_MAX_CHAR_NUM];
	va_list args_tr;
	va_start(args_tr, msgstr);
	if (fun) {
		log_format(buf, LOG_FIX_WARN, msgstr, args_tr, fun, file, line);
	} else {
		log_format(buf, LOG_FIX_WARN, msgstr, args_tr, NULL, NULL, 0);
	}
	va_end(args_tr);

	log_out_fun(buf);
}

void _log_debug(const char* fun, const char* file, int line, const char *msgstr, ...) {
	if (LOG_LEVEL_DEBUG < log_level || !msgstr) return;
	if (!log_out_fun) {
		log_init();
	}

	char buf[LOG_MAX_CHAR_NUM];
	va_list args_tr;
	va_start(args_tr, msgstr);
	if (fun) {
		log_format(buf, LOG_FIX_DEBUG, msgstr, args_tr, fun, file, line);
	} else {
		log_format(buf, LOG_FIX_DEBUG, msgstr, args_tr, NULL, NULL, 0);
	}
	va_end(args_tr);

	log_out_fun(buf);
}

void _log_info(const char* fun, const char* file, int line, const char *msgstr, ...) {
	if (LOG_LEVEL_INFO < log_level || !msgstr) return;
	if (!log_out_fun) {
		log_init();
	}

	char buf[LOG_MAX_CHAR_NUM];
	va_list args_tr;
	va_start(args_tr, msgstr);
	if (fun) {
		log_format(buf, LOG_FIX_INFO, msgstr, args_tr, fun, file, line);
	} else {
		log_format(buf, LOG_FIX_INFO, msgstr, args_tr, NULL, NULL, 0);
	}
	va_end(args_tr);

	log_out_fun(buf);
}


void log_init() {
	memset(log_out_fun_name, '\0', LOG_OUT_FUN_NAME_LEN);

	FILE *cfgFile = fopen("logcfg.ini", "r");
	if (!cfgFile) {
		log_level = LOG_LEVEL_OFF;
		log_out_fun = NULL;
		puts("****ERROR****:Do not find file logcfg.ini");
		exit(1);
		return;
	}
	// 分析行
	char delims[] = "=";
	while (!feof(cfgFile)) {
		char strLine[LOG_CFG_ROW_LEN];
		if (!fgets(strLine, LOG_CFG_ROW_LEN, cfgFile))
			break;
		if (strLine[0] == '#' || str_trim(strLine)[0] == '\0') continue;
		// 分割'='
		char *name;
		char *value;
		name = str_trim(strtok(strLine, delims));
		if (!name || strlen(name) == 0) continue;
		value = str_trim(strtok(NULL, delims));
		if (!value || strlen(value) == 0) continue;
//		char uperValue[LOG_OUT_FUN_NAME_LEN];
//		STRUPR(value, uperValue);
		if (strcmp(name, "level") == 0) {
			log_set_level(value);
		} else if (strcmp(name, "outfun") == 0) {
			log_set_out_fun(value);
		} else if (strcmp(name, "logfile") == 0) {
			strcpy(log_file_name, value);
		} else {
			printf("无效的配置项。%s \n", name);
		}
	}
	fclose(cfgFile);
}


void log_format(char *out, const char *prefix, const char *in, va_list args, const char* fun, const char* file, int line) {
	time_t timer = time(NULL);
	struct tm *t = localtime(&timer);
	char timeBuf[LOG_TIME_LEN];
	strftime(timeBuf, LOG_TIME_LEN, LOG_TIME_FMT, t);

	char *cp = out;
	strcpy(cp, prefix);
	cp += LOG_PREFIX_LEN - 1;
	strcpy(cp, timeBuf);
	cp += LOG_TIME_LEN - 1;
	if (fun) {
		int dbinf = snprintf(cp, 256, "(%s@%s.%d) ", fun, file, line);
		cp += dbinf;
	}
	int maxPrint = LOG_MAX_CHAR_NUM - (cp-out);
	int pn = vsnprintf(cp, maxPrint, in, args);  //当实际字符串长度超过maxPrint， 该函数返回的是实际长度，而不是maxPrint

	//// 字符串较长时，多余部分会被截掉，这里把最后3个字符改为"..."
	if (pn > maxPrint) {
		int i = LOG_MAX_CHAR_NUM - 4;
		while (i < LOG_MAX_CHAR_NUM - 1) {
			out[i++] = '.';
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////// 输出函数 /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void log_out_console(const char *msg) {
	LOCK_LOGGING();
	printf("%s\n", msg);
	UNLOCK_LOGGING();
}


FILE* log_file = NULL;
void log_out_file(const char *msg) {

	LOCK_LOGGING();

	if (!log_file) {	//打开log文件
		log_file = fopen(log_file_name, "a");
		if (log_file) {
			atexit(log_out_file_close);  //正常退出时关闭文件
		} else {
			printf("创建log文件出错(%s)。%s \n", getLastErrorText(), log_file_name);
			return;
		}
	}

	fwrite(msg, sizeof(char), strlen(msg), log_file);
	fwrite("\n", 1, 1, log_file);
	fflush(log_file);

	UNLOCK_LOGGING();
}

void log_out_file_close(void) {
	printf("log_out_file_close");
	if (log_file) {
		fclose(log_file);
	}
}



///////////////////////////////////////////////////////////////////////////
//////////////////////////// 多线程支持 ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
#ifdef MULTI_THREAD

#ifdef WIN32
HANDLE log_printfMutexHandle = NULL;
#else
pthread_mutex_t log_printfMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/** Obtains a lock on the logging mutex. */
int lockLoggingMutex() {
#ifdef WIN32
	switch (WaitForSingleObject(log_printfMutexHandle, INFINITE)) {
	case WAIT_ABANDONED:
		_tprintf(TEXT("Logging mutex was abandoned.\n"));
		return -1;
	case WAIT_FAILED:
		_tprintf(TEXT("Logging mutex wait failed.\n"));
		return -1;
	case WAIT_TIMEOUT:
		_tprintf(TEXT("Logging mutex wait timed out.\n"));
		return -1;
	default:
		/* Ok */
		break;
	}
#else
	if (pthread_mutex_lock(&log_printfMutex)) {
		printf("Failed to lock the Logging mutex. %s\n", getLastErrorText());
		return -1;
	}
#endif

	return 0;
}

/** Releases a lock on the logging mutex. */
int unlockLoggingMutex() {
#ifdef WIN32
	if (!ReleaseMutex(log_printfMutexHandle)) {
		_tprintf(TEXT("Failed to release logging mutex. %s\n"), getLastErrorText());
		return -1;
	}
#else
	if (pthread_mutex_unlock(&log_printfMutex)) {
		printf("Failed to unlock the Logging mutex. %s\n", getLastErrorText());
		return -1;
	}
#endif
	return 0;
}

#endif
