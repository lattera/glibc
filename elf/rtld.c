/* Run time dynamic linker.
Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <link.h>
#include "dynamic-link.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "../stdio-common/_itoa.h"


#ifdef RTLD_START
RTLD_START
#else
#error "sysdeps/MACHINE/dl-machine.h fails to define RTLD_START"
#endif

/* System-specific function to do initial startup for the dynamic linker.
   After this, file access calls and getenv must work.  This is responsible
   for setting _dl_secure if we need to be secure (e.g. setuid),
   and for setting _dl_argc and _dl_argv, and then calling _dl_main.  */
extern Elf32_Addr _dl_sysdep_start (void **start_argptr,
				    void (*dl_main) (const Elf32_Phdr *phdr,
						     Elf32_Word phent,
						     Elf32_Addr *user_entry));
extern void _dl_sysdep_start_cleanup (void);

int _dl_secure;
int _dl_argc;
char **_dl_argv;
const char *_dl_rpath;

struct r_debug dl_r_debug;

static void dl_main (const Elf32_Phdr *phdr,
		     Elf32_Word phent,
		     Elf32_Addr *user_entry);

static struct link_map rtld_map;

Elf32_Addr
_dl_start (void *arg)
{
  struct link_map bootstrap_map;

  /* Figure out the run-time load address of the dynamic linker itself.  */
  bootstrap_map.l_addr = elf_machine_load_address ();

  /* Read our own dynamic section and fill in the info array.
     Conveniently, the first element of the GOT contains the
     offset of _DYNAMIC relative to the run-time load address.  */
  bootstrap_map.l_ld = (void *) bootstrap_map.l_addr + *elf_machine_got ();
  elf_get_dynamic_info (bootstrap_map.l_ld, bootstrap_map.l_info);

#ifdef ELF_MACHINE_BEFORE_RTLD_RELOC
  ELF_MACHINE_BEFORE_RTLD_RELOC (bootstrap_map.l_info);
#endif

  /* Relocate ourselves so we can do normal function calls and
     data access using the global offset table.  */

  /* We must initialize `l_type' to make sure it is not `lt_interpreter'.
     That is the type to describe us, but not during bootstrapping--it
     indicates to elf_machine_rel{,a} that we were already relocated during
     bootstrapping, so it must anti-perform each bootstrapping relocation
     before applying the final relocation when ld.so is linked in as
     normal a shared library.  */
  bootstrap_map.l_type = lt_library;
  ELF_DYNAMIC_RELOCATE (&bootstrap_map, 0, NULL);


  /* Now life is sane; we can call functions and access global data.
     Set up to use the operating system facilities, and find out from
     the operating system's program loader where to find the program
     header table in core.  */


  /* Transfer data about ourselves to the permanent link_map structure.  */
  rtld_map.l_addr = bootstrap_map.l_addr;
  rtld_map.l_ld = bootstrap_map.l_ld;
  memcpy (rtld_map.l_info, bootstrap_map.l_info, sizeof rtld_map.l_info);
  _dl_setup_hash (&rtld_map);

  /* Cache the DT_RPATH stored in ld.so itself; this will be
     the default search path.  */
  _dl_rpath = (void *) (rtld_map.l_addr +
			rtld_map.l_info[DT_STRTAB]->d_un.d_ptr +
			rtld_map.l_info[DT_RPATH]->d_un.d_val);

  /* Call the OS-dependent function to set up life so we can do things like
     file access.  It will call `dl_main' (below) to do all the real work
     of the dynamic linker, and then unwind our frame and run the user
     entry point on the same stack we entered on.  */
  return _dl_sysdep_start (&arg, &dl_main);
}


/* Now life is peachy; we can do all normal operations.
   On to the real work.  */

void _start (void);

unsigned int _dl_skip_args;	/* Nonzero if we were run directly.  */

static void
dl_main (const Elf32_Phdr *phdr,
	 Elf32_Word phent,
	 Elf32_Addr *user_entry)
{
  void doit (void)
    {
      const Elf32_Phdr *ph;
      struct link_map *l, *last, *before_rtld;
      const char *interpreter_name;
      int lazy;
      int list_only = 0;

      if (*user_entry == (Elf32_Addr) &_start)
	{
	  /* Ho ho.  We are not the program interpreter!  We are the program
	     itself!  This means someone ran ld.so as a command.  Well, that
	     might be convenient to do sometimes.  We support it by
	     interpreting the args like this:

	     ld.so PROGRAM ARGS...

	     The first argument is the name of a file containing an ELF
	     executable we will load and run with the following arguments.
	     To simplify life here, PROGRAM is searched for using the
	     normal rules for shared objects, rather than $PATH or anything
	     like that.  We just load it and use its entry point; we don't
	     pay attention to its PT_INTERP command (we are the interpreter
	     ourselves).  This is an easy way to test a new ld.so before
	     installing it.  */
	  if (_dl_argc < 2)
	    _dl_sysdep_fatal ("\
Usage: ld.so [--list] EXECUTABLE-FILE [ARGS-FOR-PROGRAM...]\n\
You have invoked `ld.so', the helper program for shared library executables.\n\
This program usually lives in the file `/lib/ld.so', and special directives\n\
in executable files using ELF shared libraries tell the system's program\n\
loader to load the helper program from this file.  This helper program loads\n\
the shared libraries needed by the program executable, prepares the program\n\
to run, and runs it.  You may invoke this helper program directly from the\n\
command line to load and run an ELF executable file; this is like executing\n\
that file itself, but always uses this helper program from the file you\n\
specified, instead of the helper program file specified in the executable\n\
file you run.  This is mostly of use for maintainers to test new versions\n\
of this helper program; chances are you did not intend to run this program.\n",
			      NULL);

	  interpreter_name = _dl_argv[0];

	  if (! strcmp (_dl_argv[1], "--list"))
	    {
	      list_only = 1;

	      ++_dl_skip_args;
	      --_dl_argc;
	      ++_dl_argv;
	    }

	  ++_dl_skip_args;
	  --_dl_argc;
	  ++_dl_argv;

	  l = _dl_map_object (NULL, _dl_argv[0]);
	  phdr = l->l_phdr;
	  phent = l->l_phnum;
	  l->l_name = (char *) "";
	  *user_entry = l->l_entry;
	}
      else
	{
	  /* Create a link_map for the executable itself.
	     This will be what dlopen on "" returns.  */
	  l = _dl_new_object ((char *) "", "", lt_executable);
	  l->l_phdr = phdr;
	  l->l_phnum = phent;
	  interpreter_name = 0;
	  l->l_entry = *user_entry;
	}

      if (l != _dl_loaded)
	{
	  /* GDB assumes that the first element on the chain is the
	     link_map for the executable itself, and always skips it.
	     Make sure the first one is indeed that one.  */
	  l->l_prev->l_next = l->l_next;
	  if (l->l_next)
	    l->l_next->l_prev = l->l_prev;
	  l->l_prev = NULL;
	  l->l_next = _dl_loaded;
	  _dl_loaded->l_prev = l;
	  _dl_loaded = l;
	}

      /* Scan the program header table for the dynamic section.  */
      for (ph = phdr; ph < &phdr[phent]; ++ph)
	switch (ph->p_type)
	  {
	  case PT_DYNAMIC:
	    /* This tells us where to find the dynamic section,
	       which tells us everything we need to do.  */
	    l->l_ld = (void *) l->l_addr + ph->p_vaddr;
	    break;
	  case PT_INTERP:
	    /* This "interpreter segment" was used by the program loader to
	       find the program interpreter, which is this program itself, the
	       dynamic linker.  We note what name finds us, so that a future
	       dlopen call or DT_NEEDED entry, for something that wants to link
	       against the dynamic linker as a shared library, will know that
	       the shared object is already loaded.  */
	    interpreter_name = (void *) l->l_addr + ph->p_vaddr;
	    break;
	  }
      assert (interpreter_name); /* How else did we get here?  */

      /* Extract the contents of the dynamic section for easy access.  */
      elf_get_dynamic_info (l->l_ld, l->l_info);
      if (l->l_info[DT_HASH])
	/* Set up our cache of pointers into the hash table.  */
	_dl_setup_hash (l);

      if (l->l_info[DT_DEBUG])
	/* There is a DT_DEBUG entry in the dynamic section.  Fill it in
	   with the run-time address of the r_debug structure, which we
	   will set up later to communicate with the debugger.  */
	l->l_info[DT_DEBUG]->d_un.d_ptr = (Elf32_Addr) &dl_r_debug;

      /* Put the link_map for ourselves on the chain so it can be found by
	 name.  */
      rtld_map.l_name = (char *) rtld_map.l_libname = interpreter_name;
      rtld_map.l_type = lt_interpreter;
      while (l->l_next)
	l = l->l_next;
      l->l_next = &rtld_map;
      rtld_map.l_prev = l;

      /* Now process all the DT_NEEDED entries and map in the objects.
	 Each new link_map will go on the end of the chain, so we will
	 come across it later in the loop to map in its dependencies.  */
      before_rtld = NULL;
      for (l = _dl_loaded; l; l = l->l_next)
	{
	  if (l->l_info[DT_NEEDED])
	    {
	      const char *strtab
		= (void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr;
	      const Elf32_Dyn *d;
	      last = l;
	      for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
		if (d->d_tag == DT_NEEDED)
		  {
		    struct link_map *new;
		    new = _dl_map_object (l, strtab + d->d_un.d_val);
		    new->l_type = lt_library;
		    if (!before_rtld && new == &rtld_map)
		      before_rtld = last;
		    last = new;
		  }
	    }
	  l->l_deps_loaded = 1;
	}

      /* If any DT_NEEDED entry referred to the interpreter object itself,
	 reorder the list so it appears after its dependent.  If not,
	 remove it from the maps we will use for symbol resolution.  */
      rtld_map.l_prev->l_next = rtld_map.l_next;
      if (rtld_map.l_next)
	rtld_map.l_next->l_prev = rtld_map.l_prev;
      if (before_rtld)
	{
	  rtld_map.l_prev = before_rtld;
	  rtld_map.l_next = before_rtld->l_next;
	  before_rtld->l_next = &rtld_map;
	  if (rtld_map.l_next)
	    rtld_map.l_next->l_prev = &rtld_map;
	}

      if (list_only)
	{
	  /* We were run just to list the shared libraries.  It is
	     important that we do this before real relocation, because the
	     functions we call below for output may no longer work properly
	     after relocation.  */

	  if (! _dl_loaded->l_info[DT_NEEDED])
	    _dl_sysdep_message ("\t", "statically linked\n", NULL);
	  else
	    for (l = _dl_loaded->l_next; l; l = l->l_next)
	      {
		char buf[20], *bp;
		buf[sizeof buf - 1] = '\0';
		bp = _itoa (l->l_addr, &buf[sizeof buf - 1], 16, 0);
		while (&buf[sizeof buf - 1] - bp < sizeof l->l_addr * 2)
		  *--bp = '0';
		_dl_sysdep_message ("\t", l->l_libname, " => ", l->l_name,
				    " (0x", bp, ")\n", NULL);
	      }

	  _exit (0);
	}

      lazy = !_dl_secure && *(getenv ("LD_BIND_NOW") ?: "") == '\0';

      /* Do any necessary cleanups for the startup OS interface code.
	 We do these now so that no calls are made after real relocation
	 which might be resolved to different functions than we expect.  */
      _dl_sysdep_start_cleanup ();

      /* Now we have all the objects loaded.  Relocate them all.
	 We do this in reverse order so that copy relocs of earlier
	 objects overwrite the data written by later objects.  */
      l = _dl_loaded;
      while (l->l_next)
	l = l->l_next;
      do
	{
	  _dl_relocate_object (l, lazy);
	  l = l->l_prev;
	} while (l);

      /* Tell the debugger where to find the map of loaded objects.  */
      dl_r_debug.r_version = 1	/* R_DEBUG_VERSION XXX */;
      dl_r_debug.r_ldbase = rtld_map.l_addr; /* Record our load address.  */
      dl_r_debug.r_map = _dl_loaded;
      dl_r_debug.r_brk = (Elf32_Addr) &_dl_r_debug_state;

      if (rtld_map.l_info[DT_INIT])
	{
	  /* Call the initializer for the compatibility version of the
	     dynamic linker.  There is no additional initialization
	     required for the ABI-compliant dynamic linker.  */

	  (*(void (*) (void)) (rtld_map.l_addr +
			       rtld_map.l_info[DT_INIT]->d_un.d_ptr)) ();

	  /* Clear the field so a future dlopen won't run it again.  */
	  rtld_map.l_info[DT_INIT] = NULL;
	}
    }
  const char *errstring;
  const char *errobj;
  int err;

  err = _dl_catch_error (&errstring, &errobj, &doit);
  if (errstring)
    _dl_sysdep_fatal (_dl_argv[0] ?: "<program name unknown>",
		      ": error in loading shared libraries\n",
		      errobj ?: "", errobj ? ": " : "",
		      errstring, err ? ": " : "",
		      err ? strerror (err) : "", "\n", NULL);

  /* Once we return, _dl_sysdep_start will invoke
     the DT_INIT functions and then *USER_ENTRY.  */
}

/* This function exists solely to have a breakpoint set on it by the
   debugger.  */
void
_dl_r_debug_state (void)
{
}
