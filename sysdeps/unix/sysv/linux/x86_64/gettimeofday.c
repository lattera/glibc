/* Copyright (C) 2002, 2003, 2007, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <dl-vdso.h>


#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000ul


#ifdef SHARED
void *gettimeofday_ifunc (void) __asm__ ("__gettimeofday");

void *
gettimeofday_ifunc (void)
{
  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);

  /* If the vDSO is not available we fall back on the old vsyscall.  */
  return (_dl_vdso_vsym ("gettimeofday", &linux26)
	  ?: (void *) VSYSCALL_ADDR_vgettimeofday);
}
__asm (".type __gettimeofday, %gnu_indirect_function");
#else
# include <sys/time.h>

int
__gettimeofday (struct timeval *tv, struct timezone *tz)
{
  return ((int (*) (struct timeval *, struct timezone *)) VSYSCALL_ADDR_vgettimeofday) (tv, tz);
}
#endif

weak_alias (__gettimeofday, gettimeofday)
strong_alias (__gettimeofday, __gettimeofday_internal)
