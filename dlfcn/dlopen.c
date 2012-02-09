/* Load a shared object at run time.
   Copyright (C) 1995-2000,2003,2004,2005 Free Software Foundation, Inc.
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

#include <dlfcn.h>
#include <libintl.h>
#include <stddef.h>
#include <unistd.h>
#include <ldsodefs.h>

#if !defined SHARED && defined IS_IN_libdl

void *
dlopen (const char *file, int mode)
{
  return __dlopen (file, mode, RETURN_ADDRESS (0));
}
static_link_warning (dlopen)

#else

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
# ifdef SHARED
#  define NS __LM_ID_CALLER
# else
#  define NS LM_ID_BASE
# endif


static void
dlopen_doit (void *a)
{
  struct dlopen_args *args = (struct dlopen_args *) a;

  if (args->mode & ~(RTLD_BINDING_MASK | RTLD_NOLOAD | RTLD_DEEPBIND
		     | RTLD_GLOBAL | RTLD_LOCAL | RTLD_NODELETE
		     | __RTLD_SPROF))
    GLRO(dl_signal_error) (0, NULL, NULL, _("invalid mode parameter"));

  args->new = GLRO(dl_open) (args->file ?: "", args->mode | __RTLD_DLOPEN,
			     args->caller,
			     args->file == NULL ? LM_ID_BASE : NS,
			     __dlfcn_argc, __dlfcn_argv, __environ);
}


void *
__dlopen (const char *file, int mode DL_CALLER_DECL)
{
# ifdef SHARED
  if (__builtin_expect (_dlfcn_hook != NULL, 0))
    return _dlfcn_hook->dlopen (file, mode, DL_CALLER);
# endif

  struct dlopen_args args;
  args.file = file;
  args.mode = mode;
  args.caller = DL_CALLER;

# ifdef SHARED
  return _dlerror_run (dlopen_doit, &args) ? NULL : args.new;
# else
  if (_dlerror_run (dlopen_doit, &args))
    return NULL;

  __libc_register_dl_open_hook ((struct link_map *) args.new);
  __libc_register_dlfcn_hook ((struct link_map *) args.new);

  return args.new;
# endif
}
# ifdef SHARED
#  include <shlib-compat.h>
strong_alias (__dlopen, __dlopen_check)
versioned_symbol (libdl, __dlopen_check, dlopen, GLIBC_2_1);
# endif
#endif
