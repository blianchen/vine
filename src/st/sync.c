/* 
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Netscape Portable Runtime library.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are 
 * Copyright (C) 1994-2000 Netscape Communications Corporation.  All
 * Rights Reserved.
 * 
 * Contributor(s):  Silicon Graphics, Inc.
 * 
 * Portions created by SGI are Copyright (C) 2000-2001 Silicon
 * Graphics, Inc.  All Rights Reserved.
 * 
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License Version 2 or later (the
 * "GPL"), in which case the provisions of the GPL are applicable 
 * instead of those above.  If you wish to allow use of your 
 * version of this file only under the terms of the GPL and not to
 * allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by
 * the GPL.  If you do not delete the provisions above, a recipient
 * may use your version of this file under either the MPL or the
 * GPL.
 */

/*
 * This file is derived directly from Netscape Communications Corporation,
 * and consists of extensive modifications made during the year(s) 1999-2000.
 */

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "common.h"


extern time_t _st_curr_time;
extern st_utime_t _st_last_tset;
extern int _st_active_count;

static st_utime_t (*_st_utime)(void) = NULL;


/*****************************************
 * Time functions
 */

st_utime_t st_utime(void) {
	if (_st_utime == NULL) {
#ifdef MD_GET_UTIME
		MD_GET_UTIME();
#else
#error Unknown OS
#endif
	}

	return (*_st_utime)();
}


int st_set_utime_function(st_utime_t (*func)(void))
{
  if (_st_active_count) {
    errno = EINVAL;
    return -1;
  }

  _st_utime = func;

  return 0;
}


st_utime_t st_utime_last_clock(void)
{
  return _ST_LAST_CLOCK;
}


int st_timecache_set(int on)
{
  int wason = (_st_curr_time) ? 1 : 0;

  if (on) {
    _st_curr_time = time(NULL);
    _st_last_tset = st_utime();
  } else
    _st_curr_time = 0;

  return wason;
}


time_t st_time(void)
{
  if (_st_curr_time)
    return _st_curr_time;

  return time(NULL);
}


int st_usleep(st_utime_t usecs)
{
  _st_thread_t *me = _ST_CURRENT_THREAD();

  if (me->flags & _ST_FL_INTERRUPT) {
    me->flags &= ~_ST_FL_INTERRUPT;
    errno = EINTR;
    return -1;
  }

  if (usecs != ST_UTIME_NO_TIMEOUT) {
    me->state = _ST_ST_SLEEPING;
    _ST_ADD_SLEEPQ(me, usecs);
  } else
    me->state = _ST_ST_SUSPENDED;

  _ST_SWITCH_CONTEXT(me);

  if (me->flags & _ST_FL_INTERRUPT) {
    me->flags &= ~_ST_FL_INTERRUPT;
    errno = EINTR;
    return -1;
  }

  return 0;
}


int st_sleep(int secs)
{
  return st_usleep((secs >= 0) ? secs * (st_utime_t) 1000000LL :
				 ST_UTIME_NO_TIMEOUT);
}

