/* Get a thread-specific data pointer for a thread.
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


td_err_e
td_thr_tsd (const td_thrhandle_t *th, const thread_key_t tk, void **data)
{
  LOG ("td_thr_tsd");

  /* Check correct value of key.  */
  if (tk >= th->th_ta_p->pthread_keys_max)
    return TD_BADKEY;

  /* Get the key entry.  */
  uintptr_t seq;
  if (ps_pdread (th->th_ta_p->ph, &th->th_ta_p->keys[tk].seq, &seq,
		 sizeof (uintptr_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Fail if this key is not at all used.  */
  if (KEY_UNUSED (seq))
    return TD_BADKEY;

  /* Compute the indeces.  */
  int pthread_key_2ndlevel_size = th->th_ta_p->pthread_key_2ndlevel_size;
  unsigned int idx1st = tk / pthread_key_2ndlevel_size;
  unsigned int idx2nd = tk % pthread_key_2ndlevel_size;

  struct pthread_key_data *level1;
  if (ps_pdread (th->th_ta_p->ph,
		 &((struct pthread *) th->th_unique)->specific[idx1st],
		 &level1, sizeof (level1)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Check the pointer to the second level array.  */
  if (level1 == NULL)
    return TD_NOTSD;

  struct pthread_key_data level2;
  if (ps_pdread (th->th_ta_p->ph, &level1[idx2nd], &level2,
		 sizeof (level2)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Check whether the data is valid.  */
  if (level2.seq != seq)
    return TD_NOTSD;

  if (level2.data != NULL)
    *data = level2.data;

  return TD_OK;
}
