/* Look up a versioned symbol in a shared object loaded by `dlopen'.
   Copyright (C) 1995-2000, 2004 Free Software Foundation, Inc.
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

#include <ldsodefs.h>

#if !defined SHARED && defined IS_IN_libdl

void *
weak_function
dlvsym (void *handle, const char *name, const char *version_str)
{
  return __dlvsym (handle, name, version_str, RETURN_ADDRESS (0));
}

#else

struct dlvsym_args
{
  /* The arguments to dlvsym_doit.  */
  void *handle;
  const char *name;
  const char *version;
  void *who;

  /* The return values of dlvsym_doit.  */
  void *sym;
};


static void
dlvsym_doit (void *a)
{
  struct dlvsym_args *args = (struct dlvsym_args *)a;

  args->sym = _dl_vsym (args->handle, args->name, args->version, args->who);
}

void *
__dlvsym (void *handle, const char *name, const char *version_str
	  DL_CALLER_DECL)
{
# ifdef SHARED
  if (__builtin_expect (_dlfcn_hook != NULL, 0))
    return _dlfcn_hook->dlvsym (handle, name, version_str, DL_CALLER);
# endif

  struct dlvsym_args args;
  args.handle = handle;
  args.name = name;
  args.who = DL_CALLER;
  args.version = version_str;

  /* Protect against concurrent loads and unloads.  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));

  void *result = (_dlerror_run (dlvsym_doit, &args) ? NULL : args.sym);

  __rtld_lock_unlock_recursive (GL(dl_load_lock));

  return result;
}
# ifdef SHARED
weak_alias (__dlvsym, dlvsym)
# endif
#endif
