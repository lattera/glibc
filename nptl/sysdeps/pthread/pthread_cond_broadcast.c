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

#include <endian.h>
#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <pthread.h>
#include <pthreadP.h>

#include <shlib-compat.h>


int
__pthread_cond_broadcast (cond)
     pthread_cond_t *cond;
{
  /* Make sure we are alone.  */
  lll_mutex_lock (cond->__data.__lock);

  /* Are there any waiters to be woken?  */
  if (cond->__data.__total_seq > cond->__data.__wakeup_seq)
    {
      /* Yes.  Mark them all as woken.  */
      cond->__data.__wakeup_seq = cond->__data.__total_seq;

      /* The futex syscall operates on a 32-bit word.  That is fine,
	 we just use the low 32 bits of the sequence counter.  */
#if BYTE_ORDER == LITTLE_ENDIAN
      int *futex = ((int *) (&cond->__data.__wakeup_seq));
#elif BYTE_ORDER == BIG_ENDIAN
      int *futex = ((int *) (&cond->__data.__wakeup_seq)) + 1;
#else
# error "No valid byte order"
#endif

      /* Wake everybody.  */
      lll_futex_wake (futex, INT_MAX);
    }

  /* We are done.  */
  lll_mutex_unlock (cond->__data.__lock);

  return 0;
}

versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
		  GLIBC_2_3_2);
