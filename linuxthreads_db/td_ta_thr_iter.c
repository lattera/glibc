/* Iterate over a process's threads.
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

#include "thread_dbP.h"


td_err_e
td_ta_thr_iter (const td_thragent_t *ta, td_thr_iter_f *callback,
		void *cbdata_p, td_thr_state_e state, int ti_pri,
		sigset_t *ti_sigmask_p, unsigned int ti_user_flags)
{
  struct pthread_handle_struct *handles = ta->handles;
  int pthread_threads_max = ta->pthread_threads_max;
  int cnt;

  LOG (__FUNCTION__);

  /* Now get all descriptors, one after the other.  */
  for (cnt = 0; cnt < pthread_threads_max; ++cnt, ++handles)
    {
      struct pthread_handle_struct phc;

      if (cnt == 1)
	/* Skip the manager thread.  */
	continue;

      if (ps_pdread (ta->ph, handles, &phc,
		     sizeof (struct pthread_handle_struct)) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

      if (phc.h_descr != NULL)
	{
	  struct _pthread_descr_struct pds;
	  td_thrhandle_t th;

	  if (ps_pdread (ta->ph, phc.h_descr, &pds,
			 sizeof (struct _pthread_descr_struct)) != PS_OK)
	    return TD_ERR;	/* XXX Other error value?  */

	  /* Now test whether this thread matches the specified
	     conditions.  */

	  /* Only if the priority level is as high or higher.  */
	  if (pds.p_priority < ti_pri)
	    continue;

	  /* Yep, it matches.  Call the callback function.  */
	  th.th_ta_p = (td_thragent_t *) ta;
	  th.th_unique = phc.h_descr;
	  if (callback (&th, cbdata_p) != 0)
	    return TD_DBERR;
	}
    }

  return TD_OK;
}
