/* The `struct utmp' type, describing entries in the utmp file.  GNU version.
   Copyright (C) 1993, 1996, 1997 Free Software Foundation, Inc.

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

#ifndef _UTMPBITS_H

#define _UTMPBITS_H	1
#include <features.h>

#include <paths.h>
#include <sys/time.h>
#include <sys/types.h>


#define UT_UNKNOWN	0	/* for ut_type field */

#define RUN_LVL		1
#define BOOT_TIME	2
#define NEW_TIME	3
#define OLD_TIME	4

#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8
#define ACCOUNTING	9

#define UT_LINESIZE	32
#define UT_NAMESIZE	32
#define UT_HOSTSIZE	256

__BEGIN_DECLS

struct lastlog
{
  time_t ll_time;
  char ll_line[UT_LINESIZE];
  char ll_host[UT_HOSTSIZE];
};


/* XXX We are not ready to use this now.  It needs some more research.
   Simly copying the behaviour of other implementations is no big
   help.  */
#if 0
/* Which program created the record.  */
enum utlogin
{
  unknown,
  X,
  local,
  rlogin,
  telnet,
  rsh,
  ftp,
  screen,
  splitvt,
  xterm
  /* More could be added here.  */
};
#endif


struct exit_status
{
  short int e_termination;	/* Process termination status.  */
  short int e_exit;		/* Process exit status.  */
};


struct utmp
{
  short int ut_type;		/* Type of login.  */
  pid_t ut_pid;			/* Pid of login process.  */
  char ut_line[UT_LINESIZE];	/* NUL-terminated devicename of tty.  */
  char ut_id[4];		/* Inittab id. */
  char ut_user[UT_NAMESIZE];	/* Username (not NUL terminated).  */
#define ut_name	ut_user		/* Compatible field name for same.  */
  char ut_host[UT_HOSTSIZE];	/* Hostname for remote login.  */
  struct exit_status ut_exit;	/* The exit status of a process marked
				   as DEAD_PROCESS.  */
  long ut_session;		/* Session ID, used for windowing.  */
  struct timeval ut_tv;		/* Time entry was made.  */
  int32_t ut_addr_v6[4];	/* Internet address of remote host.  */
  char pad[20];			/* Reserved for future use.  */
};

/* Backwards compatibility hacks.  */
#ifndef _NO_UT_TIME
/* We have a problem here: `ut_time' is also used otherwise.  Define
   _NO_UT_TIME if the compiler complains.  */
# define ut_time	ut_tv.tv_sec
#endif
#define ut_xtime	ut_tv.tv_sec
#define ut_addr		ut_addr_v6[0]

/* Tell the user that we have a modern system with UT_HOST, UT_PID,
   UT_TYPE, UT_ID and UT_TV fields.  */
#define _HAVE_UT_TYPE	1
#define _HAVE_UT_PID	1
#define _HAVE_UT_ID	1
#define _HAVE_UT_TV	1
#define _HAVE_UT_HOST	1

__END_DECLS

#endif /* !_UTMP_H_ */
