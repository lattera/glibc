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

#include <string.h>
#include "pthreadP.h"
#include <lowlevellock.h>


int
pthread_getattr_np (thread_id, attr)
     pthread_t thread_id;
     pthread_attr_t *attr;
{
  struct pthread *thread = (struct pthread *) thread_id;
  struct pthread_attr *iattr = (struct pthread_attr *) attr;

  /* We have to handle cancellation in the following code since we are
     locking another threads desriptor.  */
  pthread_cleanup_push ((void (*) (void *)) lll_unlock_wake_cb, &thread->lock);

  lll_lock (thread->lock);

  /* The thread library is responsible for keeping the values in the
     thread desriptor up-to-date in case the user changes them.  */
  memcpy (&iattr->schedparam, &thread->schedparam,
	  sizeof (struct sched_param));
  iattr->schedpolicy = thread->schedpolicy;

  /* Clear the flags work.  */
  iattr->flags = thread->flags;

  /* The thread might be detached by now.  */
  if (IS_DETACHED (thread))
    iattr->flags |= ATTR_FLAG_DETACHSTATE;

  /* This is the guardsize after adjusting it.  */
  iattr->guardsize = thread->guardsize;

  /* The sizes are subject to alignment.  */
  iattr->stacksize = thread->stackblock_size;
  iattr->stackaddr = (char *) thread->stackblock + iattr->stacksize;
  iattr->flags |= ATTR_FLAG_STACKADDR;

  lll_unlock (thread->lock);

  pthread_cleanup_pop (0);

  return 0;
}
