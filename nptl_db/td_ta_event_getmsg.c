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

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "thread_dbP.h"


td_err_e
td_ta_event_getmsg (const td_thragent_t *ta, td_event_msg_t *msg)
{
  /* XXX I cannot think of another way but using a static variable.  */
  /* XXX Use at least __thread once it is possible.  */
  static td_thrhandle_t th;

  LOG ("td_ta_event_getmsg");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  /* Get the pointer to the thread descriptor with the last event.  */
  psaddr_t addr;
  if (ps_pdread (ta->ph, ta->pthread_last_event,
		 &addr, sizeof (struct pthread *)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  if (addr == 0)
    /* Nothing waiting.  */
    return TD_NOMSG;

  /* Read the event structure from the target.  */
  td_eventbuf_t event;
  if (ps_pdread (ta->ph, (char *) addr + offsetof (struct pthread, eventbuf),
		 &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* If the structure is on the list there better be an event recorded.  */
  assert (event.eventnum != TD_EVENT_NONE);

  /* Generate the thread descriptor.  */
  th.th_ta_p = (td_thragent_t *) ta;
  th.th_unique = addr;

  /* Fill the user's data structure.  */
  msg->event = event.eventnum;
  msg->th_p = &th;
  msg->msg.data = (uintptr_t) event.eventdata;

  /* Get the pointer to the next descriptor with an event.  */
  psaddr_t next;
  if (ps_pdread (ta->ph, (char *) addr + offsetof (struct pthread, nextevent),
		 &next, sizeof (struct pthread *)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Store the pointer in the list head variable.  */
  if (ps_pdwrite (ta->ph, ta->pthread_last_event,
		  &next, sizeof (struct pthread *)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Clear the next pointer in the current descriptor.  */
  next = NULL;
  if (ps_pdwrite (ta->ph, (char *) addr + offsetof (struct pthread, nextevent),
		  &next, sizeof (struct pthread *)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  return TD_OK;
}
