/* Handle loading and unloading shared objects for internal libc purposes.
   Copyright (C) 1999, 2000, 2001, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.columbia.edu>, 1999.

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
#include <stdlib.h>
#include <ldsodefs.h>

/* The purpose of this file is to provide wrappers around the dynamic
   linker error mechanism (similar to dlopen() et al in libdl) which
   are usable from within libc.  Generally we want to throw away the
   string that dlerror() would return and just pass back a null pointer
   for errors.  This also lets the rest of libc not know about the error
   handling mechanism.

   Much of this code came from gconv_dl.c with slight modifications. */

static int
internal_function
dlerror_run (void (*operate) (void *), void *args)
{
  const char *objname;
  const char *last_errstring = NULL;
  int result;

  (void) GLRO(dl_catch_error) (&objname, &last_errstring, operate, args);

  result = last_errstring != NULL;
  if (result && last_errstring != _dl_out_of_memory)
    free ((char *) last_errstring);

  return result;
}

/* These functions are called by dlerror_run... */

struct do_dlopen_args
{
  /* Argument to do_dlopen.  */
  const char *name;
  /* Opening mode.  */
  int mode;

  /* Return from do_dlopen.  */
  struct link_map *map;
};

struct do_dlsym_args
{
  /* Arguments to do_dlsym.  */
  struct link_map *map;
  const char *name;

  /* Return values of do_dlsym.  */
  lookup_t loadbase;
  const ElfW(Sym) *ref;
};

static void
do_dlopen (void *ptr)
{
  struct do_dlopen_args *args = (struct do_dlopen_args *) ptr;
  /* Open and relocate the shared object.  */
  args->map = _dl_open (args->name, args->mode, NULL, __LM_ID_CALLER);
}

static void
do_dlsym (void *ptr)
{
  struct do_dlsym_args *args = (struct do_dlsym_args *) ptr;
  args->ref = NULL;
  args->loadbase = GLRO(dl_lookup_symbol_x) (args->name, args->map, &args->ref,
					     args->map->l_local_scope, NULL, 0,
					     DL_LOOKUP_RETURN_NEWEST, NULL);
}

static void
do_dlclose (void *ptr)
{
  _dl_close ((struct link_map *) ptr);
}

/* This code is to support __libc_dlopen from __libc_dlopen'ed shared
   libraries.  We need to ensure the statically linked __libc_dlopen
   etc. functions are used instead of the dynamically loaded.  */
struct dl_open_hook
{
  void *(*dlopen_mode) (const char *name, int mode);
  void *(*dlsym) (void *map, const char *name);
  int (*dlclose) (void *map);
};

#ifdef SHARED
extern struct dl_open_hook *_dl_open_hook;
libc_hidden_proto (_dl_open_hook);
struct dl_open_hook *_dl_open_hook __attribute__((nocommon));
libc_hidden_data_def (_dl_open_hook);
#else
static void
do_dlsym_private (void *ptr)
{
  lookup_t l;
  struct r_found_version vers;
  vers.name = "GLIBC_PRIVATE";
  vers.hidden = 1;
  /* vers.hash = _dl_elf_hash (version);  */
  vers.hash = 0x0963cf85;
  vers.filename = NULL;

  struct do_dlsym_args *args = (struct do_dlsym_args *) ptr;
  args->ref = NULL;
  l = GLRO(dl_lookup_symbol_x) (args->name, args->map, &args->ref,
				args->map->l_scope, &vers, 0, 0, NULL);
  args->loadbase = l;
}

static struct dl_open_hook _dl_open_hook =
  {
    .dlopen_mode = __libc_dlopen_mode,
    .dlsym = __libc_dlsym,
    .dlclose = __libc_dlclose
  };
#endif

/* ... and these functions call dlerror_run. */

void *
__libc_dlopen_mode (const char *name, int mode)
{
  struct do_dlopen_args args;
  args.name = name;
  args.mode = mode;

#ifdef SHARED
  if (__builtin_expect (_dl_open_hook != NULL, 0))
    return _dl_open_hook->dlopen_mode (name, mode);
  return (dlerror_run (do_dlopen, &args) ? NULL : (void *) args.map);
#else
  if (dlerror_run (do_dlopen, &args))
    return NULL;

  __libc_register_dl_open_hook (args.map);
  __libc_register_dlfcn_hook (args.map);
  return (void *) args.map;
#endif
}
libc_hidden_def (__libc_dlopen_mode)

#ifndef SHARED
void *
__libc_dlsym_private (struct link_map *map, const char *name)
{
  struct do_dlsym_args sargs;
  sargs.map = map;
  sargs.name = name;

  if (! dlerror_run (do_dlsym_private, &sargs))
    return DL_SYMBOL_ADDRESS (sargs.loadbase, sargs.ref);
  return NULL;
}

void
__libc_register_dl_open_hook (struct link_map *map)
{
  struct dl_open_hook **hook;

  hook = (struct dl_open_hook **) __libc_dlsym_private (map, "_dl_open_hook");
  if (hook != NULL)
    *hook = &_dl_open_hook;
}
#endif

void *
__libc_dlsym (void *map, const char *name)
{
  struct do_dlsym_args args;
  args.map = map;
  args.name = name;

#ifdef SHARED
  if (__builtin_expect (_dl_open_hook != NULL, 0))
    return _dl_open_hook->dlsym (map, name);
#endif
  return (dlerror_run (do_dlsym, &args) ? NULL
	  : (void *) (DL_SYMBOL_ADDRESS (args.loadbase, args.ref)));
}
libc_hidden_def (__libc_dlsym)

int
__libc_dlclose (void *map)
{
#ifdef SHARED
  if (__builtin_expect (_dl_open_hook != NULL, 0))
    return _dl_open_hook->dlclose (map);
#endif
  return dlerror_run (do_dlclose, map);
}
libc_hidden_def (__libc_dlclose)


libc_freeres_fn (free_mem)
{
  struct link_map *l;
  struct r_search_path_elem *d;

  /* Remove all search directories.  */
  d = GL(dl_all_dirs);
  while (d != GLRO(dl_init_all_dirs))
    {
      struct r_search_path_elem *old = d;
      d = d->next;
      free (old);
    }

  /* Remove all additional names added to the objects.  */
  for (Lmid_t ns = 0; ns < DL_NNS; ++ns)
    for (l = GL(dl_ns)[ns]._ns_loaded; l != NULL; l = l->l_next)
      {
	struct libname_list *lnp = l->l_libname->next;

	l->l_libname->next = NULL;

	while (lnp != NULL)
	  {
	    struct libname_list *old = lnp;
	    lnp = lnp->next;
	    if (! old->dont_free)
	    free (old);
	  }
      }
}
