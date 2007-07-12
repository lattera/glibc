/* Enqueue and list of read or write requests.
   Copyright (C) 1997,1998,1999,2000,2001,2002,2003,2005,2006
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <kaio_misc.h>

#ifndef USE_KAIO
#include <lio_listio.c>
#else

#ifndef lio_listio
#include <aio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define LIO_OPCODE_BASE 0
#endif

#include <shlib-compat.h>


/* We need this special structure to handle asynchronous I/O.  */
struct async_waitlist
  {
    int counter;
    struct sigevent sigev;
    struct waitlist list[0];
  };


/* The code in glibc 2.1 to glibc 2.4 issued only one event when all
   requests submitted with lio_listio finished.  The existing practice
   is to issue events for the individual requests as well.  This is
   what the new code does.  */
#if SHLIB_COMPAT (librt, GLIBC_2_1, GLIBC_2_4)
# define LIO_MODE(mode) ((mode) & 127)
# define NO_INDIVIDUAL_EVENT_P(mode) ((mode) & 128)
#else
# define LIO_MODE(mode) mode
# define NO_INDIVIDUAL_EVENT_P(mode) 0
#endif


static int
lio_listio_internal (int mode, struct aiocb *const list[], int nent,
		     struct sigevent *sig)
{
  struct sigevent defsigev;
  struct requestlist *requests[nent];
  int cnt;
  volatile int total = 0;
  int result = 0, op = 0;
  kctx_t kctx = KCTX_NONE;

  if (sig == NULL)
    {
      defsigev.sigev_notify = SIGEV_NONE;
      sig = &defsigev;
    }

  /* Request the mutex.  */
  pthread_mutex_lock (&__aio_requests_mutex);

  if (LIO_MODE (mode) == LIO_WAIT && ! __have_no_kernel_aio && nent > 0)
    {
      int res;
      INTERNAL_SYSCALL_DECL (err);

      kctx = 0;
      do
	res = INTERNAL_SYSCALL (io_setup, err, 2, nent, &kctx);
      while (INTERNAL_SYSCALL_ERROR_P (res, err)
	     && INTERNAL_SYSCALL_ERRNO (res, err) == EINTR);
      if (INTERNAL_SYSCALL_ERROR_P (res, err))
	{
	  kctx = KCTX_NONE;
	  if (INTERNAL_SYSCALL_ERRNO (res, err) == ENOSYS)
	    __have_no_kernel_aio = 1;
	}
    }
  else if (LIO_MODE (mode) == LIO_NOWAIT)
    {
      op = LIO_KTHREAD;
      if (sig->sigev_notify != SIGEV_NONE)
	op = LIO_KTHREAD | LIO_KTHREAD_REQUIRED;
    }
  op |= LIO_OPCODE_BASE;

  /* Now we can enqueue all requests.  Since we already acquired the
     mutex the enqueue function need not do this.  */
  for (cnt = 0; cnt < nent; ++cnt)
    if (list[cnt] != NULL && list[cnt]->aio_lio_opcode != LIO_NOP)
      {
	if (NO_INDIVIDUAL_EVENT_P (mode))
	  list[cnt]->aio_sigevent.sigev_notify = SIGEV_NONE;
	requests[cnt]
	  = __aio_enqueue_request_ctx ((aiocb_union *) list[cnt],
				       list[cnt]->aio_lio_opcode | op,
				       kctx);

	if (requests[cnt] != NULL)
	  /* Successfully enqueued.  */
	  ++total;
	else
	  /* Signal that we've seen an error.  `errno' and the error code
	     of the aiocb will tell more.  */
	  result = -1;
      }
    else
      requests[cnt] = NULL;

  if (total == 0)
    {
      /* We don't have anything to do except signalling if we work
	 asynchronously.  */

      if (kctx != KCTX_NONE)
	{
	  INTERNAL_SYSCALL_DECL (err);
	  INTERNAL_SYSCALL (io_destroy, err, 1, kctx);
	}

      /* Release the mutex.  We do this before raising a signal since the
	 signal handler might do a `siglongjmp' and then the mutex is
	 locked forever.  */
      pthread_mutex_unlock (&__aio_requests_mutex);

      if (LIO_MODE (mode) == LIO_NOWAIT)
	{
#ifdef BROKEN_THREAD_SIGNALS
	  __aio_notify_only (sig,
			     sig->sigev_notify == SIGEV_SIGNAL ? getpid () : 0);
#else
	  __aio_notify_only (sig);
#endif
	}

      return result;
    }
  else if (LIO_MODE (mode) == LIO_WAIT)
    {
#ifndef DONT_NEED_AIO_MISC_COND
      pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
      int oldstate;
#endif
      struct waitlist waitlist[nent];
      volatile int ktotal = 0;

      total = 0;
      for (cnt = 0; cnt < nent; ++cnt)
	{
	  assert (requests[cnt] == NULL || list[cnt] != NULL);

	  if (requests[cnt] != NULL && list[cnt]->aio_lio_opcode != LIO_NOP)
	    {
	      if (requests[cnt]->kioctx != KCTX_NONE)
		{
		  assert (requests[cnt]->kioctx == kctx);
		  waitlist[cnt].counterp = &ktotal;
		  ++ktotal;
		}
	      else
		{
		  waitlist[cnt].counterp = &total;
	  	  ++total;
		}
#ifndef DONT_NEED_AIO_MISC_COND
	      waitlist[cnt].cond = &cond;
#endif
	      waitlist[cnt].result = &result;
	      waitlist[cnt].next = requests[cnt]->waiting;
	      waitlist[cnt].sigevp = NULL;
#ifdef BROKEN_THREAD_SIGNALS
	      waitlist[cnt].caller_pid = 0;	/* Not needed.  */
#endif
	      requests[cnt]->waiting = &waitlist[cnt];
	    }
	}

      while (ktotal > 0)
	__aio_wait_for_events (kctx, NULL);
#ifdef DONT_NEED_AIO_MISC_COND
      AIO_MISC_WAIT (result, total, NULL, 0);
#else
      /* Since `pthread_cond_wait'/`pthread_cond_timedwait' are cancellation
	 points we must be careful.  We added entries to the waiting lists
	 which we must remove.  So defer cancellation for now.  */
      pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &oldstate);

      while (total > 0)
	pthread_cond_wait (&cond, &__aio_requests_mutex);

      /* Now it's time to restore the cancellation state.  */
      pthread_setcancelstate (oldstate, NULL);

      /* Release the conditional variable.  */
      if (pthread_cond_destroy (&cond) != 0)
	/* This must never happen.  */
	abort ();
#endif

      if (kctx != KCTX_NONE)
	{
	  INTERNAL_SYSCALL_DECL (err);
	  INTERNAL_SYSCALL (io_destroy, err, 1, kctx);
	}

      /* If any of the I/O requests failed, return -1 and set errno.  */
      if (result != 0)
	{
	  __set_errno (result == EINTR ? EINTR : EIO);
	  result = -1;
	}
    }
  else if (sig->sigev_notify != SIGEV_NONE)
    {
      struct async_waitlist *waitlist;

      waitlist = (struct async_waitlist *)
	malloc (sizeof (struct async_waitlist)
		+ (nent * sizeof (struct waitlist)));

      if (waitlist == NULL)
	{
	  __set_errno (EAGAIN);
	  result = -1;
	}
      else
	{
#ifdef BROKEN_THREAD_SIGNALS
	  pid_t caller_pid = sig->sigev_notify == SIGEV_SIGNAL ? getpid () : 0;
#endif
	  total = 0;

	  for (cnt = 0; cnt < nent; ++cnt)
	    {
	      assert (requests[cnt] == NULL || list[cnt] != NULL);

	      if (requests[cnt] != NULL
		  && list[cnt]->aio_lio_opcode != LIO_NOP)
		{
#ifndef DONT_NEED_AIO_MISC_COND
		  waitlist->list[cnt].cond = NULL;
#endif
		  waitlist->list[cnt].result = NULL;
		  waitlist->list[cnt].next = requests[cnt]->waiting;
		  waitlist->list[cnt].counterp = &waitlist->counter;
		  waitlist->list[cnt].sigevp = &waitlist->sigev;
#ifdef BROKEN_THREAD_SIGNALS
		  waitlist->list[cnt].caller_pid = caller_pid;
#endif
		  requests[cnt]->waiting = &waitlist->list[cnt];
		  ++total;
		}
	    }

	  waitlist->counter = total;
	  waitlist->sigev = *sig;
	}
    }

  /* Release the mutex.  */
  pthread_mutex_unlock (&__aio_requests_mutex);

  return result;
}


#if SHLIB_COMPAT (librt, GLIBC_2_1, GLIBC_2_4)
int
attribute_compat_text_section
__lio_listio_21 (int mode, struct aiocb *const list[], int nent,
		 struct sigevent *sig)
{
  /* Check arguments.  */
  if (mode != LIO_WAIT && mode != LIO_NOWAIT)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return lio_listio_internal (mode | LIO_NO_INDIVIDUAL_EVENT, list, nent, sig);
}
compat_symbol (librt, __lio_listio_21, lio_listio, GLIBC_2_1);
#endif


int
__lio_listio_item_notify (int mode, struct aiocb *const list[], int nent,
			  struct sigevent *sig)
{
    /* Check arguments.  */
  if (mode != LIO_WAIT && mode != LIO_NOWAIT)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return lio_listio_internal (mode, list, nent, sig);
}
versioned_symbol (librt, __lio_listio_item_notify, lio_listio, GLIBC_2_4);

#endif
