/* sem_post -- post to a POSIX semaphore.  SPARC version.
   Copyright (C) 2003, 2004, 2007, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>

#include <shlib-compat.h>

int
__new_sem_post (sem_t *sem)
{
  struct sparc_new_sem *isem = (struct sparc_new_sem *) sem;

  atomic_increment (&isem->value);
  atomic_full_barrier ();
  if (isem->nwaiters > 0)
    {
      int err = lll_futex_wake (&isem->value, 1,
				isem->private ^ FUTEX_PRIVATE_FLAG);
      if (__builtin_expect (err, 0) < 0)
	{
	  __set_errno (-err);
	  return -1;
	}
    }
  return 0;
}
versioned_symbol (libpthread, __new_sem_post, sem_post, GLIBC_2_1);


#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)
int
attribute_compat_text_section
__old_sem_post (sem_t *sem)
{
  struct sparc_old_sem *isem = (struct sparc_old_sem *) sem;
  int err;

  atomic_increment (&isem->value);
  err = lll_futex_wake (&isem->value, 1,
			isem->private ^ FUTEX_PRIVATE_FLAG);
  if (__builtin_expect (err, 0) < 0)
    {
      __set_errno (-err);
      return -1;
    }
  return 0;
}
compat_symbol (libpthread, __old_sem_post, sem_post, GLIBC_2_0);
#endif
