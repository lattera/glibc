/* Iterate over a process's threads.
   Copyright (C) 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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

#include "thread_dbP.h"
#include <nptl/descr.h>


static td_err_e
iterate_thread_list (const td_thragent_t *ta, td_thr_iter_f *callback,
		     void *cbdata_p, td_thr_state_e state, int ti_pri,
		     psaddr_t head)
{
  list_t list;
  td_err_e result = TD_OK;

  /* Test the state.
     XXX This is incomplete.  Normally this test should be in the loop.  */
  if (state != TD_THR_ANY_STATE)
    return TD_OK;

  if (ps_pdread (ta->ph, head, &list, sizeof (list_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  if (list.next == 0 && list.prev == 0 && head == ta->stack_user)
    {
      /* __pthread_initialize_minimal has not run.
	 There is just the main thread to return.  */
      td_thrhandle_t th;
      td_err_e err = td_ta_map_lwp2thr (ta, ps_getpid (ta->ph), &th);
      return (err != TD_OK ? err
	      : callback (&th, cbdata_p) != 0 ? TD_DBERR : TD_OK);
    }

  while (list.next != head)
    {
      psaddr_t addr = ((psaddr_t) list.next
		       - offsetof (struct pthread, list));

      int schedpolicy;
      if (ps_pdread (ta->ph, &((struct pthread *) addr)->schedpolicy,
		     &schedpolicy, sizeof (int)) != PS_OK)
	{
	  result = TD_ERR;	/* XXX Other error value?  */
	  break;
	}

      struct sched_param schedparam;
      if (ps_pdread (ta->ph, &((struct pthread *) addr)->schedparam,
		     &schedparam, sizeof (struct sched_param)) != PS_OK)
	{
	  result = TD_ERR;	/* XXX Other error value?  */
	  break;
	}

      /* Now test whether this thread matches the specified
	 conditions.  */

      /* Only if the priority level is as high or higher.  */
      int descr_pri = (schedpolicy == SCHED_OTHER
		       ? 0 : schedparam.sched_priority);
      if (descr_pri >= ti_pri)
	{
	  /* XXX For now we ignore threads which are not running anymore.
	     The reason is that gdb tries to get the registers and fails.
	     In future we should have a special mode of the thread library
	     in which we keep the process around until the actual join
	     operation happened.  */
	  int cancelhandling;
	  if (ps_pdread (ta->ph, &((struct pthread *) addr)->cancelhandling,
			 &cancelhandling, sizeof (int)) != PS_OK)
	    {
	      result = TD_ERR;	/* XXX Other error value?  */
	      break;
	    }

	  if ((cancelhandling & TERMINATED_BITMASK) == 0)
	    {
	      /* Yep, it matches.  Call the callback function.  */
	      td_thrhandle_t th;
	      th.th_ta_p = (td_thragent_t *) ta;
	      th.th_unique = addr;
	      if (callback (&th, cbdata_p) != 0)
		return TD_DBERR;
	    }
	}

      /* Get the pointer to the next element.  */
      if (ps_pdread (ta->ph, &((struct pthread *) addr)->list,
		     &list, sizeof (list_t)) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */
    }

  return result;
}


td_err_e
td_ta_thr_iter (const td_thragent_t *ta, td_thr_iter_f *callback,
		void *cbdata_p, td_thr_state_e state, int ti_pri,
		sigset_t *ti_sigmask_p, unsigned int ti_user_flags)
{
  LOG ("td_ta_thr_iter");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  /* The thread library keeps two lists for the running threads.  One
     list contains the thread which are using user-provided stacks
     (this includes the main thread) and the other includes the
     threads for which the thread library allocated the stacks.  We
     have to iterate over both lists separately.  We start with the
     list of threads with user-defined stacks.  */
  td_err_e result = iterate_thread_list (ta, callback, cbdata_p, state, ti_pri,
					 ta->stack_user);

  /* And the threads with stacks allocated by the implementation.  */
  if (result == TD_OK)
    result = iterate_thread_list (ta, callback, cbdata_p, state, ti_pri,
				  ta->stack_used);

  return result;
}
