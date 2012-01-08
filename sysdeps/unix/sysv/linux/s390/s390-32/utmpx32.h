/* The `struct utmp' type, describing entries in the utmp file.  GNU version.
   Copyright (C) 1993, 1996, 1997, 1998, 1999, 2002, 2008, 2010, 2012
   Free Software Foundation, Inc.
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

#ifndef _UTMPX32_H
#define _UTMPX32_H 1

#include <paths.h>
#include <sys/time.h>
#include <sys/types.h>
#include <bits/wordsize.h>
#include <utmpx.h>

/* The structure describing an entry in the user accounting database.  */
struct utmpx32
{
  short int ut_type;		/* Type of login.  */
  __pid_t ut_pid;		/* Process ID of login process.  */
  char ut_line[__UT_LINESIZE];	/* Devicename.  */
  char ut_id[4];		/* Inittab ID. */
  char ut_user[__UT_NAMESIZE];	/* Username.  */
  char ut_host[__UT_HOSTSIZE];	/* Hostname for remote login.  */
  struct __exit_status ut_exit;	/* Exit status of a process marked
				   as DEAD_PROCESS.  */
  __int64_t ut_session;		/* Session ID, used for windowing.  */
  struct
  {
    __int64_t tv_sec;		/* Seconds.  */
    __int64_t tv_usec;		/* Microseconds.  */
  } ut_tv;			/* Time entry was made.  */

  __int32_t ut_addr_v6[4];	/* Internet address of remote host.  */
  char __unused[20];		/* Reserved for future use.  */
};

/* The internal interface needed by the compat wrapper functions.  */
extern struct utmpx *__getutxent (void);
extern struct utmpx *__getutxid (const struct utmpx *__id);
extern struct utmpx *__getutxline (const struct utmpx *__line);
extern struct utmpx *__pututxline (const struct utmpx *__utmpx);
extern void __updwtmpx (const char *__wtmpx_file, const struct utmpx *__utmpx);
extern void __getutmp (const struct utmpx *__utmpx, struct utmp *__utmp);
extern void __getutmpx (const struct utmp *__utmp, struct utmpx *__utmpx);

#endif /* utmpx32.h */
