/* Attach to target process.
   Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

#include <stddef.h>
#include <stdlib.h>
#include <gnu/lib-names.h>

#include "thread_dbP.h"


td_err_e
td_ta_new (struct ps_prochandle *ps, td_thragent_t **ta)
{
  psaddr_t addr;

  LOG (__FUNCTION__);

  /* See whether the library contains the necessary symbols.  */
  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO, "__pthread_threads_debug",
		         &addr) != PS_OK)
    return TD_LIBTHREAD;

  /* Fill in the appropriate information.  */
  *ta = (td_thragent_t *) malloc (sizeof (td_thragent_t));
  if (*ta == NULL)
    return TD_MALLOC;

  /* Store the proc handle which we will pass to the callback functions
     back into the debugger.  */
  (*ta)->ph = ps;

  /* See whether the library contains the necessary symbols.  */
  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO, "__pthread_handles",
		         &addr) != PS_OK)
    {
    free_return:
      free (*ta);
      return TD_ERR;
    }

  (*ta)->handles = (struct pthread_handle_struct *) addr;


  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO, "pthread_keys",
		         &addr) != PS_OK)
    goto free_return;

  /* Cast to the right type.  */
  (*ta)->keys = (struct pthread_key_struct *) addr;

  /* Find out about the maximum number of threads.  Old implementations
     don't provide this information.  In this case we assume that the
     debug  library is compiled with the same values.  */
  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO,
			 "__linuxthreads_pthread_threads_max", &addr) != PS_OK)
    (*ta)->pthread_threads_max = PTHREAD_THREADS_MAX;
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->pthread_threads_max, sizeof (int))
	  != PS_OK)
	goto free_return;
    }

  /* Similar for the maximum number of thread local data keys.  */
  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO,
			 "__linuxthreads_pthread_keys_max", &addr) != PS_OK)
    (*ta)->pthread_keys_max = PTHREAD_KEYS_MAX;
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->pthread_keys_max, sizeof (int))
	  != PS_OK)
	goto free_return;
    }

  /* And for the size of the second level arrays for the keys.  */
  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO,
			 "__linuxthreads_pthread_sizeof_descr", &addr)
      != PS_OK)
    (*ta)->sizeof_descr = sizeof (struct _pthread_descr_struct);
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->sizeof_descr, sizeof (int)) != PS_OK)
	goto free_return;
    }

  /* Similar for the maximum number of thread local data keys.  */
  if (ps_pglobal_lookup (ps, LIBPTHREAD_SO,
			 "__linuxthreads_pthread_keys_max", &addr) != PS_OK)
    (*ta)->pthread_keys_max = PTHREAD_KEYS_MAX;
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->pthread_keys_max, sizeof (int))
	  != PS_OK)
	goto free_return;
    }

  return TD_OK;
}
