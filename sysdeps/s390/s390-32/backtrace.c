/* Return backtrace of current program state.
   Copyright (C) 2000, 2001, 2003, 2004, 2005 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>.
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

#include <bits/libc-lock.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stddef.h>
#include <stdlib.h>
#include <unwind.h>

/* This is a global variable set at program start time.  It marks the
   highest used stack address.  */
extern void *__libc_stack_end;


/* This is the stack layout we see for every non-leaf function.
           size                    offset
    %r15 ->    +------------------+
             4 | back chain       |  0
             4 | end of stack     |  4
             8 | glue             |  8
             8 | scratch          | 16
            40 | save area r6-r15 | 24
            16 | save area f4,f6  | 64
            16 | empty            | 80
               +------------------+
   r14 in the save area holds the return address.
*/

struct layout
{
  int back_chain;
  int end_of_stack;
  int glue[2];
  int scratch[2];
  int save_grps[10];
  int save_fp[4];
  int empty[2];
};

struct trace_arg
{
  void **array;
  int cnt, size;
};

#ifdef SHARED
static _Unwind_Reason_Code (*unwind_backtrace) (_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*unwind_getip) (struct _Unwind_Context *);

static void
init (void)
{
  void *handle = __libc_dlopen ("libgcc_s.so.1");

  if (handle == NULL)
    return;

  unwind_backtrace = __libc_dlsym (handle, "_Unwind_Backtrace");
  unwind_getip = __libc_dlsym (handle, "_Unwind_GetIP");
  if (unwind_getip == NULL)
    unwind_backtrace = NULL;
}
#else
# define unwind_backtrace _Unwind_Backtrace
# define unwind_getip _Unwind_GetIP
#endif

static int
__backchain_backtrace (void **array, int size)
{
  /* We assume that all the code is generated with frame pointers set.  */
  struct layout *stack;
  int cnt = 0;

  asm ("LR  %0,%%r15" : "=d" (stack) );
  /* We skip the call to this function, it makes no sense to record it.  */
  stack = (struct layout *) stack->back_chain;
  while (cnt < size)
    {
      if (stack == NULL || (void *) stack > __libc_stack_end)
	/* This means the address is out of range.  Note that for the
	   toplevel we see a frame pointer with value NULL which clearly is
	   out of range.  */
	break;

      array[cnt++] = (void *) (stack->save_grps[8] & 0x7fffffff);

      stack = (struct layout *) stack->back_chain;
    }

  return cnt;
}

static _Unwind_Reason_Code
backtrace_helper (struct _Unwind_Context *ctx, void *a)
{
  struct trace_arg *arg = a;

  /* We are first called with address in the __backtrace function.
     Skip it.  */
  if (arg->cnt != -1)
    arg->array[arg->cnt] = (void *) unwind_getip (ctx);
  if (++arg->cnt == arg->size)
    return _URC_END_OF_STACK;
  return _URC_NO_REASON;
}

int
__backtrace (void **array, int size)
{
  struct trace_arg arg = { .array = array, .size = size, .cnt = -1 };
#ifdef SHARED
  __libc_once_define (static, once);

  __libc_once (once, init);
#endif
  if (unwind_backtrace == NULL)
    return __backchain_backtrace (array, size);

  if (size >= 1)
    unwind_backtrace (backtrace_helper, &arg);

  return arg.cnt != -1 ? arg.cnt : 0;
}

weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)
