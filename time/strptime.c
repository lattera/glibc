/* Convert a string representation of time to a time value.
   Copyright (C) 1996-2000, 2001, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

/* XXX This version of the implementation is not really complete.
   Some of the fields cannot add information alone.  But if seeing
   some of them in the same format (such as year, week and weekday)
   this is enough information for determining the date.  */

#include <time.h>
#include <locale/localeinfo.h>


char *
strptime (buf, format, tm)
     const char *buf;
     const char *format;
     struct tm *tm;
{
  enum ptime_locale_status decided = not;
  return __strptime_internal (buf, format, tm, &decided, -1,
			      _NL_CURRENT_LOCALE);
}
libc_hidden_def (strptime)
