/* Load a shared object at runtime, relocate it, and run its initializer.
   Copyright (C) 1996-2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>		/* Check whether MAP_COPY is defined.  */
#include <sys/param.h>
#include <bits/libc-lock.h>
#include <ldsodefs.h>
#include <bp-sym.h>
#include <caller.h>

#include <dl-dst.h>


#ifndef SHARED
/* Giving this initialized value preallocates some surplus bytes in the
   static TLS area, see __libc_setup_tls (libc-tls.c).  */
size_t _dl_tls_static_size = 2048;
#endif

extern ElfW(Addr) _dl_sysdep_start (void **start_argptr,
				    void (*dl_main) (const ElfW(Phdr) *phdr,
						     ElfW(Word) phnum,
						     ElfW(Addr) *user_entry));
weak_extern (BP_SYM (_dl_sysdep_start))

extern int __libc_multiple_libcs;	/* Defined in init-first.c.  */

extern int __libc_argc attribute_hidden;
extern char **__libc_argv attribute_hidden;

extern char **__environ;

/* Undefine the following for debugging.  */
/* #define SCOPE_DEBUG 1 */
#ifdef SCOPE_DEBUG
static void show_scope (struct link_map *new);
#endif

/* We must be carefull not to leave us in an inconsistent state.  Thus we
   catch any error and re-raise it after cleaning up.  */

struct dl_open_args
{
  const char *file;
  int mode;
  /* This is the caller of the dlopen() function.  */
  const void *caller_dlopen;
  /* This is the caller if _dl_open().  */
  const void *caller_dl_open;
  struct link_map *map;
};


static int
add_to_global (struct link_map *new)
{
  struct link_map **new_global;
  unsigned int to_add = 0;
  unsigned int cnt;

  /* Count the objects we have to put in the global scope.  */
  for (cnt = 0; cnt < new->l_searchlist.r_nlist; ++cnt)
    if (new->l_searchlist.r_list[cnt]->l_global == 0)
      ++to_add;

  /* The symbols of the new objects and its dependencies are to be
     introduced into the global scope that will be used to resolve
     references from other dynamically-loaded objects.

     The global scope is the searchlist in the main link map.  We
     extend this list if necessary.  There is one problem though:
     since this structure was allocated very early (before the libc
     is loaded) the memory it uses is allocated by the malloc()-stub
     in the ld.so.  When we come here these functions are not used
     anymore.  Instead the malloc() implementation of the libc is
     used.  But this means the block from the main map cannot be used
     in an realloc() call.  Therefore we allocate a completely new
     array the first time we have to add something to the locale scope.  */

  if (GL(dl_global_scope_alloc) == 0)
    {
      /* This is the first dynamic object given global scope.  */
      GL(dl_global_scope_alloc) = GL(dl_main_searchlist)->r_nlist + to_add + 8;
      new_global = (struct link_map **)
	malloc (GL(dl_global_scope_alloc) * sizeof (struct link_map *));
      if (new_global == NULL)
	{
	  GL(dl_global_scope_alloc) = 0;
	nomem:
	  GLRO(dl_signal_error) (ENOMEM, new->l_libname->name, NULL,
				 N_("cannot extend global scope"));
	  return 1;
	}

      /* Copy over the old entries.  */
      memcpy (new_global, GL(dl_main_searchlist)->r_list,
	      (GL(dl_main_searchlist)->r_nlist * sizeof (struct link_map *)));

      GL(dl_main_searchlist)->r_list = new_global;
    }
  else if (GL(dl_main_searchlist)->r_nlist + to_add
	   > GL(dl_global_scope_alloc))
    {
      /* We have to extend the existing array of link maps in the
	 main map.  */
      new_global = (struct link_map **)
	realloc (GL(dl_main_searchlist)->r_list,
		 ((GL(dl_global_scope_alloc) + to_add + 8)
		  * sizeof (struct link_map *)));
      if (new_global == NULL)
	goto nomem;

      GL(dl_global_scope_alloc) += to_add + 8;
      GL(dl_main_searchlist)->r_list = new_global;
    }

  /* Now add the new entries.  */
  for (cnt = 0; cnt < new->l_searchlist.r_nlist; ++cnt)
    {
      struct link_map *map = new->l_searchlist.r_list[cnt];

      if (map->l_global == 0)
	{
	  map->l_global = 1;
	  GL(dl_main_searchlist)->r_list[GL(dl_main_searchlist)->r_nlist]
	    = map;
	  ++GL(dl_main_searchlist)->r_nlist;
	}
    }

  return 0;
}


static void
dl_open_worker (void *a)
{
  struct dl_open_args *args = a;
  const char *file = args->file;
  int mode = args->mode;
  struct link_map *new, *l;
  int lazy;
  unsigned int i;
#ifdef USE_TLS
  bool any_tls;
#endif
  struct link_map *call_map = NULL;

  /* Check whether _dl_open() has been called from a valid DSO.  */
  if (__check_caller (args->caller_dl_open, allow_libc|allow_libdl) != 0)
    GLRO(dl_signal_error) (0, "dlopen", NULL, N_("invalid caller"));

  /* Determine the caller's map if necessary.  This is needed in case
     we have a DST or when the file name has no path in which case we
     need to look along the RUNPATH/RPATH of the caller.  */
  const char *dst = strchr (file, '$');
  if (dst != NULL || strchr (file, '/') == NULL)
    {
      const void *caller_dlopen = args->caller_dlopen;

      /* We have to find out from which object the caller is calling.  */
      call_map = NULL;
      for (l = GL(dl_loaded); l; l = l->l_next)
	if (caller_dlopen >= (const void *) l->l_map_start
	    && caller_dlopen < (const void *) l->l_map_end)
	  {
	    /* There must be exactly one DSO for the range of the virtual
	       memory.  Otherwise something is really broken.  */
	    call_map = l;
	    break;
	  }

      if (call_map == NULL)
	/* In this case we assume this is the main application.  */
	call_map = GL(dl_loaded);
    }

  /* Maybe we have to expand a DST.  */
  if (__builtin_expect (dst != NULL, 0))
    {
      size_t len = strlen (file);
      size_t required;
      char *new_file;

      /* DSTs must not appear in SUID/SGID programs.  */
      if (__libc_enable_secure)
	/* This is an error.  */
	GLRO(dl_signal_error) (0, "dlopen", NULL,
			       N_("DST not allowed in SUID/SGID programs"));


      /* Determine how much space we need.  We have to allocate the
	 memory locally.  */
      required = DL_DST_REQUIRED (call_map, file, len, _dl_dst_count (dst, 0));

      /* Get space for the new file name.  */
      new_file = (char *) alloca (required + 1);

      /* Generate the new file name.  */
      _dl_dst_substitute (call_map, file, new_file, 0);

      /* If the substitution failed don't try to load.  */
      if (*new_file == '\0')
	GLRO(dl_signal_error) (0, "dlopen", NULL,
			       N_("empty dynamic string token substitution"));

      /* Now we have a new file name.  */
      file = new_file;

      /* It does not matter whether call_map is set even if we
	 computed it only because of the DST.  Since the path contains
	 a slash the value is not used.  See dl-load.c.  */
    }

  /* Load the named object.  */
  args->map = new = GLRO(dl_map_object) (call_map, file, 0, lt_loaded, 0,
					 mode | __RTLD_CALLMAP);

  /* If the pointer returned is NULL this means the RTLD_NOLOAD flag is
     set and the object is not already loaded.  */
  if (new == NULL)
    {
      assert (mode & RTLD_NOLOAD);
      return;
    }

  if (__builtin_expect (mode & __RTLD_SPROF, 0))
    /* This happens only if we load a DSO for 'sprof'.  */
    return;

  /* It was already open.  */
  if (__builtin_expect (new->l_searchlist.r_list != NULL, 0))
    {
      /* Let the user know about the opencount.  */
      if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FILES, 0))
	GLRO(dl_debug_printf) ("opening file=%s; opencount == %u\n\n",
			       new->l_name, new->l_opencount);

      /* If the user requested the object to be in the global namespace
	 but it is not so far, add it now.  */
      if ((mode & RTLD_GLOBAL) && new->l_global == 0)
	(void) add_to_global (new);

      /* Increment just the reference counter of the object.  */
      ++new->l_opencount;

      return;
    }

  /* Load that object's dependencies.  */
  GLRO(dl_map_object_deps) (new, NULL, 0, 0,
			    mode & (__RTLD_DLOPEN | RTLD_DEEPBIND));

  /* So far, so good.  Now check the versions.  */
  for (i = 0; i < new->l_searchlist.r_nlist; ++i)
    if (new->l_searchlist.r_list[i]->l_versions == NULL)
      (void) GLRO(dl_check_map_versions) (new->l_searchlist.r_list[i], 0, 0);

#ifdef SCOPE_DEBUG
  show_scope (new);
#endif

  /* Only do lazy relocation if `LD_BIND_NOW' is not set.  */
  lazy = (mode & RTLD_BINDING_MASK) == RTLD_LAZY && GLRO(dl_lazy);

  /* Relocate the objects loaded.  We do this in reverse order so that copy
     relocs of earlier objects overwrite the data written by later objects.  */

  l = new;
  while (l->l_next)
    l = l->l_next;
  while (1)
    {
      if (! l->l_relocated)
	{
#ifdef SHARED
	  if (GLRO(dl_profile) != NULL)
	    {
	      /* If this here is the shared object which we want to profile
		 make sure the profile is started.  We can find out whether
	         this is necessary or not by observing the `_dl_profile_map'
	         variable.  If was NULL but is not NULL afterwars we must
		 start the profiling.  */
	      struct link_map *old_profile_map = GL(dl_profile_map);

	      GLRO(dl_relocate_object) (l, l->l_scope, 1, 1);

	      if (old_profile_map == NULL && GL(dl_profile_map) != NULL)
		{
		  /* We must prepare the profiling.  */
		  GLRO(dl_start_profile) ();

		  /* Prevent unloading the object.  */
		  GL(dl_profile_map)->l_flags_1 |= DF_1_NODELETE;
		}
	    }
	  else
#endif
	    GLRO(dl_relocate_object) (l, l->l_scope, lazy, 0);
	}

      if (l == new)
	break;
      l = l->l_prev;
    }

#ifdef USE_TLS
  /* Do static TLS initialization now if it has been delayed because
     the TLS template might not be fully relocated at _dl_allocate_static_tls
     time.  */
  for (l = new; l; l = l->l_next)
    if (l->l_need_tls_init)
      {
	l->l_need_tls_init = 0;
	GL(dl_init_static_tls) (l);
      }

  /* We normally don't bump the TLS generation counter.  There must be
     actually a need to do this.  */
  any_tls = false;
#endif

  /* Increment the open count for all dependencies.  If the file is
     not loaded as a dependency here add the search list of the newly
     loaded object to the scope.  */
  for (i = 0; i < new->l_searchlist.r_nlist; ++i)
    if (++new->l_searchlist.r_list[i]->l_opencount > 1
	&& new->l_searchlist.r_list[i]->l_type == lt_loaded)
      {
	struct link_map *imap = new->l_searchlist.r_list[i];
	struct r_scope_elem **runp = imap->l_scope;
	size_t cnt = 0;

	while (*runp != NULL)
	  {
	    /* This can happen if imap was just loaded, but during
	       relocation had l_opencount bumped because of relocation
	       dependency.  Avoid duplicates in l_scope.  */
	    if (__builtin_expect (*runp == &new->l_searchlist, 0))
	      break;

	    ++cnt;
	    ++runp;
	  }

	if (*runp != NULL)
	  /* Avoid duplicates.  */
	  continue;

	if (__builtin_expect (cnt + 1 >= imap->l_scope_max, 0))
	  {
	    /* The 'r_scope' array is too small.  Allocate a new one
	       dynamically.  */
	    struct r_scope_elem **newp;
	    size_t new_size = imap->l_scope_max * 2;

	    if (imap->l_scope == imap->l_scope_mem)
	      {
		newp = (struct r_scope_elem **)
		  malloc (new_size * sizeof (struct r_scope_elem *));
		if (newp == NULL)
		  GLRO(dl_signal_error) (ENOMEM, "dlopen", NULL,
					 N_("cannot create scope list"));
		imap->l_scope = memcpy (newp, imap->l_scope,
					cnt * sizeof (imap->l_scope[0]));
	      }
	    else
	      {
		newp = (struct r_scope_elem **)
		  realloc (imap->l_scope,
			   new_size * sizeof (struct r_scope_elem *));
		if (newp == NULL)
		  GLRO(dl_signal_error) (ENOMEM, "dlopen", NULL,
					 N_("cannot create scope list"));
		imap->l_scope = newp;
	      }

	    imap->l_scope_max = new_size;
	  }

	imap->l_scope[cnt++] = &new->l_searchlist;
	imap->l_scope[cnt] = NULL;
      }
#if USE_TLS
    else if (new->l_searchlist.r_list[i]->l_opencount == 1
	     /* Only if the module defines thread local data.  */
	     && __builtin_expect (new->l_searchlist.r_list[i]->l_tls_blocksize
				  > 0, 0))
      {
	/* Now that we know the object is loaded successfully add
	   modules containing TLS data to the dtv info table.  We
	   might have to increase its size.  */
	struct dtv_slotinfo_list *listp;
	struct dtv_slotinfo_list *prevp;
	size_t idx = new->l_searchlist.r_list[i]->l_tls_modid;

	assert (new->l_searchlist.r_list[i]->l_type == lt_loaded);

	/* Find the place in the dtv slotinfo list.  */
	listp = GL(dl_tls_dtv_slotinfo_list);
	prevp = NULL;		/* Needed to shut up gcc.  */
	do
	  {
	    /* Does it fit in the array of this list element?  */
	    if (idx < listp->len)
	      break;
	    idx -= listp->len;
	    prevp = listp;
	    listp = listp->next;
	  }
	while (listp != NULL);

	if (listp == NULL)
	  {
	    /* When we come here it means we have to add a new element
	       to the slotinfo list.  And the new module must be in
	       the first slot.  */
	    assert (idx == 0);

	    listp = prevp->next = (struct dtv_slotinfo_list *)
	      malloc (sizeof (struct dtv_slotinfo_list)
		      + TLS_SLOTINFO_SURPLUS * sizeof (struct dtv_slotinfo));
	    if (listp == NULL)
	      {
		/* We ran out of memory.  We will simply fail this
		   call but don't undo anything we did so far.  The
		   application will crash or be terminated anyway very
		   soon.  */

		/* We have to do this since some entries in the dtv
		   slotinfo array might already point to this
		   generation.  */
		++GL(dl_tls_generation);

		GLRO(dl_signal_error) (ENOMEM, "dlopen", NULL, N_("\
cannot create TLS data structures"));
	      }

	    listp->len = TLS_SLOTINFO_SURPLUS;
	    listp->next = NULL;
	    memset (listp->slotinfo, '\0',
		    TLS_SLOTINFO_SURPLUS * sizeof (struct dtv_slotinfo));
	  }

	/* Add the information into the slotinfo data structure.  */
	listp->slotinfo[idx].map = new->l_searchlist.r_list[i];
	listp->slotinfo[idx].gen = GL(dl_tls_generation) + 1;

	/* We have to bump the generation counter.  */
	any_tls = true;
      }

  /* Bump the generation number if necessary.  */
  if (any_tls)
    if (__builtin_expect (++GL(dl_tls_generation) == 0, 0))
      __libc_fatal (_("TLS generation counter wrapped!  Please send report with the 'glibcbug' script."));
#endif

  /* Run the initializer functions of new objects.  */
  GLRO(dl_init) (new, __libc_argc, __libc_argv, __environ);

  /* Now we can make the new map available in the global scope.  */
  if (mode & RTLD_GLOBAL)
    /* Move the object in the global namespace.  */
    if (add_to_global (new) != 0)
      /* It failed.  */
      return;

  /* Mark the object as not deletable if the RTLD_NODELETE flags was
     passed.  */
  if (__builtin_expect (mode & RTLD_NODELETE, 0))
    new->l_flags_1 |= DF_1_NODELETE;

#ifndef SHARED
  /* We must be the static _dl_open in libc.a.  A static program that
     has loaded a dynamic object now has competition.  */
  __libc_multiple_libcs = 1;
#endif

  /* Let the user know about the opencount.  */
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_FILES, 0))
    GLRO(dl_debug_printf) ("opening file=%s; opencount == %u\n\n",
			   new->l_name, new->l_opencount);
}


void *
internal_function
_dl_open (const char *file, int mode, const void *caller_dlopen)
{
  struct dl_open_args args;
  const char *objname;
  const char *errstring;
  int errcode;

  if ((mode & RTLD_BINDING_MASK) == 0)
    /* One of the flags must be set.  */
    GLRO(dl_signal_error) (EINVAL, file, NULL,
			   N_("invalid mode for dlopen()"));

  /* Make sure we are alone.  */
  __rtld_lock_lock_recursive (GL(dl_load_lock));

  args.file = file;
  args.mode = mode;
  args.caller_dlopen = caller_dlopen;
  args.caller_dl_open = RETURN_ADDRESS (0);
  args.map = NULL;
  errcode = GLRO(dl_catch_error) (&objname, &errstring, dl_open_worker, &args);

#ifndef MAP_COPY
  /* We must munmap() the cache file.  */
  GLRO(dl_unload_cache) ();
#endif

  /* Release the lock.  */
  __rtld_lock_unlock_recursive (GL(dl_load_lock));

  if (__builtin_expect (errstring != NULL, 0))
    {
      /* Some error occurred during loading.  */
      char *local_errstring;
      size_t len_errstring;

      /* Remove the object from memory.  It may be in an inconsistent
	 state if relocation failed, for example.  */
      if (args.map)
	{
	  unsigned int i;

	  /* Increment open counters for all objects since this
	     sometimes has not happened yet.  */
	  if (args.map->l_searchlist.r_list[0]->l_opencount == 0)
	    for (i = 0; i < args.map->l_searchlist.r_nlist; ++i)
	      ++args.map->l_searchlist.r_list[i]->l_opencount;

#ifdef USE_TLS
	  /* Maybe some of the modules which were loaded uses TLS.
	     Since it will be removed in the following _dl_close call
	     we have to mark the dtv array as having gaps to fill
	     the holes.  This is a pessimistic assumption which won't
	     hurt if not true.  */
	  GL(dl_tls_dtv_gaps) = true;
#endif

	  _dl_close (args.map);
	}

      /* Make a local copy of the error string so that we can release the
	 memory allocated for it.  */
      len_errstring = strlen (errstring) + 1;
      if (objname == errstring + len_errstring)
	{
	  size_t total_len = len_errstring + strlen (objname) + 1;
	  local_errstring = alloca (total_len);
	  memcpy (local_errstring, errstring, total_len);
	  objname = local_errstring + len_errstring;
	}
      else
	{
	  local_errstring = alloca (len_errstring);
	  memcpy (local_errstring, errstring, len_errstring);
	}

      if (errstring != _dl_out_of_memory)
	free ((char *) errstring);

      /* Reraise the error.  */
      GLRO(dl_signal_error) (errcode, objname, NULL, local_errstring);
    }

#ifndef SHARED
  DL_STATIC_INIT (args.map);
#endif

  return args.map;
}
libc_hidden_def (_dl_open)


#ifdef SCOPE_DEBUG
#include <unistd.h>

static void
show_scope (struct link_map *new)
{
  int scope_cnt;

  for (scope_cnt = 0; new->l_scope[scope_cnt] != NULL; ++scope_cnt)
    {
      char numbuf[2];
      unsigned int cnt;

      numbuf[0] = '0' + scope_cnt;
      numbuf[1] = '\0';
      _dl_printf ("scope %s:", numbuf);

      for (cnt = 0; cnt < new->l_scope[scope_cnt]->r_nlist; ++cnt)
	if (*new->l_scope[scope_cnt]->r_list[cnt]->l_name)
	  _dl_printf (" %s", new->l_scope[scope_cnt]->r_list[cnt]->l_name);
	else
	  _dl_printf (" <main>");

      _dl_printf ("\n");
    }
}
#endif
