/* Structures and defenitions for the user accounting database.  AIX.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.

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

#ifndef _UTMPX_H
# error "Never include <bits/utmpx.h> directly; use <utmpx.h> instead."
#endif

#include <bits/types.h>
#include <sys/time.h>


#ifdef __USE_GNU
# include <paths.h>
# define _PATH_UTMPX	_PATH_UTMP
# define _PATH_WTMPX	_PATH_WTMP
#endif


#define __UT_LINESIZE	12
#define __UT_NAMESIZE	8
#define __UT_HOSTSIZE	16


/* The structure describing an entry in the user accounting database.  */
struct utmpx
{
  char ut_user[__UT_NAMESIZE];	/* Username.  */
  char ut_id[14];		/* Inittab ID. */
  char ut_line[__UT_LINESIZE];	/* Devicename.  */
  short int ut_type;		/* Type of login.  */
  __pid_t ut_pid;		/* Process ID of login process.  */
  struct timeval ut_tv;		/* Time entry was made.  */
  char ut_host[__UT_HOSTSIZE];	/* Hostname for remote login.  */
};


/* Values for the `ut_type' field of a `struct utmpx'.  */
#define EMPTY		0	/* No valid user accounting information.  */

#define RUN_LVL		1	/* The system's runlevel.  */
#define BOOT_TIME	2	/* Time of system boot.  */
#define NEW_TIME	3	/* Time after system clock changed.  */
#define OLD_TIME	4	/* Time when system clock changed.  */

#define INIT_PROCESS	5	/* Process spawned by the init process.  */
#define LOGIN_PROCESS	6	/* Session leader of a logged in user.  */
#define USER_PROCESS	7	/* Normal process.  */
#define DEAD_PROCESS	8	/* Terminated process.  */

#ifdef __USE_GNU
# define ACCOUNTING	9	/* System accounting.  */
#endif

#define _HAVE_UT_TYPE   1
#define _HAVE_UT_PID    1
#define _HAVE_UT_ID     1
#define _HAVE_UT_TV     1
#define _HAVE_UT_HOST   1

