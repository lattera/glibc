/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

#include "xtmp.h"

/* Convert the entry XT to the new utmp format and store it in UT.
   Fields in UT that were not present in the old utmp format are
   initialized to zero.  */
void
xtmp_to_utmp (const struct xtmp *xtmp, struct utmp *utmp)
{
  memset (utmp, 0, sizeof (struct utmp));
#if _HAVE_XT_TYPE - 0
  utmp->ut_type = xtmp->xt_type;
#endif
#if _HAVE_XT_PID - 0
  utmp->ut_pid = xtmp->xt_pid;
#endif
  strncpy (utmp->ut_line, xtmp->xt_line, XT_LINESIZE);
#if _HAVE_XT_ID - 0
  strncpy (utmp->ut_id, xtmp->xt_id, sizeof xtmp->xt_id);
#endif
  utmp->ut_time = xtmp->xt_time;
  strncpy (utmp->ut_user, xtmp->xt_user, XT_NAMESIZE);
#if _HAVE_XT_HOST - 0
  strncpy (utmp->ut_host, xtmp->xt_host, XT_HOSTSIZE);
#endif
  utmp->ut_addr = xtmp->xt_addr;
}

/* Convert the entry UTMP to the old utmp format and store it in XTMP.  */
void
utmp_to_xtmp (const struct utmp *utmp, struct xtmp *xtmp)
{
  memset (xtmp, 0, sizeof (struct xtmp));
#if _HAVE_XT_TYPE - 0
  xtmp->xt_type = utmp->ut_type;
#endif
#if _HAVE_XT_PID - 0
  xtmp->xt_pid = utmp->ut_pid;
#endif
  strncpy (xtmp->xt_line, utmp->ut_line, XT_LINESIZE);
  xtmp->xt_line[XT_LINESIZE] = '\0';
#if _HAVE_XT_ID - 0
  strncpy (xtmp->xt_id, utmp->ut_id, sizeof xtmp->xt_id);
#endif
  xtmp->xt_time = utmp->ut_time;
  strncpy (xtmp->xt_user, utmp->ut_user, XT_NAMESIZE);
#if _HAVE_XT_HOST - 0
  strncpy (xtmp->xt_host, utmp->ut_host, XT_HOSTSIZE);
  xtmp->xt_host[XT_HOSTSIZE] = '\0';
#endif
  xtmp->xt_addr = utmp->ut_addr;
}

/* Compare an old style entry XTMP with a new style entry UTMP.  The
   function returns 1 if the information that is in both old and new
   style entries is identical.  Otherwise this function returns 0.  */
int
compare_entry (const struct utmp *xtmp, const struct utmp *utmp)
{
  return
    (
#if _HAVE_XT_TYPE - 0
     xtmp->ut_type == utmp->ut_type
#endif
#if _HAVE_XT_PID - 0
     && xtmp->ut_pid == utmp->ut_pid
#endif
     && !strncmp (xtmp->ut_line, utmp->ut_line, XT_LINESIZE - 1)
#if _HAVE_XT_ID - 0
     && !strncmp (xtmp->ut_id, utmp->ut_id, sizeof utmp->ut_id)
#endif
     && xtmp->ut_time == utmp->ut_time
     && !strncmp (xtmp->ut_user, utmp->ut_user, XT_NAMESIZE)
#if _HAVE_XT_HOST - 0
     && !strncmp (xtmp->ut_host, utmp->ut_host, XT_HOSTSIZE - 1)
#endif
     && xtmp->ut_addr == utmp->ut_addr);
}
