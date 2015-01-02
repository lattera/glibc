/* Copyright (C) 1999-2015 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdlib.h>
#include <atomic.h>
#include "exit.h"
#include <fork.h>
#include <sysdep.h>
#include <stdint.h>

/* If D is non-NULL, call all functions registered with `__cxa_atexit'
   with the same dso handle.  Otherwise, if D is NULL, call all of the
   registered handlers.  */
void
__cxa_finalize (void *d)
{
  struct exit_function_list *funcs;

 restart:
  for (funcs = __exit_funcs; funcs; funcs = funcs->next)
    {
      struct exit_function *f;

      for (f = &funcs->fns[funcs->idx - 1]; f >= &funcs->fns[0]; --f)
	{
	  void (*cxafn) (void *arg, int status);
	  void *cxaarg;

	  if ((d == NULL || d == f->func.cxa.dso_handle)
	      /* We don't want to run this cleanup more than once.  */
	      && (cxafn = f->func.cxa.fn,
		  cxaarg = f->func.cxa.arg,
		  ! catomic_compare_and_exchange_bool_acq (&f->flavor, ef_free,
							   ef_cxa)))
	    {
	      uint64_t check = __new_exitfn_called;

#ifdef PTR_DEMANGLE
	      PTR_DEMANGLE (cxafn);
#endif
	      cxafn (cxaarg, 0);

	      /* It is possible that that last exit function registered
		 more exit functions.  Start the loop over.  */
	      if (__glibc_unlikely (check != __new_exitfn_called))
		goto restart;
	    }
	}
    }

  /* Also remove the quick_exit handlers, but do not call them.  */
  for (funcs = __quick_exit_funcs; funcs; funcs = funcs->next)
    {
      struct exit_function *f;

      for (f = &funcs->fns[funcs->idx - 1]; f >= &funcs->fns[0]; --f)
	if (d == NULL || d == f->func.cxa.dso_handle)
	  f->flavor = ef_free;
    }

  /* Remove the registered fork handlers.  We do not have to
     unregister anything if the program is going to terminate anyway.  */
#ifdef UNREGISTER_ATFORK
  if (d != NULL)
    UNREGISTER_ATFORK (d);
#endif
}
