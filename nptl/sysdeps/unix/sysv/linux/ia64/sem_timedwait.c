/* Copyright (C) 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#include <lowlevellock.h>
#include <internaltypes.h>

#include <shlib-compat.h>


int
sem_timedwait (sem, abstime)
     sem_t *sem;
     const struct timespec *abstime;
{
  int oldval, val;

  val = *(int *) sem;
  do
    {
      while (__builtin_expect (val == 0, 0))
	{
	  /* Check for invalid timeout values.  */
	  if (abstime->tv_nsec >= 1000000000)
	    {
	      __set_errno (EINVAL);
	      return -1;
	    }

	  /* Get the current time.  */
	  struct timeval tv;
	  (void) gettimeofday(&tv, NULL);

	  /* Compute the relative timeout.  */
	  struct timespec rt;
	  rt.tv_sec = abstime->tv_sec - tv.tv_sec;
	  rt.tv_nsec = abstime->tv_nsec - tv.tv_usec * 1000;
	  if (rt.tv_nsec < 0)
	    {
	      rt.tv_nsec += 1000000000;
	      --rt.tv_sec;
	    }
	  /* Already timed out.  */
	  if (rt.tv_sec < 0)
	    {
	      __set_errno (ETIMEDOUT);
	      return -1;
	    }

	  /* Do wait.  */
	  int err = lll_futex_timed_wait ((int *) sem, 0, &rt);

	  /* Returned after timing out?  */
	  if (err == -ETIMEDOUT)
	    {
	      __set_errno (ETIMEDOUT);
	      return -1;
	    }

	  /* Handle EINTR.  */
	  if (err != 0 && err != -EWOULDBLOCK)
	    {
	      __set_errno (-err);
	      return -1;
	    }

	  val = *(int *) sem;
	}
      oldval = val;
    }
  while ((val = lll_compare_and_swap ((int *) sem, oldval, oldval - 1))
	 != oldval);
  return 0;
}
