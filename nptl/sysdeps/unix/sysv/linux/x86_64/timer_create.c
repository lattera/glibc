/* Copyright (C) 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <shlib-compat.h>
#include "compat-timer.h"
#include <atomic.h>


#define timer_create_alias __timer_create_new
#include "../timer_create.c"

#undef timer_create
versioned_symbol (librt, __timer_create_new, timer_create, GLIBC_2_3_3);


#if SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
timer_t __compat_timer_list[OLD_TIMER_MAX] attribute_hidden;


int
__timer_create_old (clockid_t clock_id, struct sigevent *evp, int *timerid)
{
  timer_t newp;

  int res = __timer_create_new (clock_id, evp, &newp);
  if (res == 0)
    {
      int i;
      for (i = 0; i < OLD_TIMER_MAX; ++i)
	if (__compat_timer_list[i] == NULL
	    && ! atomic_compare_and_exchange_bool_acq (&__compat_timer_list[i],
						       newp, NULL))
	  {
	    *timerid = i;
	    break;
	  }

      if (__builtin_expect (i == OLD_TIMER_MAX, 0))
	{
	  /* No free slot.  */
	  (void) __timer_delete_new (newp);
	  __set_errno (EINVAL);
	  res = -1;
	}
    }

  return res;
}
compat_symbol (librt, __timer_create_old, timer_create, GLIBC_2_2);
#endif
