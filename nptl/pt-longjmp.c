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

#include <setjmp.h>
#include <stdlib.h>
#include "pthreadP.h"


void
__pthread_cleanup_upto (__jmp_buf target, char *targetframe)
{
  struct pthread *self = THREAD_SELF;
  struct _pthread_cleanup_buffer *cbuf;

  for (cbuf = THREAD_GETMEM (self, cleanup);
       cbuf != NULL && _JMPBUF_UNWINDS (target, cbuf);
       cbuf = cbuf->__prev)
    {
#if _STACK_GROWS_DOWN
      if ((char *) cbuf <= targetframe)
        {
          cbuf = NULL;
          break;
        }
#elif _STACK_GROWS_UP
      if ((char *) cbuf >= targetframe)
        {
          cbuf = NULL;
          break;
        }
#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif

      /* Call the cleanup code.  */
      cbuf->__routine (cbuf->__arg);
    }

  THREAD_SETMEM (self, cleanup, cbuf);
}



void
longjmp (jmp_buf env, int val)
{
  __libc_longjmp (env, val);
}
weak_alias (longjmp, siglongjmp)
