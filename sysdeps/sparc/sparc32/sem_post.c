/* sem_post -- post to a POSIX semaphore.  Generic futex-using version.
   Copyright (C) 2003-2015 Free Software Foundation, Inc.
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

#include <atomic.h>
#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>
#include <semaphore.h>

#include <shlib-compat.h>

/* Wrapper for lll_futex_wake, with error checking.
   TODO Remove when cleaning up the futex API throughout glibc.  */
static __always_inline void
futex_wake (unsigned int* futex, int processes_to_wake, int private)
{
  int res = lll_futex_wake (futex, processes_to_wake, private);
  /* No error.  Ignore the number of woken processes.  */
  if (res >= 0)
    return;
  switch (res)
    {
    case -EFAULT: /* Could have happened due to memory reuse.  */
    case -EINVAL: /* Could be either due to incorrect alignment (a bug in
		     glibc or in the application) or due to memory being
		     reused for a PI futex.  We cannot distinguish between the
		     two causes, and one of them is correct use, so we do not
		     act in this case.  */
      return;
    case -ENOSYS: /* Must have been caused by a glibc bug.  */
    /* No other errors are documented at this time.  */
    default:
      abort ();
    }
}


/* See sem_wait for an explanation of the algorithm.  */
int
__new_sem_post (sem_t *sem)
{
  struct new_sem *isem = (struct new_sem *) sem;
  int private = isem->private;
  unsigned int v;

  __sparc32_atomic_do_lock24(&isem->pad);

  v = isem->value;
  if ((v << SEM_VALUE_SHIFT) == SEM_VALUE_MAX)
    {
      __sparc32_atomic_do_unlock24(&isem->pad);

      __set_errno (EOVERFLOW);
      return -1;
    }
  isem->value = v + (1 << SEM_VALUE_SHIFT);

  __sparc32_atomic_do_unlock24(&isem->pad);

  if ((v & SEM_NWAITERS_MASK) != 0)
    futex_wake (&isem->value, 1, private);

  return 0;
}
versioned_symbol (libpthread, __new_sem_post, sem_post, GLIBC_2_1);


#if SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_1)
int
attribute_compat_text_section
__old_sem_post (sem_t *sem)
{
  int *futex = (int *) sem;

  /* We must need to synchronize with consumers of this token, so the atomic
     increment must have release MO semantics.  */
  atomic_write_barrier ();
  (void) atomic_increment_val (futex);
  /* We always have to assume it is a shared semaphore.  */
  int err = lll_futex_wake (futex, 1, LLL_SHARED);
  if (__builtin_expect (err, 0) < 0)
    {
      __set_errno (-err);
      return -1;
    }
  return 0;
}
compat_symbol (libpthread, __old_sem_post, sem_post, GLIBC_2_0);
#endif
