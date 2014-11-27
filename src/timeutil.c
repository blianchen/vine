
#include <timeutil.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include <sys/select.h>

#include <str.h>
#include <utils.h>
#include <exception/sys_exception.h>

/**
 * Implementation of the Time util
 */

#define TM_GMTOFF tm_gmtoff

/* Counter the number of leap year in the range [0, y). The |y| is the
 year, including century (e.g., 2012) */
int count_leap_year(int y) {
	y -= 1;
	return y / 4 - y / 100 + y / 400;
}

/* Returns nonzero if the |y| is the leap year. The |y| is the year,
 including century (e.g., 2012) */
int is_leap_year(int y) {
	return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}


time_t time_toTimestamp(const char *s) {
	if (STR_DEF(s)) {
		struct tm t = { };
		if (time_toDateTime(s, &t)) {
			t.tm_year -= 1900;
			time_t offset = t.TM_GMTOFF;
			return timegm(&t) - offset;
		}
	}
	return 0;
}

// time_toDateTime function
#include "todatetime.rec"


//struct tm *time_toDateTime(const char *s, struct tm *t) {
////	int n = sscanf(s, "%4d-%2d-%2d-%2d-%2d-%2d",
//	int n = sscanf(s, "%d-%d-%d %d:%d:%d",
//					 &t->tm_year,
//					 &t->tm_mon,
//					 &t->tm_mday,
//					 &t->tm_hour,
//					 &t->tm_min,
//					 &t->tm_sec);
//	if (n == EOF) {
//		return NULL;
//	}
//
//	t->tm_year -= 1900;
//	t->tm_mon --;
//	t->tm_isdst = -1;
//	return t;
//}


char *time_toString(time_t time, char result[20]) {
    assert(result);
    char x[2];
    struct tm ts = {.tm_isdst = -1};
    gmtime_r(&time, &ts);
    memcpy(result, "YYYY-MM-DD HH:MM:SS\0", 20);
    /*              0    5  8  11 14 17 */
    i2a((ts.tm_year+1900)/100);
    result[0] = x[0];
    result[1] = x[1];
    i2a((ts.tm_year+1900)%100);
    result[2] = x[0];
    result[3] = x[1];
    i2a(ts.tm_mon + 1); // Months in 01-12
    result[5] = x[0];
    result[6] = x[1];
    i2a(ts.tm_mday);
    result[8] = x[0];
    result[9] = x[1];
    i2a(ts.tm_hour);
    result[11] = x[0];
    result[12] = x[1];
    i2a(ts.tm_min);
    result[14] = x[0];
    result[15] = x[1];
    i2a(ts.tm_sec);
    result[17] = x[0];
    result[18] = x[1];
return result;
}


time_t time_now(void) {
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0) {
		return -1;
	}
//                THROW(AssertException, "%s", getLastErrorText());
	return t.tv_sec;
}


long long time_milli(void) {
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0) {
		return -1;
	}
	return (long long)t.tv_sec * 1000  +  (long long)t.tv_usec / 1000;
}

int time_usleep(long u) {
	struct timeval t;
	t.tv_sec = u / 1000000;
	t.tv_usec = (suseconds_t) (u % 1000000);
	select(0, 0, 0, 0, &t);
	return 1;
}


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif
