/* Return backtrace of current program state.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

#include <libc-lock.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unwind.h>

struct trace_arg
{
  void **array;
  int cnt, size;
  void *lastfp, *lastsp;
};

#ifdef SHARED
static _Unwind_Reason_Code (*unwind_backtrace) (_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*unwind_getip) (struct _Unwind_Context *);
static _Unwind_Ptr (*unwind_getcfa) (struct _Unwind_Context *);
static _Unwind_Ptr (*unwind_getgr) (struct _Unwind_Context *, int);
static void *libgcc_handle;

static void
init (void)
{
  libgcc_handle = __libc_dlopen ("libgcc_s.so.2");

  if (libgcc_handle == NULL)
    return;

  unwind_backtrace = __libc_dlsym (libgcc_handle, "_Unwind_Backtrace");
  unwind_getip = __libc_dlsym (libgcc_handle, "_Unwind_GetIP");
  unwind_getcfa = __libc_dlsym (libgcc_handle, "_Unwind_GetCFA");
  unwind_getgr = __libc_dlsym (libgcc_handle, "_Unwind_GetGR");
  if (unwind_getip == NULL || unwind_getgr == NULL || unwind_getcfa == NULL)
    {
      unwind_backtrace = NULL;
      __libc_dlclose (libgcc_handle);
      libgcc_handle = NULL;
    }
}
#else
# define unwind_backtrace _Unwind_Backtrace
# define unwind_getip _Unwind_GetIP
# define unwind_getcfa _Unwind_GetCFA
# define unwind_getgr _Unwind_GetGR
#endif

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

  /* %fp is DWARF2 register 14 on M68K.  */
  arg->lastfp = (void *) unwind_getgr (ctx, 14);
  arg->lastsp = (void *) unwind_getcfa (ctx);
  return _URC_NO_REASON;
}


/* This is a global variable set at program start time.  It marks the
   highest used stack address.  */
extern void *__libc_stack_end;


/* This is the stack layout we see with every stack frame
   if not compiled without frame pointer.

           +-----------------+       +-----------------+
    %fp -> | %fp last frame--------> | %fp last frame--->...
           |                 |       |                 |
           | return address  |       | return address  |
           +-----------------+       +-----------------+

   First try as far to get as far as possible using
   _Unwind_Backtrace which handles -fomit-frame-pointer
   as well, but requires .eh_frame info.  Then fall back to
   walking the stack manually.  */

struct layout
{
  struct layout *fp;
  void *ret;
};


int
__backtrace (void **array, int size)
{
  struct trace_arg arg = { .array = array, .size = size, .cnt = -1 };

  if (size <= 0)
    return 0;

#ifdef SHARED
  __libc_once_define (static, once);

  __libc_once (once, init);
  if (unwind_backtrace == NULL)
    return 0;
#endif

  unwind_backtrace (backtrace_helper, &arg);

  if (arg.cnt > 1 && arg.array[arg.cnt - 1] == NULL)
    --arg.cnt;
  else if (arg.cnt < size)
    {
      struct layout *fp = (struct layout *) arg.lastfp;

      while (arg.cnt < size)
	{
	  /* Check for out of range.  */
	  if ((void *) fp < arg.lastsp || (void *) fp > __libc_stack_end
	      || ((long) fp & 1))
	    break;

	  array[arg.cnt++] = fp->ret;
	  fp = fp->fp;
	}
    }
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
