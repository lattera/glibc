/* Return backtrace of current program state.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David S. Miller <davem@davemloft.net>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <execinfo.h>
#include <stddef.h>
#include <sysdep.h>
#include <sys/trap.h>
#include <dlfcn.h>
#include <unwind.h>
#include <backtrace.h>

struct layout
{
  unsigned long locals[8];
  unsigned long ins[6];
  unsigned long next;
  void *return_address;
};

struct trace_arg
{
  void **array;
  _Unwind_Word cfa;
  int cnt;
  int size;
};

#ifdef SHARED
static _Unwind_Reason_Code (*unwind_backtrace) (_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*unwind_getip) (struct _Unwind_Context *);
static _Unwind_Word (*unwind_getcfa) (struct _Unwind_Context *);
static void *libgcc_handle;

/* Dummy version in case libgcc_s does not contain the real code.  */
static _Unwind_Word
dummy_getcfa (struct _Unwind_Context *ctx __attribute__ ((unused)))
{
  return 0;
}

static void
init (void)
{
  libgcc_handle = __libc_dlopen ("libgcc_s.so.1");

  if (libgcc_handle == NULL)
    return;

  unwind_backtrace = __libc_dlsym (libgcc_handle, "_Unwind_Backtrace");
  unwind_getip = __libc_dlsym (libgcc_handle, "_Unwind_GetIP");
  if (unwind_getip == NULL)
    unwind_backtrace = NULL;
  unwind_getcfa = (__libc_dlsym (libgcc_handle, "_Unwind_GetCFA")
		  ?: dummy_getcfa);
}
#else
# define unwind_backtrace _Unwind_Backtrace
# define unwind_getip _Unwind_GetIP
# define unwind_getcfa _Unwind_GetCFA
#endif

static _Unwind_Reason_Code
backtrace_helper (struct _Unwind_Context *ctx, void *a)
{
  struct trace_arg *arg = a;
  _Unwind_Ptr ip;

  /* We are first called with address in the __backtrace function.
     Skip it.  */
  if (arg->cnt != -1)
    {
      ip = unwind_getip (ctx);
      arg->array[arg->cnt] = (void *) ip;

      /* Check whether we make any progress.  */
      _Unwind_Word cfa = unwind_getcfa (ctx);

      if (arg->cnt > 0 && arg->array[arg->cnt - 1] == arg->array[arg->cnt]
	 && cfa == arg->cfa)
       return _URC_END_OF_STACK;
      arg->cfa = cfa;
    }
  if (++arg->cnt == arg->size)
    return _URC_END_OF_STACK;
  return _URC_NO_REASON;
}

int
__backtrace (void **array, int size)
{
  struct trace_arg arg = { .array = array, .cfa = 0, .size = size, .cnt = -1 };
  bool use_unwinder;
  int count;

  if (!size)
    return 0;

  use_unwinder = true;
#ifdef SHARED
  __libc_once_define (static, once);

  __libc_once (once, init);
  if (unwind_backtrace == NULL)
    use_unwinder = false;
#endif

  if (use_unwinder == false)
    {
      struct layout *current;
      unsigned long fp, i7;

      asm volatile ("mov %%fp, %0" : "=r"(fp));
      asm volatile ("mov %%i7, %0" : "=r"(i7));
      current = (struct layout *) (fp + BACKTRACE_STACK_BIAS);

      array[0] = (void *) i7;

      if (size == 1)
	return 1;

      backtrace_flush_register_windows();
      for (count = 1; count < size; count++)
	{
	  array[count] = current->return_address;
	  if (!current->next)
	    break;
	  current = (struct layout *) (current->next + BACKTRACE_STACK_BIAS);
	}
    }
  else
    {
      unwind_backtrace (backtrace_helper, &arg);

      /* _Unwind_Backtrace seems to put NULL address above
	 _start.  Fix it up here.  */
      if (arg.cnt > 1 && arg.array[arg.cnt - 1] == NULL)
	--arg.cnt;
      count = arg.cnt != -1 ? arg.cnt : 0;
    }
  return count;
}
weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)
