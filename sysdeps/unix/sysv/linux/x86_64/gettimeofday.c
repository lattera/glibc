/* Copyright (C) 2002-2015 Free Software Foundation, Inc.
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

#include <sys/time.h>

#ifdef SHARED
/* If the vDSO is not available we fall back on the old vsyscall.  */
# define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000ul
# define GETTIMEOFAY_FALLBACK  (void*)VSYSCALL_ADDR_vgettimeofday
#endif

#include <sysdeps/unix/sysv/linux/x86/gettimeofday.c>
