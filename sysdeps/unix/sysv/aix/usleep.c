/* Copyright (C) 1991, 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <time.h>
#include <unistd.h>
#include <errno.h>

extern int __libc_nanosleep (const struct timespec *requested_time,
			     struct timespec *remaining);

/* Sleep USECONDS microseconds, or until a previously set timer goes off.  */
int
usleep (useconds)
     useconds_t useconds;
{
  struct timespec ts ={tv_sec:0,tv_nsec:(long int)useconds * 1000};
  __libc_nanosleep(&ts,&ts);
  return 0;
}
