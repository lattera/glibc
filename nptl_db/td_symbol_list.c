/* Return list of symbols the library can request.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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

#include <assert.h>
#include <gnu/lib-names.h>
#include "thread_dbP.h"


static const char *symbol_list_arr[] =
{
  [SYM_PTHREAD_THREADS_EVENTS] = "__nptl_threads_events",
  [SYM_PTHREAD_LAST_EVENT] = "__nptl_last_event",
  [SYM_PTHREAD_NTHREADS] = "nptl_nthreads",
  [SYM_PTHREAD_STACK_USED] = "stack_used",
  [SYM_PTHREAD_STACK_USER] = "__stack_user",
  [SYM_PTHREAD_KEYS] = "__pthread_keys",
  [SYM_PTHREAD_KEYS_MAX] = "__pthread_pthread_keys_max",
  [SYM_PTHREAD_SIZEOF_DESCR] = "__pthread_pthread_sizeof_descr",
  [SYM_PTHREAD_CREATE_EVENT] = "__nptl_create_event",
  [SYM_PTHREAD_DEATH_EVENT] = "__nptl_death_event",
  [SYM_PTHREAD_VERSION] = "nptl_version",
  [SYM_NUM_MESSAGES] = NULL
};


const char **
td_symbol_list (void)
{
  return symbol_list_arr;
}


int
td_lookup (struct ps_prochandle *ps, int idx, psaddr_t *sym_addr)
{
  assert (idx >= 0 && idx < SYM_NUM_MESSAGES);
  return ps_pglobal_lookup (ps, LIBPTHREAD_SO, symbol_list_arr[idx], sym_addr);
}
