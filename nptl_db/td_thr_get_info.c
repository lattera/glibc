/* Get thread information.
   Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
#include <string.h>
#include "thread_dbP.h"


td_err_e
td_thr_get_info (const td_thrhandle_t *th, td_thrinfo_t *infop)
{
  LOG ("td_thr_get_info");

  /* Get the thread descriptor.  */
  struct pthread pds;
  if (ps_pdread (th->th_ta_p->ph, th->th_unique, &pds,
		 sizeof (struct pthread)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Fill in information.  Clear first to provide reproducable
     results for the fields we do not fill in.  */
  memset (infop, '\0', sizeof (td_thrinfo_t));

  infop->ti_tid = th->th_unique;
  infop->ti_tls = (char *) pds.specific;
  infop->ti_pri = (pds.schedpolicy == SCHED_OTHER
		   ? 0 : pds.schedparam.sched_priority);
  infop->ti_type = TD_THR_USER;

  if ((pds.cancelhandling & EXITING_BIT) == 0)
    /* XXX For now there is no way to get more information.  */
    infop->ti_state = TD_THR_ACTIVE;
  else if ((pds.cancelhandling & TERMINATED_BIT) == 0)
    infop->ti_state = TD_THR_ZOMBIE;
  else
    infop->ti_state = TD_THR_UNKNOWN;

  /* Initialization which are the same in both cases.  */
  infop->ti_lid = pds.tid ?: ps_getpid (th->th_ta_p->ph);
  infop->ti_ta_p = th->th_ta_p;
  infop->ti_startfunc = pds.start_routine;
  memcpy (&infop->ti_events, &pds.eventbuf.eventmask,
	  sizeof (td_thr_events_t));
  infop->ti_traceme = pds.report_events != false;

  return TD_OK;
}
