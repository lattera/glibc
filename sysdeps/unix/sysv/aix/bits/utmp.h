/* The `struct utmp' type, describing entries in the utmp file.  AIX.
   Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
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

#ifndef _UTMP_H
# error "Never include <bits/utmp.h> directly; use <utmp.h> instead."
#endif


#include <time.h>

#define _PATH_UTMP      "/etc/utmp"
#define _PATH_WTMP      "/var/adm/wtmp"
#define _PATH_LASTLOG   "/var/adm/lastlog"


#define UT_LINESIZE	12
#define UT_NAMESIZE	8
#define UT_HOSTSIZE	16


struct utmp
  {
#define	ut_name	ut_user
    char ut_user[UT_NAMESIZE];
    char ut_id[14];
    char ut_line[UT_LINESIZE];
    short int ut_type;
    short int ut_pid;
    struct exit_status
      {
	short int e_termination;
	short int e_exit;
      } ut_exit;
    __time_t ut_time;
    char ut_host[UT_HOSTSIZE];
  };


/* Tell the user that we have a modern system with UT_HOST, UT_TYPE, and
   UT_ID fields.  */
#define _HAVE_UT_TYPE	1
#define _HAVE_UT_PID	1
#define _HAVE_UT_ID	1
#define _HAVE_UT_HOST	1


/* Values for the `ut_type' field of a `struct utmp'.  */
#define EMPTY		0
#define RUN_LVL		1
#define BOOT_TIME	2
#define OLD_TIME	3
#define NEW_TIME	4
#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8
#define ACCOUNTING	9
