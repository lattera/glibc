/* Load a shared object at runtime, relocate it, and run its initializer.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <bits/libc-lock.h>
#include <elf/ldsodefs.h>


extern ElfW(Addr) _dl_sysdep_start (void **start_argptr,
				    void (*dl_main) (const ElfW(Phdr) *phdr,
						     ElfW(Word) phnum,
						     ElfW(Addr) *user_entry));
weak_extern (_dl_sysdep_start)

extern int __libc_multiple_libcs;	/* Defined in init-first.c.  */

extern int __libc_argc;
extern char **__libc_argv;

extern char **__environ;

static size_t _dl_global_scope_alloc;


/* During the program run we must not modify the global data of
   loaded shared object simultanously in two threads.  Therefore we
   protect `_dl_open' and `_dl_close' in dl-close.c.

   This must be a recursive lock since the initializer function of
   the loaded object might as well require a call to this function.
   At this time it is not anymore a problem to modify the tables.  */
__libc_lock_define_initialized_recursive (, _dl_load_lock)


struct link_map *
internal_function
_dl_open (const char *file, int mode)
{
  struct link_map *new, *l;
  ElfW(Addr) init;
  struct r_debug *r;

  /* Make sure we are alone.  */
  __libc_lock_lock (_dl_load_lock);

  /* Load the named object.  */
  new = _dl_map_object (NULL, file, 0, lt_loaded, 0);
  if (new->l_searchlist)
    {
      /* It was already open.  */
      __libc_lock_unlock (_dl_load_lock);
      return new;
    }

  /* Load that object's dependencies.  */
  _dl_map_object_deps (new, NULL, 0, 0);

  /* So far, so good.  Now check the versions.  */
  (void) _dl_check_all_versions (new, 0);

  /* Relocate the objects loaded.  We do this in reverse order so that copy
     relocs of earlier objects overwrite the data written by later objects.  */

  l = new;
  while (l->l_next)
    l = l->l_next;
  while (1)
    {
      if (! l->l_relocated)
	{
	  /* We use an indirect call call for _dl_relocate_object because
	     we must avoid using the PLT in the call.  If our PLT entry for
	     _dl_relocate_object hasn't been used yet, then the dynamic
	     linker fixup routine will clobber _dl_global_scope during its
	     work.  We must be sure that nothing will require a PLT fixup
	     between when _dl_object_relocation_scope returns and when we
	     enter the dynamic linker's code (_dl_relocate_object).  */
	  __typeof (_dl_relocate_object) *reloc = &_dl_relocate_object;

	  /* GCC is very clever.  If we wouldn't add some magic it would
	     simply optimize away our nice little variable `reloc' and we
	     would result in a not working binary.  So let's swing the
	     magic ward.  */
	  asm ("" : "=r" (reloc) : "0" (reloc));

#ifdef PIC
	  if (_dl_profile != NULL)
	    {
	      /* If this here is the shared object which we want to profile
		 make sure the profile is started.  We can find out whether
	         this is necessary or not by observing the `_dl_profile_map'
	         variable.  If was NULL but is not NULL afterwars we must
		 start the profiling.  */
	      struct link_map *old_profile_map = _dl_profile_map;

	      (*reloc) (l, _dl_object_relocation_scope (l), 1, 1);

	      if (old_profile_map == NULL && _dl_profile_map != NULL)
		/* We must prepare the profiling.  */
		_dl_start_profile (_dl_profile_map, _dl_profile_output);
	    }
	  else
#endif
	    (*reloc) (l, _dl_object_relocation_scope (l),
		      (mode & RTLD_BINDING_MASK) == RTLD_LAZY, 0);

	  *_dl_global_scope_end = NULL;
	}

      if (l == new)
	break;
      l = l->l_prev;
    }

  new->l_global = (mode & RTLD_GLOBAL) ? 1 : 0;
  if (new->l_global)
    {
      /* The symbols of the new object and its dependencies are to be
	 introduced into the global scope that will be used to resolve
	 references from other dynamically-loaded objects.  */

      if (_dl_global_scope_alloc == 0)
	{
	  /* This is the first dynamic object given global scope.  */
	  _dl_global_scope_alloc = 8;
	  _dl_global_scope = malloc (_dl_global_scope_alloc
				     * sizeof (struct link_map *));
	  if (! _dl_global_scope)
	    {
	      _dl_global_scope = _dl_default_scope;
	    nomem:
	      _dl_close (new);
	      _dl_signal_error (ENOMEM, file, "cannot extend global scope");
	    }
	  _dl_global_scope[2] = _dl_default_scope[2];
	  _dl_global_scope[3] = new;
	  _dl_global_scope[4] = NULL;
	  _dl_global_scope[5] = NULL;
	  _dl_global_scope_end = &_dl_global_scope [4];
	}
      else
	{
	  if (_dl_global_scope_alloc <
	      (size_t) (_dl_global_scope_end - _dl_global_scope + 2))
	    {
	      /* Must extend the list.  */
	      struct link_map **new = realloc (_dl_global_scope,
					       _dl_global_scope_alloc * 2
					       * sizeof (struct link_map *));
	      if (! new)
		goto nomem;
	      _dl_global_scope_end = new + (_dl_global_scope_end -
					    _dl_global_scope);
	      _dl_global_scope = new;
	      _dl_global_scope_alloc *= 2;
	    }

	  /* Append the new object and re-terminate the list.  */
	  *_dl_global_scope_end++ = new;
	  /* We keep the list double-terminated so the last element
	     can be filled in for symbol lookups.  */
	  _dl_global_scope_end[0] = NULL;
	  _dl_global_scope_end[1] = NULL;
	}
    }


  /* Notify the debugger we have added some objects.  We need to call
     _dl_debug_initialize in a static program in case dynamic linking has
     not been used before.  */
  r = _dl_debug_initialize (0);
  r->r_state = RT_ADD;
  _dl_debug_state ();

  /* Run the initializer functions of new objects.  */
  while (init = _dl_init_next (new))
    (*(void (*) (int, char **, char **)) init) (__libc_argc, __libc_argv,
						__environ);

  if (_dl_sysdep_start == NULL)
    /* We must be the static _dl_open in libc.a.  A static program that
       has loaded a dynamic object now has competition.  */
    __libc_multiple_libcs = 1;

  /* Release the lock.  */
  __libc_lock_unlock (_dl_load_lock);

  return new;
}
