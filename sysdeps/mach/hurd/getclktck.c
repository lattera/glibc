/* Return run-time value of CLK_TCK for Hurd.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <mach.h>
#include <mach/host_info.h>

#ifndef SYSTEM_CLK_TCK
# define SYSTEM_CLK_TCK 100
#endif

/* Return frequency of times().  */
int
__getclktck ()
{
  struct host_sched_info hsi;
  mach_msg_type_number_t count;
  error_t err;

  count = HOST_SCHED_INFO_COUNT;
  err = __host_info (__mach_task_self (), HOST_SCHED_INFO,
		     (host_info_t) &hsi, &count);
  if (err)
    return SYSTEM_CLK_TCK;

  return hsi.min_quantum;
}

/* Before glibc 2.2, the Hurd actually did this differently, so we
   need to keep a compatibility symbol.  */

#include <shlib-compat.h>

#if SHLIB_COMPAT (libc, GLIBC_2_1_1, GLIBC_2_2)
compat_symbol (libc, __getclktck, __libc_clk_tck, GLIBC_2_1_1);
#endif
