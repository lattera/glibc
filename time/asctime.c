/* Copyright (C) 1991,1993,1995-1997,2000,2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "../locale/localeinfo.h"
#include <errno.h>
#include <stdio.h>
#include <time.h>

/* This is defined in locale/C-time.c in the GNU libc.  */
extern const struct locale_data _nl_C_LC_TIME attribute_hidden;
#define ab_day_name(DAY) (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (ABDAY_1)+(DAY)].string)
#define ab_month_name(MON) (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (ABMON_1)+(MON)].string)

static const char format[] = "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n";
static char result[	         3+1+ 3+1+20+1+20+1+20+1+20+1+20+1 + 1];

/* Returns a string of the form "Day Mon dd hh:mm:ss yyyy\n"
   which is the representation of TP in that form.  */
char *
asctime (const struct tm *tp)
{
  return __asctime_r (tp, result);
}
libc_hidden_def (asctime)

char *
__asctime_r (const struct tm *tp, char *buf)
{
  if (tp == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  if (sprintf (buf, format,
	       (tp->tm_wday < 0 || tp->tm_wday >= 7 ?
		"???" : ab_day_name (tp->tm_wday)),
	       (tp->tm_mon < 0 || tp->tm_mon >= 12 ?
		"???" : ab_month_name (tp->tm_mon)),
	       tp->tm_mday, tp->tm_hour, tp->tm_min,
	       tp->tm_sec, 1900 + tp->tm_year) < 0)
    return NULL;

  return buf;
}
weak_alias (__asctime_r, asctime_r)
