/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>
   and Richard Henderson <rth@redhat.com>, 2003.

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

#include <setjmp.h>
#include <stdlib.h>
#include "pthreadP.h"


#ifdef HAVE_FORCED_UNWIND

static _Unwind_Reason_Code
unwind_stop (int version, _Unwind_Action actions,
	     _Unwind_Exception_Class exc_class,
	     struct _Unwind_Exception *exc_obj,
	     struct _Unwind_Context *context, void *stop_parameter)
{
  struct pthread_unwind_buf *buf = stop_parameter;

  /* Do longjmp if we're at "end of stack", aka "end of unwind data".
     We assume there are only C frame without unwind data in between
     here and the jmp_buf target.  Otherwise simply note that the CFA
     of a function is NOT within it's stack frame; it's the SP of the
     previous frame.  */
  if ((actions & _UA_END_OF_STACK)
      || ! _JMPBUF_UNWINDS  (buf->cancel_jmp_buf[0].jmp_buf,
			     _Unwind_GetCFA (context)))
    __libc_longjmp ((struct __jmp_buf_tag *) buf->cancel_jmp_buf, 1);

  return _URC_NO_REASON;
}


static void
unwind_cleanup (_Unwind_Reason_Code reason, struct _Unwind_Exception *exc)
{
  /* Nothing to do.  */
}

#endif	/* have forced unwind */


void
__cleanup_fct_attribute __attribute ((noreturn))
__pthread_unwind (__pthread_unwind_buf_t *buf)
{
  struct pthread_unwind_buf *ibuf = (struct pthread_unwind_buf *) buf;
  struct pthread *self = THREAD_SELF;

  /* Handle the compatibility stuff first.  Execute all handlers
     registered with the old method.  We don't execute them in order,
     instead, they will run first.  */
  struct _pthread_cleanup_buffer *oldp = ibuf->priv.data.cleanup;
  struct _pthread_cleanup_buffer *curp = THREAD_GETMEM (self, cleanup);

  if (curp != oldp)
    {
      do
	{
	  /* Pointer to the next element.  */
	  struct _pthread_cleanup_buffer *nextp = curp->__prev;

	  /* Call the handler.  */
	  curp->__routine (curp->__arg);

	  /* To the next.  */
	  curp = nextp;
	}
      while (curp != oldp);

      /* Mark the current element as handled.  */
      THREAD_SETMEM (self, cleanup, curp);
    }

#ifdef HAVE_FORCED_UNWIND
  /* This is not a catchable exception, so don't provide any details about
     the exception type.  We do need to initialize the field though.  */
  THREAD_SETMEM (self, exc.exception_class, 0);
  THREAD_SETMEM (self, exc.exception_cleanup, unwind_cleanup);

  _Unwind_ForcedUnwind (&self->exc, unwind_stop, ibuf);
#else
  /* We simply jump to the registered setjmp buffer.  */
  __libc_longjmp ((struct __jmp_buf_tag *) ibuf->cancel_jmp_buf, 1);
#endif
  /* NOTREACHED */

  /* We better do not get here.  */
  abort ();
}


void
__cleanup_fct_attribute __attribute ((noreturn))
__pthread_unwind_next (__pthread_unwind_buf_t *buf)
{
  struct pthread_unwind_buf *ibuf = (struct pthread_unwind_buf *) buf;

  __pthread_unwind (ibuf->priv.data.prev);
}
