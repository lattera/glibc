/* Copyright (C) 2006-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* We define a special synchronization primitive for AIO.  POSIX
   conditional variables would be ideal but the pthread_cond_*wait
   operations do not return on EINTR.  This is a requirement for
   correct aio_suspend and lio_listio implementations.  */

#include <assert.h>
#include <nptl/pthreadP.h>
#include <futex-internal.h>

#define DONT_NEED_AIO_MISC_COND	1

#define AIO_MISC_NOTIFY(waitlist) \
  do {									      \
    if (*waitlist->counterp > 0 && --*waitlist->counterp == 0)		      \
      futex_wake ((unsigned int *) waitlist->counterp, 1, FUTEX_PRIVATE);     \
  } while (0)

#define AIO_MISC_WAIT(result, futex, timeout, cancel)			      \
  do {									      \
    volatile unsigned int *futexaddr = &futex;				      \
    unsigned int oldval = futex;					      \
									      \
    if (oldval != 0)							      \
      {									      \
	pthread_mutex_unlock (&__aio_requests_mutex);			      \
									      \
	int oldtype;							      \
	if (cancel)							      \
	  oldtype = LIBC_CANCEL_ASYNC ();				      \
									      \
	int status;							      \
	do								      \
	  {								      \
	    status = futex_reltimed_wait ((unsigned int *) futexaddr, oldval, \
					  timeout, FUTEX_PRIVATE);	      \
	    if (status != EAGAIN)					      \
	      break;							      \
									      \
	    oldval = *futexaddr;					      \
	  }								      \
	while (oldval != 0);						      \
									      \
	if (cancel)							      \
	  LIBC_CANCEL_RESET (oldtype);					      \
									      \
	if (status == EINTR)						      \
	  result = EINTR;						      \
	else if (status == ETIMEDOUT)					      \
	  result = EAGAIN;						      \
	else								      \
	  assert (status == 0 || status == EAGAIN);			      \
									      \
	pthread_mutex_lock (&__aio_requests_mutex);			      \
      }									      \
  } while (0)

#include_next <aio_misc.h>
