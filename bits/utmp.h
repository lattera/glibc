/* The `struct utmp' type, describing entries in the utmp file.  Generic/BSDish
   Copyright (C) 1993-2016 Free Software Foundation, Inc.
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

#include <paths.h>
#include <time.h>


#define	UT_NAMESIZE	8
#define	UT_LINESIZE	8
#define	UT_HOSTSIZE	16


struct lastlog
  {
    time_t ll_time;
    char ll_line[UT_LINESIZE];
    char ll_host[UT_HOSTSIZE];
  };

struct utmp
  {
    char ut_line[UT_LINESIZE];
    char ut_user[UT_NAMESIZE];
#define ut_name ut_user
    char ut_host[UT_HOSTSIZE];
    long int ut_time;
  };


#define _HAVE_UT_HOST 1		/* We have the ut_host field.  */
