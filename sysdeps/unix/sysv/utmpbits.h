/* The `struct utmp' type, describing entries in the utmp file.  System V.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _UTMPBITS_H

#define _UTMPBITS_H	1

#include <time.h>

#define _PATH_UTMP      "/var/adm/utmp"
#define _PATH_WTMP      "/var/adm/wtmp"
#define _PATH_LASTLOG   "/var/adm/lastlog"

__BEGIN_DECLS

struct utmp
  {
#define	ut_name	ut_user
    char ut_user[8];
    char ut_id[4];
    char ut_line[12];
    short ut_pid;
    short ut_type;
    struct exit_status
      {
	short e_termination;
	short e_exit;
      } ut_exit;
    time_t ut_time;
  };


/* Tell the user that we have a modern system with UT_HOST, UT_TYPE, UT_ID
   and UT_TV fields.  */
#define _HAVE_UT_TYPE	1
#define _HAVE_UT_ID	1
#define _HAVE_UT_TV	1
#define _HAVE_UT_HOST	1

__END_DECLS

#endif /* utmpbits.h  */
