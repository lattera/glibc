/* Copyright (C) 1999 Free Software Foundation, Inc.
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

#include <string.h>
#include <utmp.h>
#include <utmpx.h>

/* Copy the information in UTMP to UTMPX. */
void
getutmpx (const struct utmp *utmp, struct utmpx *utmpx)
{
  memset (utmpx, 0, sizeof (struct utmpx));

#if _HAVE_UT_TYPE - 0
  utmpx->ut_type = utmp->ut_type;
#endif
#if _HAVE_UT_PID - 0
  utmpx->ut_pid = utmp->ut_pid;
#endif
  memcpy (utmpx->ut_line, utmp->ut_line, sizeof (utmp->ut_line));
  memcpy (utmpx->ut_user, utmp->ut_user, sizeof (utmp->ut_user));
#if _HAVE_UT_ID - 0
  memcpy (utmpx->ut_id, utmp->ut_id, sizeof (utmp->ut_id));
#endif
#if _HAVE_UT_HOST - 0
  memcpy (utmpx->ut_host, utmp->ut_host, sizeof (utmp->ut_host));
#endif
#if _HAVE_UT_TV - 0
  utmpx->ut_tv = utmp->ut_tv;
#else
  utmpx->ut_time = utmp->ut_time;
#endif
}
