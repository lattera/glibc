/* Copyright (C) 1991, 1992, 1996 Free Software Foundation, Inc.
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

#include <sys/times.h>
#include <time.h>
#include <unistd.h>

/* Return the time used by the program so far (user time + system time).  */
clock_t
clock (void)
{
  struct tms buf;
  long clk_tck = __sysconf (_SC_CLK_TCK);

  if (__times (&buf) < 0)
    return (clock_t) -1;

  return
    (clk_tck <= CLOCKS_PER_SEC)
    ? ((unsigned long) buf.tms_utime + buf.tms_stime) * (CLOCKS_PER_SEC
							 / clk_tck)
    : ((unsigned long) buf.tms_utime + buf.tms_stime) / (clk_tck
							 / CLOCKS_PER_SEC);
}
