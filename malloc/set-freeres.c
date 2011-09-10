/* Copyright (C) 1997,1999-2003,2011 Free Software Foundation, Inc.
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

#include <atomic.h>
#include <stdlib.h>
#include <set-hooks.h>
#include <libc-internal.h>

#include "../libio/libioP.h"

DEFINE_HOOK (__libc_subfreeres, (void));

symbol_set_define (__libc_freeres_ptrs);

void __libc_freeres_fn_section
__libc_freeres (void)
{
  /* This function might be called from different places.  So better
     protect for multiple executions since these are fatal.  */
  static long int already_called;

  if (! atomic_compare_and_exchange_bool_acq (&already_called, 1, 0))
    {
      void * const *p;

      _IO_cleanup ();

      RUN_HOOK (__libc_subfreeres, ());

      for (p = symbol_set_first_element (__libc_freeres_ptrs);
	   ! symbol_set_end_p (__libc_freeres_ptrs, p); ++p)
	free (*p);
    }
}
libc_hidden_def (__libc_freeres)
