/* Validate a thread handle.
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

#include "thread_dbP.h"


static td_err_e
check_thread_list (const td_thrhandle_t *th, psaddr_t head)
{
  list_t list;
  td_err_e result = TD_NOTHR;

  if (ps_pdread (th->th_ta_p->ph, head, &list.next, sizeof (list.next))
      != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  while (list.next != head)
    if ((psaddr_t) list.next - offsetof (struct pthread, header.data.list)
	== th->th_unique)
      {
	result = TD_OK;
	break;
      }
    else if (ps_pdread (th->th_ta_p->ph, list.next, &list.next,
			sizeof (list.next)) != PS_OK)
      {
	result = TD_ERR;	/* XXX Other error value?  */
	break;
      }

  return result;
}


td_err_e
td_thr_validate (const td_thrhandle_t *th)
{
  LOG ("td_thr_validate");

  /* First check the list with threads using user allocated stacks.  */
  td_err_e result = check_thread_list (th, th->th_ta_p->stack_user);

  /* If our thread is not on this list search the list with stack
     using implementation allocated stacks.  */
  if (result == TD_NOTHR)
    result = check_thread_list (th, th->th_ta_p->stack_used);

  return result;
}
