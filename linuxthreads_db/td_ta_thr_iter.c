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
  int pthread_threads_max;
  size_t sizeof_descr;
  struct pthread_handle_struct *phc;
  int cnt;
#ifdef ALL_THREADS_STOPPED
  int num;
#else
# define num 1
#endif

  LOG (__FUNCTION__);

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  pthread_threads_max = ta->pthread_threads_max;
  sizeof_descr = ta->sizeof_descr;
  phc = (struct pthread_handle_struct *) alloca (sizeof (phc[0])
						 * pthread_threads_max);

  /* Read all the descriptors.  */
  if (ps_pdread (ta->ph, ta->handles, phc,
		 sizeof (struct pthread_handle_struct) * pthread_threads_max)
      != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

#ifdef ALL_THREADS_STOPPED
  /* Read the number of currently active threads.  */
  if (ps_pdread (ta->ph, ta->pthread_handles_num, &num, sizeof (int)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */
#endif

  /* Now get all descriptors, one after the other.  */
  for (cnt = 0; cnt < pthread_threads_max && num > 0; ++cnt)
    if (phc[cnt].h_descr != NULL)
      {
	struct _pthread_descr_struct pds;
	td_thrhandle_t th;

#ifdef ALL_THREADS_STOPPED
	/* First count this active thread.  */
	--num;
#endif

	if (ps_pdread (ta->ph, phc[cnt].h_descr, &pds, sizeof_descr)
	    != PS_OK)
	  return TD_ERR;	/* XXX Other error value?  */

	/* The manager thread must be handled special.  The descriptor
	   exists but the thread only gets created when the first
	   `pthread_create' call is issued.  A clear indication that
	   this happened is when the p_pid field is non-zero.  */
	if (cnt == 1 && pds.p_pid == 0)
	  continue;

	/* Now test whether this thread matches the specified
	   conditions.  */

	/* Only if the priority level is as high or higher.  */
	if (pds.p_priority < ti_pri)
	  continue;

	/* Test the state.
	   XXX This is incomplete.  */
	if (state != TD_THR_ANY_STATE)
	  continue;

	/* XXX For now we ignore threads which are not running anymore.
	   The reason is that gdb tries to get the registers and fails.
	   In future we should have a special mode of the thread library
	   in which we keep the process around until the actual join
	   operation happened.  */
	if (pds.p_exited != 0)
	  continue;

	/* Yep, it matches.  Call the callback function.  */
	th.th_ta_p = (td_thragent_t *) ta;
	th.th_unique = phc[cnt].h_descr;
	if (callback (&th, cbdata_p) != 0)
	  return TD_DBERR;
      }

  return TD_OK;
}
