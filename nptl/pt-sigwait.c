/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sysdep.h>
#include "pthreadP.h"


int
sigwait (const sigset_t *set, int *sig)
{
  int result;
  int oldtype;

  CANCEL_ASYNC (oldtype);

#ifdef INTERNAL_SYSCALL
  result = INTERNAL_SYSCALL (rt_sigtimedwait, 4, set, NULL, NULL, _NSIG / 8);
  if (! INTERNAL_SYSCALL_ERROR_P (result))
    {
      *sig = result;
      result = 0;
    }
  else
    result = INTERNAL_SYSCALL_ERRNO (result);
#elif defined INLINE_SYSCALL
  result = INLINE_SYSCALL (rt_sigtimedwait, 4, set, NULL, NULL, _NSIG / 8);
  if (result != -1)
    {
      *sig = result;
      result = 0;
    }
  else
    result = errno;
#else
  result = __sigwait (set, sig);
#endif

  CANCEL_RESET (oldtype);

  return result;
}
