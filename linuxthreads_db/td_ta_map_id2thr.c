/* Map thread ID to thread handle.
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
td_ta_map_id2thr (const td_thragent_t *ta, pthread_t pt, td_thrhandle_t *th)
{
  struct pthread_handle_struct *handles = ta->handles;
  struct pthread_handle_struct phc;
  int pthread_threads_max = ta->pthread_threads_max;

  LOG (__FUNCTION__);

  /* We can compute the entry in the handle array we want.  */
  if (ps_pdread (ta->ph, handles + pt % pthread_threads_max, &phc,
		 sizeof (struct pthread_handle_struct)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Test whether this entry is in use.  */
  if (phc.h_descr == NULL)
    return TD_BADTH;

  /* Create the `td_thrhandle_t' object.  */
  th->th_ta_p = (td_thragent_t *) ta;
  th->th_unique = phc.h_descr;

  return TD_OK;
}
