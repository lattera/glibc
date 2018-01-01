/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <shlib-compat.h>
#include "compat-timer.h"


#define timer_getoverrun_alias __timer_getoverrun_new
#include <sysdeps/unix/sysv/linux/timer_getoverr.c>

#undef timer_getoverrun
versioned_symbol (librt, __timer_getoverrun_new, timer_getoverrun,
		  GLIBC_2_3_3);


#if SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
int
__timer_getoverrun_old (int timerid)
{
  return __timer_getoverrun_new (__compat_timer_list[timerid]);
}
compat_symbol (librt, __timer_getoverrun_old, timer_getoverrun, GLIBC_2_2);
#endif
