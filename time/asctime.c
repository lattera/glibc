/* Copyright (C) 1991, 1993 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <localeinfo.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>


/* Returns a string of the form "Day Mon dd hh:mm:ss yyyy\n"
   which is the representation of TP in that form.  */
char *
DEFUN(asctime, (tp), CONST struct tm *tp)
{
  static const char format[] = "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n";
  static char result[	         3+1+ 3+1+20+1+20+1+20+1+20+1+20+1 + 1];

  if (tp == NULL)
    {
      errno = EINVAL;
      return NULL;
    }
  
  if (sprintf (result, format,
	       (tp->tm_wday < 0 || tp->tm_wday >= 7 ?
		"???" : _time_info->abbrev_wkday[tp->tm_wday]),
	       (tp->tm_mon < 0 || tp->tm_mon >= 12 ?
		"???" : _time_info->abbrev_month[tp->tm_mon]),
	       tp->tm_mday, tp->tm_hour, tp->tm_min,
	       tp->tm_sec, 1900 + tp->tm_year) < 0)
    return NULL;

  return result;
}
