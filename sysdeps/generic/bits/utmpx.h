/* Structures and definitions for the user accounting database.  Generic/BSDish
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

#ifndef _UTMPX_H
#error "Never use <bits/utmpx.h> directly; include <utmpx.h> instead."
#endif


#define	__UT_NAMESIZE	8
#define	__UT_LINESIZE	8
#define	__UT_HOSTSIZE	16

struct utmpx
{
  char ut_line[__UT_LINESIZE];
  char ut_name[__UT_NAMESIZE];
  char ut_host[__UT_HOSTSIZE];
  long ut_time;
};
