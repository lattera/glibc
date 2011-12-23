/* Copyright (C) 2006, 2007, 2008, 2011 Free Software Foundation, Inc.
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

/* We define a special synchronization primitive for AIO.  POSIX
   conditional variables would be ideal but the pthread_cond_*wait
   operations do not return on EINTR.  This is a requirement for
   correct aio_suspend and lio_listio implementations.  */

#include <assert.h>
#include <signal.h>
#include <pthreadP.h>
#include <lowlevellock.h>

#define DONT_NEED_GAI_MISC_COND	1

#define GAI_MISC_NOTIFY(waitlist) \
  do {									      \
    if (*waitlist->counterp > 0 && --*waitlist->counterp == 0)		      \
      lll_futex_wake (waitlist->counterp, 1, LLL_PRIVATE);		      \
  } while (0)

#define GAI_MISC_WAIT(result, futex, timeout, cancel) \
  do {									      \
    volatile int *futexaddr = &futex;					      \
    int oldval = futex;							      \
									      \
    if (oldval != 0)							      \
      {									      \
	pthread_mutex_unlock (&__gai_requests_mutex);			      \
									      \
	int oldtype;							      \
	if (cancel)							      \
	  oldtype = LIBC_CANCEL_ASYNC ();				      \
									      \
	int status;							      \
	do								      \
	  {								      \
	    status = lll_futex_timed_wait (futexaddr, oldval, timeout,	      \
					   LLL_PRIVATE);		      \
	    if (status != -EWOULDBLOCK)					      \
	      break;							      \
									      \
	    oldval = *futexaddr;					      \
	  }								      \
	while (oldval != 0);						      \
									      \
	if (cancel)							      \
	  LIBC_CANCEL_RESET (oldtype);					      \
									      \
	if (status == -EINTR)						      \
	  result = EINTR;						      \
	else if (status == -ETIMEDOUT)					      \
	  result = EAGAIN;						      \
	else								      \
	  assert (status == 0 || status == -EWOULDBLOCK);		      \
									      \
	pthread_mutex_lock (&__gai_requests_mutex);			      \
      }									      \
  } while (0)


#define gai_start_notify_thread __gai_start_notify_thread
#define gai_create_helper_thread __gai_create_helper_thread

extern inline void
__gai_start_notify_thread (void)
{
  sigset_t ss;
  sigemptyset (&ss);
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, &ss, NULL, _NSIG / 8);
}

extern inline int
__gai_create_helper_thread (pthread_t *threadp, void *(*tf) (void *),
			    void *arg)
{
  pthread_attr_t attr;

  /* Make sure the thread is created detached.  */
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  /* The helper thread needs only very little resources.  */
  (void) pthread_attr_setstacksize (&attr,
				    __pthread_get_minstack (&attr)
				    + 4 * PTHREAD_STACK_MIN);

  /* Block all signals in the helper thread.  To do this thoroughly we
     temporarily have to block all signals here.  */
  sigset_t ss;
  sigset_t oss;
  sigfillset (&ss);
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, &ss, &oss, _NSIG / 8);

  int ret = pthread_create (threadp, &attr, tf, arg);

  /* Restore the signal mask.  */
  INTERNAL_SYSCALL (rt_sigprocmask, err, 4, SIG_SETMASK, &oss, NULL,
		    _NSIG / 8);

  (void) pthread_attr_destroy (&attr);
  return ret;
}

#include_next <gai_misc.h>
