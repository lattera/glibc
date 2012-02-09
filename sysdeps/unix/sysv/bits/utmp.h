/* The `struct utmp' type, describing entries in the utmp file.  System V.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _UTMP_H
# error "Never include <bits/utmp.h> directly; use <utmp.h> instead."
#endif


#include <time.h>

#define _PATH_UTMP      "/var/adm/utmp"
#define _PATH_WTMP      "/var/adm/wtmp"
#define _PATH_LASTLOG   "/var/adm/lastlog"


struct utmp
  {
#define	ut_name	ut_user
    char ut_user[8];
    char ut_id[4];
    char ut_line[12];
    short int ut_pid;
    short int ut_type;
    struct exit_status
      {
	short int e_termination;
	short int e_exit;
      } ut_exit;
    __time_t ut_time;
  };


/* Tell the user that we have a modern system with UT_HOST, UT_TYPE, UT_ID
   and UT_TV fields.  */
#define _HAVE_UT_TYPE	1
#define _HAVE_UT_ID	1
#define _HAVE_UT_TV	1
#define _HAVE_UT_HOST	1
