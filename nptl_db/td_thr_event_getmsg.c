/* Retrieve event.
   Copyright (C) 1999, 2001, 2002, 2003 Free Software Foundation, Inc.
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
td_thr_event_getmsg (const td_thrhandle_t *th, td_event_msg_t *msg)
{
  td_eventbuf_t event;

  LOG ("td_thr_event_getmsg");

  /* Read the event structure from the target.  */
  if (ps_pdread (th->th_ta_p->ph,
		 ((char *) th->th_unique
		  + offsetof (struct pthread, eventbuf)),
		 &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Check whether an event occurred.  */
  if (event.eventnum == TD_EVENT_NONE)
    /* Nothing.  */
    return TD_NOMSG;

  /* Fill the user's data structure.  */
  msg->event = event.eventnum;
  msg->th_p = th;
  msg->msg.data = (uintptr_t) event.eventdata;

  /* And clear the event message in the target.  */
  memset (&event, '\0', sizeof (td_eventbuf_t));
  if (ps_pdwrite (th->th_ta_p->ph,
		  ((char *) th->th_unique
		   + offsetof (struct pthread, eventbuf)),
		  &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Get the pointer to the thread descriptor with the last event.
     If it doesn't match TH, then walk down the list until we find it.
     We must splice it out of the list so that there is no dangling
     pointer to it later when it dies.  */
  psaddr_t thp, prevp = th->th_ta_p->pthread_last_event;
  if (ps_pdread (th->th_ta_p->ph,
		 prevp, &thp, sizeof (struct pthread *)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  psaddr_t next;
  while (thp != 0)
    {
      if (ps_pdread (th->th_ta_p->ph,
		     (char *) thp + offsetof (struct pthread, nextevent),
		     &next, sizeof (struct pthread *)) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

      if (next == thp)
	return TD_DBERR;

      if (thp == th->th_unique)
	{
	  /* PREVP points at this thread, splice it out.  */
	  if (prevp == (char *) next + offsetof (struct pthread, nextevent))
	    return TD_DBERR;

	  if (ps_pdwrite (th->th_ta_p->ph, prevp, &next, sizeof next) != PS_OK)
	    return TD_ERR;	/* XXX Other error value?  */

	  /* Now clear this thread's own next pointer so it's not dangling
	     when the thread resumes and then chains on for its next event.  */
	  next = NULL;
	  if (ps_pdwrite (th->th_ta_p->ph,
			  (char *) thp + offsetof (struct pthread, nextevent),
			  &next, sizeof next) != PS_OK)
	    return TD_ERR;	/* XXX Other error value?  */

	  return TD_OK;
	}

      prevp = (char *) thp + offsetof (struct pthread, nextevent);
      thp = next;
    }

  /* Ack!  This should not happen.  */
  return TD_DBERR;
}
