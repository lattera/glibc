/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Thread cancellation */

#include <errno.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"
#include "restart.h"

int pthread_setcancelstate(int state, int * oldstate)
{
  pthread_descr self = thread_self();
  if (state < PTHREAD_CANCEL_ENABLE || state > PTHREAD_CANCEL_DISABLE)
    return EINVAL;
  if (oldstate != NULL) *oldstate = self->p_cancelstate;
  self->p_cancelstate = state;
  if (self->p_canceled &&
      self->p_cancelstate == PTHREAD_CANCEL_ENABLE &&
      self->p_canceltype == PTHREAD_CANCEL_ASYNCHRONOUS)
    pthread_exit(PTHREAD_CANCELED);
  return 0;
}

int pthread_setcanceltype(int type, int * oldtype)
{
  pthread_descr self = thread_self();
  if (type < PTHREAD_CANCEL_DEFERRED || type > PTHREAD_CANCEL_ASYNCHRONOUS)
    return EINVAL;
  if (oldtype != NULL) *oldtype = self->p_canceltype;
  self->p_canceltype = type;
  if (self->p_canceled &&
      self->p_cancelstate == PTHREAD_CANCEL_ENABLE &&
      self->p_canceltype == PTHREAD_CANCEL_ASYNCHRONOUS)
    pthread_exit(PTHREAD_CANCELED);
  return 0;
}

int pthread_cancel(pthread_t thread)
{
  pthread_handle handle = thread_handle(thread);
  int pid;

  __pthread_lock(&handle->h_lock);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  handle->h_descr->p_canceled = 1;
  pid = handle->h_descr->p_pid;
  __pthread_unlock(&handle->h_lock);
  kill(pid, PTHREAD_SIG_CANCEL);
  return 0;
}

void pthread_testcancel(void)
{
  pthread_descr self = thread_self();
  if (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE)
    pthread_exit(PTHREAD_CANCELED);
}

void _pthread_cleanup_push(struct _pthread_cleanup_buffer * buffer,
			   void (*routine)(void *), void * arg)
{
  pthread_descr self = thread_self();
  buffer->routine = routine;
  buffer->arg = arg;
  buffer->prev = self->p_cleanup;
  self->p_cleanup = buffer;
}

void _pthread_cleanup_pop(struct _pthread_cleanup_buffer * buffer,
			  int execute)
{
  pthread_descr self = thread_self();
  if (execute) buffer->routine(buffer->arg);
  self->p_cleanup = buffer->prev;
}

void _pthread_cleanup_push_defer(struct _pthread_cleanup_buffer * buffer,
				 void (*routine)(void *), void * arg)
{
  pthread_descr self = thread_self();
  buffer->routine = routine;
  buffer->arg = arg;
  buffer->canceltype = self->p_canceltype;
  buffer->prev = self->p_cleanup;
  self->p_canceltype = PTHREAD_CANCEL_DEFERRED;
  self->p_cleanup = buffer;
}

void _pthread_cleanup_pop_restore(struct _pthread_cleanup_buffer * buffer,
				  int execute)
{
  pthread_descr self = thread_self();
  if (execute) buffer->routine(buffer->arg);
  self->p_cleanup = buffer->prev;
  self->p_canceltype = buffer->canceltype;
  if (self->p_canceled &&
      self->p_cancelstate == PTHREAD_CANCEL_ENABLE &&
      self->p_canceltype == PTHREAD_CANCEL_ASYNCHRONOUS)
    pthread_exit(PTHREAD_CANCELED);
}

void __pthread_perform_cleanup(void)
{
  pthread_descr self = thread_self();
  struct _pthread_cleanup_buffer * c;
  for (c = self->p_cleanup; c != NULL; c = c->prev) c->routine(c->arg);
}

#ifndef PIC
/* We need a hook to force the cancelation wrappers to be linked in when
   static libpthread is used.  */
extern const int __pthread_provide_wrappers;
static const int * const __pthread_require_wrappers =
  &__pthread_provide_wrappers;
#endif
