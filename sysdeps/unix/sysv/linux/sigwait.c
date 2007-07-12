/* Copyright (C) 1997,1998,2000,2002-2004,2005 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#define __need_NULL
#include <stddef.h>
#include <string.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#ifdef __NR_rt_sigtimedwait

/* Return any pending signal or wait for one for the given time.  */
static int
do_sigwait (const sigset_t *set, int *sig)
{
  int ret;

#ifdef SIGCANCEL
  sigset_t tmpset;
  if (set != NULL
      && (__builtin_expect (__sigismember (set, SIGCANCEL), 0)
# ifdef SIGSETXID
	  || __builtin_expect (__sigismember (set, SIGSETXID), 0)
# endif
	  ))
    {
      /* Create a temporary mask without the bit for SIGCANCEL set.  */
      // We are not copying more than we have to.
      memcpy (&tmpset, set, _NSIG / 8);
      __sigdelset (&tmpset, SIGCANCEL);
# ifdef SIGSETXID
      __sigdelset (&tmpset, SIGSETXID);
# endif
      set = &tmpset;
    }
#endif

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
#ifdef INTERNAL_SYSCALL
  INTERNAL_SYSCALL_DECL (err);
  do
    ret = INTERNAL_SYSCALL (rt_sigtimedwait, err, 4, CHECK_SIGSET (set),
			    NULL, NULL, _NSIG / 8);
  while (INTERNAL_SYSCALL_ERROR_P (ret, err)
	 && INTERNAL_SYSCALL_ERRNO (ret, err) == EINTR);
  if (! INTERNAL_SYSCALL_ERROR_P (ret, err))
    {
      *sig = ret;
      ret = 0;
    }
  else
    ret = INTERNAL_SYSCALL_ERRNO (ret, err);
#else
  do
    ret = INLINE_SYSCALL (rt_sigtimedwait, 4, CHECK_SIGSET (set),
			  NULL, NULL, _NSIG / 8);
  while (ret == -1 && errno == EINTR);
  if (ret != -1)
    {
      *sig = ret;
      ret = 0;
    }
  else
    ret = errno;
#endif

  return ret;
}

int
__sigwait (set, sig)
     const sigset_t *set;
     int *sig;
{
  if (SINGLE_THREAD_P)
    return do_sigwait (set, sig);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = do_sigwait (set, sig);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
libc_hidden_def (__sigwait)
weak_alias (__sigwait, sigwait)
#else
# include <sysdeps/posix/sigwait.c>
#endif
strong_alias (__sigwait, __libc_sigwait)
