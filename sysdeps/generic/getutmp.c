/* Copyright (C) 1999 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <utmp.h>
#include <utmpx.h>

void
getutmp (const struct utmpx *utmpx, struct utmp *utmp)
{
  utmp->ut_type = utmpx->ut_type;
  utmp->ut_pid = utmpx->ut_pid;
  memcpy (utmp->ut_line, utmpx->ut_line, sizeof (utmp->ut_line));
  memcpy (utmp->ut_id, utmpx->ut_id, sizeof (utmp->ut_id));
  memcpy (utmp->ut_user, utmpx->ut_user, sizeof (utmp->ut_user));
  memcpy (utmp->ut_host, utmpx->ut_host, sizeof (utmp->ut_host));
#ifdef _GNU_SOURCE
  utmp->ut_exit.e_termination = utmpx->ut_exit.e_termination;
  utmp->ut_exit.e_exit = utmpx->ut_exit.e_exit;
#else
  utmp->ut_exit.__e_termination = utmpx->ut_exit.e_termination;
  utmp->ut_exit.__e_exit = utmpx->ut_exit.e_exit;
#endif
  utmp->ut_session = utmpx->ut_session;
  utmp->ut_tv = utmpx->ut_tv;
}
