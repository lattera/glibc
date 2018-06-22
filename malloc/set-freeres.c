/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

#include <atomic.h>
#include <stdlib.h>
#include <set-hooks.h>
#include <libc-internal.h>

#include "../libio/libioP.h"

DEFINE_HOOK (__libc_subfreeres, (void));

symbol_set_define (__libc_freeres_ptrs);

extern __attribute__ ((weak)) void __libdl_freeres (void);

extern __attribute__ ((weak)) void __libpthread_freeres (void);

void __libc_freeres_fn_section
__libc_freeres (void)
{
  /* This function might be called from different places.  So better
     protect for multiple executions since these are fatal.  */
  static long int already_called;

  if (!atomic_compare_and_exchange_bool_acq (&already_called, 1, 0))
    {
      void *const *p;

      _IO_cleanup ();

      /* We run the resource freeing after IO cleanup.  */
      RUN_HOOK (__libc_subfreeres, ());

      /* Call the libdl list of cleanup functions
	 (weak-ref-and-check).  */
      if (&__libdl_freeres != NULL)
	__libdl_freeres ();

      /* Call the libpthread list of cleanup functions
	 (weak-ref-and-check).  */
      if (&__libpthread_freeres != NULL)
	__libpthread_freeres ();

      for (p = symbol_set_first_element (__libc_freeres_ptrs);
           !symbol_set_end_p (__libc_freeres_ptrs, p); ++p)
        free (*p);
    }
}
libc_hidden_def (__libc_freeres)
