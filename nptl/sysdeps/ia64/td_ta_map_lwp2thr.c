/* Which thread is running on an LWP?  IA-64 version.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <tls.h>


td_err_e
td_ta_map_lwp2thr (const td_thragent_t *ta, lwpid_t lwpid, td_thrhandle_t *th)
{
  LOG ("td_ta_map_lwp2thr");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  prgregset_t regs;
  if (ps_lgetregs (ta->ph, lwpid, regs) != PS_OK)
    return TD_ERR;

  /* IA-64 thread register is r13.  */
  th->th_unique = regs[13];

  /* Found it.  Now complete the `td_thrhandle_t' object.  */
  th->th_ta_p = (td_thragent_t *) ta;

  return TD_OK;
}
