/* Load a shared object at run time.
   Copyright (C) 1995-1999, 2000, 2004 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <stddef.h>
#include <unistd.h>
#include <ldsodefs.h>

/* This file is for compatibility with glibc 2.0.  Compile it only if
   versioning is used.  */
#include <shlib-compat.h>
#if SHLIB_COMPAT (libdl, GLIBC_2_0, GLIBC_2_1)

struct dlopen_args
{
  /* The arguments for dlopen_doit.  */
  const char *file;
  int mode;
  /* The return value of dlopen_doit.  */
  void *new;
  /* Address of the caller.  */
  const void *caller;
};


/* Non-shared code has no support for multiple namespaces.  */
#ifdef SHARED
# define NS __LM_ID_CALLER
#else
# define NS LM_ID_BASE
#endif


static void
dlopen_doit (void *a)
{
  struct dlopen_args *args = (struct dlopen_args *) a;

  args->new = GLRO(dl_open) (args->file ?: "", args->mode | __RTLD_DLOPEN,
			     args->caller,
			     args->file == NULL ? LM_ID_BASE : NS,
			     __dlfcn_argc, __dlfcn_argv, __environ);
}

extern void *__dlopen_nocheck (const char *file, int mode);
void *
__dlopen_nocheck (const char *file, int mode)
{
  struct dlopen_args args;
  args.file = file;
  args.caller = RETURN_ADDRESS (0);

  if ((mode & RTLD_BINDING_MASK) == 0)
    /* By default assume RTLD_LAZY.  */
    mode |= RTLD_LAZY;
  args.mode = mode;

  if (__builtin_expect (_dlfcn_hook != NULL, 0))
    return _dlfcn_hook->dlopen (file, mode, RETURN_ADDRESS (0));

  return _dlerror_run (dlopen_doit, &args) ? NULL : args.new;
}
compat_symbol (libdl, __dlopen_nocheck, dlopen, GLIBC_2_0);
#endif
