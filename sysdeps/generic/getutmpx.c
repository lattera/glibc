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
getutmpx (const struct utmp *utmp, struct utmpx *utmpx)
{
  utmpx->ut_type = utmp->ut_type;
  utmpx->ut_pid = utmp->ut_pid;
  memcpy (utmpx->ut_line, utmp->ut_line, sizeof (utmpx->ut_line));
  memcpy (utmpx->ut_id, utmp->ut_id, sizeof (utmpx->ut_id));
  memcpy (utmpx->ut_user, utmp->ut_user, sizeof (utmpx->ut_user));
  memcpy (utmpx->ut_host, utmp->ut_host, sizeof (utmpx->ut_host));
#ifdef _GNU_SOURCE
  utmpx->ut_exit.e_termination = utmp->ut_exit.e_termination;
  utmpx->ut_exit.e_exit = utmp->ut_exit.e_exit;
#else
  utmpx->ut_exit.__e_termination = utmp->ut_exit.e_termination;
  utmpx->ut_exit.__e_exit = utmp->ut_exit.e_exit;
#endif
  utmpx->ut_session = utmp->ut_session;
  utmpx->ut_tv = utmp->ut_tv;
}
