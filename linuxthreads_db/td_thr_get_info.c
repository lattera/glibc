/* Get thread information.
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

#include "thread_dbP.h"


td_err_e
td_thr_get_info (const td_thrhandle_t *th, td_thrinfo_t *infop)
{
  struct _pthread_descr_struct pds;

  LOG (__FUNCTION__);

  /* Get the thread descriptor.  */
  if (ps_pdread (th->th_ta_p->ph, th->th_unique, &pds,
		 th->th_ta_p->sizeof_descr) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Fill in information.  Clear first to provide reproducable
     results for the fields we do not fill in.  */
  memset (infop, '\0', sizeof (td_thrinfo_t));

  infop->ti_ta_p = th->th_ta_p;
  infop->ti_tid = pds.p_tid;
  infop->ti_tls = (char *) pds.p_specific;
  infop->ti_pri = pds.p_priority;
  /* The first thread (0 being the initial one) is the manager thread
     Mark it appropriately.  */
  infop->ti_type = ((pds.p_tid % th->th_ta_p->pthread_threads_max) == 1
		    ? TD_THR_SYSTEM : TD_THR_USER);

  /* We can get the following information only if the thread descriptor
     in the target processor is large enough, i.e., comes from a recent
     enough library.  */
  if (offsetof (struct _pthread_descr_struct, p_startfct)
      < th->th_ta_p->sizeof_descr)
    infop->ti_startfunc = pds.p_startfct;

  return TD_OK;
}
