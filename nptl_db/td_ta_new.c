/* Attach to target process.
   Copyright (C) 1999, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 1999.

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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <version.h>

#include "thread_dbP.h"


/* Datatype for the list of known thread agents.  Normally there will
   be exactly one so we don't spend much though on making it fast.  */
LIST_HEAD (__td_agent_list);


td_err_e
td_ta_new (struct ps_prochandle *ps, td_thragent_t **ta)
{
  psaddr_t addr;
  psaddr_t versaddr;
  char versbuf[sizeof (VERSION)];

  LOG ("td_ta_new");

  /* Get the global event mask.  This is one of the variables which
     are new in the thread library to enable debugging.  If it is
     not available we cannot debug.  */
  if (td_lookup (ps, SYM_PTHREAD_THREADS_EVENTS, &addr) != PS_OK)
    return TD_NOLIBTHREAD;

  /* Check whether the versions match.  */
  if (td_lookup (ps, SYM_PTHREAD_VERSION, &versaddr) != PS_OK)
    return TD_VERSION;
  if (ps_pdread (ps, versaddr, versbuf, sizeof (versbuf)) != PS_OK)
    return TD_ERR;

  if (versbuf[sizeof (versbuf) - 1] != '\0' || strcmp (versbuf, VERSION) != 0)
    /* Not the right version.  */
    return TD_VERSION;

  /* Fill in the appropriate information.  */
  *ta = (td_thragent_t *) malloc (sizeof (td_thragent_t));
  if (*ta == NULL)
    return TD_MALLOC;

  /* Store the proc handle which we will pass to the callback functions
     back into the debugger.  */
  (*ta)->ph = ps;

  /* Remember the address.  */
  (*ta)->pthread_threads_eventsp = (td_thr_events_t *) addr;

  /* Get the pointer to the variable pointing to the thread descriptor
     with the last event.  */
  if (td_lookup (ps, SYM_PTHREAD_LAST_EVENT, &(*ta)->pthread_last_event)
      != PS_OK)
    {
    free_return:
      free (*ta);
      return TD_ERR;
    }


  if (td_lookup (ps, SYM_PTHREAD_STACK_USER, &addr) != PS_OK)
    goto free_return;
  /* Cast to the right type.  */
  (*ta)->stack_user = (list_t *) addr;

  if (td_lookup (ps, SYM_PTHREAD_STACK_USED, &addr) != PS_OK)
    goto free_return;
  /* Cast to the right type.  */
  (*ta)->stack_used = (list_t *) addr;


  if (td_lookup (ps, SYM_PTHREAD_KEYS, &addr) != PS_OK)
    goto free_return;
  /* Cast to the right type.  */
  (*ta)->keys = (struct pthread_key_struct *) addr;


  /* Similarly for the maximum number of thread local data keys.  */
  if (td_lookup (ps, SYM_PTHREAD_KEYS_MAX, &addr) != PS_OK
      || ps_pdread (ps, addr, &(*ta)->pthread_keys_max, sizeof (int)) != PS_OK)
    goto free_return;


  /* And for the size of the second level arrays for the keys.  */
  if (td_lookup (ps, SYM_PTHREAD_SIZEOF_DESCR, &addr) != PS_OK
      || ps_pdread (ps, addr, &(*ta)->sizeof_descr, sizeof (int)) != PS_OK)
    goto free_return;


  /* Now add the new agent descriptor to the list.  */
  list_add (&(*ta)->list, &__td_agent_list);

  return TD_OK;
}
