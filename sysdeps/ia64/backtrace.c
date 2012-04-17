/* Return backtrace of current program state.
   Copyright (C) 2003-2005, 2007, 2009, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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
#include <stdlib.h>
#include <unwind.h>

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

  /* We are first called with address in the __backtrace function.
     Skip it.  */
  if (arg->cnt != -1)
    {
      arg->array[arg->cnt] = (void *) unwind_getip (ctx);

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
__backtrace (array, size)
     void **array;
     int size;
{
  struct trace_arg arg = { .array = array, .cfa = 0, .size = size, .cnt = -1 };
#ifdef SHARED
  __libc_once_define (static, once);

  __libc_once (once, init);
  if (unwind_backtrace == NULL)
    return 0;
#endif

  if (size >= 1)
    unwind_backtrace (backtrace_helper, &arg);

  /* _Unwind_Backtrace on IA-64 seems to put NULL address above
     _start.  Fix it up here.  */
  if (arg.cnt > 1 && arg.array[arg.cnt - 1] == NULL)
    --arg.cnt;
  return arg.cnt != -1 ? arg.cnt : 0;
}
weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)


#ifdef SHARED
/* Free all resources if necessary.  */
libc_freeres_fn (free_mem)
{
  unwind_backtrace = NULL;
  if (libgcc_handle != NULL)
    {
      __libc_dlclose (libgcc_handle);
      libgcc_handle = NULL;
    }
}
#endif
