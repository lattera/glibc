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

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>

#include <shlib-compat.h>


int
__new_sem_wait (sem_t *sem)
{
  while (1)
    {
      int oldval;
      int newval;

      /* Atomically decrement semaphore counter if it is > 0.  */
      lll_compare_and_swap ((int *) sem, oldval, newval,
			    "ltr %2,%1; jnp 1f; ahi %2,-1");

      /* oldval != newval if the semaphore count has been decremented.	*/
      if (oldval != newval)
	return 0;

      /* Do wait.  */
      int err = lll_futex_wait ((int *) sem, 0);

      /* Handle EINTR.  */
      if (err != 0 && err != -EWOULDBLOCK)
	{
	  __set_errno (-err);
	  return -1;
	}
    }
}

versioned_symbol (libpthread, __new_sem_wait, sem_wait, GLIBC_2_1);
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
strong_alias (__new_sem_wait, __old_sem_wait)
compat_symbol (libpthread, __old_sem_wait, sem_wait, GLIBC_2_0);
#endif
