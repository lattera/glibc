/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LOWLEVELSEM_H
#define _LOWLEVELSEM_H	1

#include <dl-sysdep.h>
#include <tls.h>
#include <lowlevellock.h>
#include <semaphore.h>


static inline int
__attribute__ ((always_inline))
lll_sem_wait (sem_t *sem)
{
  int oldval;
  int newval;

  while (1)
    {
      /* Atomically decrement semaphore counter if it is > 0.  */
      lll_compare_and_swap ((int *) sem, oldval, newval,
			    "ltr %2,%1; jnp 1f; ahi %2,-1");
      /* oldval != newval if the semaphore count has been decremented.	*/
      if (oldval != newval)
	return 0;
      int err = lll_futex_wait ((int *) sem, 0);
      if (err != 0 && err != -EWOULDBLOCK)
	return -err;
    }
  return 0;
}


#if 0
/* Not defined anywhere.  */
extern int __lll_sem_timedwait (sem_t *sem, const struct timespec *ts)
     attribute_hidden;
#define lll_sem_timedwait(sem, timeout) \
  __lll_sem_timedwait (sem, timeout)
#endif

static inline void
__attribute__ ((always_inline))
lll_sem_post(sem_t *sem)
{
  int oldval;
  int newval;

  lll_compare_and_swap ((int *) sem, oldval, newval, "lr %2,%1; ahi %2,1");
  lll_futex_wake ((int *) sem, newval);
}

#endif	/* lowlevelsem.h */
