/* Run time dynamic linker.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
extern ElfW(Addr) _dl_sysdep_start (void **start_argptr,
				    void (*dl_main) (const ElfW(Phdr) *phdr,
						     ElfW(Half) phent,
						     ElfW(Addr) *user_entry));
extern void _dl_sysdep_start_cleanup (void);

int _dl_secure;
int _dl_argc;
char **_dl_argv;
const char *_dl_rpath;

static void dl_main (const ElfW(Phdr) *phdr,
		     ElfW(Half) phent,
		     ElfW(Addr) *user_entry);

struct link_map _dl_rtld_map;

ElfW(Addr)
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

  ELF_DYNAMIC_RELOCATE (&bootstrap_map, 0, NULL);


  /* Now life is sane; we can call functions and access global data.
     Set up to use the operating system facilities, and find out from
     the operating system's program loader where to find the program
     header table in core.  */


  /* Transfer data about ourselves to the permanent link_map structure.  */
  _dl_rtld_map.l_addr = bootstrap_map.l_addr;
  _dl_rtld_map.l_ld = bootstrap_map.l_ld;
  memcpy (_dl_rtld_map.l_info, bootstrap_map.l_info,
	  sizeof _dl_rtld_map.l_info);
  _dl_setup_hash (&_dl_rtld_map);

  /* Cache the DT_RPATH stored in ld.so itself; this will be
     the default search path.  */
  _dl_rpath = (void *) (_dl_rtld_map.l_addr +
			_dl_rtld_map.l_info[DT_STRTAB]->d_un.d_ptr +
			_dl_rtld_map.l_info[DT_RPATH]->d_un.d_val);

  /* Call the OS-dependent function to set up life so we can do things like
     file access.  It will call `dl_main' (below) to do all the real work
     of the dynamic linker, and then unwind our frame and run the user
     entry point on the same stack we entered on.  */
  return _dl_sysdep_start (arg, &dl_main);
}


/* Now life is peachy; we can do all normal operations.
   On to the real work.  */

void _start (void);

unsigned int _dl_skip_args;	/* Nonzero if we were run directly.  */

static void
dl_main (const ElfW(Phdr) *phdr,
	 ElfW(Half) phent,
	 ElfW(Addr) *user_entry)
{
  const ElfW(Phdr) *ph;
  struct link_map *l;
  const char *interpreter_name;
  int lazy;
  int list_only = 0;

  if (*user_entry == (ElfW(Addr)) &_start)
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

      l = _dl_map_object (NULL, _dl_argv[0], lt_library);
      phdr = l->l_phdr;
      phent = l->l_phnum;
      l->l_name = (char *) "";
      *user_entry = l->l_entry;
    }
  else
    {
      /* Create a link_map for the executable itself.
	 This will be what dlopen on "" returns.  */
      l = _dl_new_object ((char *) "", "", lt_library);
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
  assert (interpreter_name);	/* How else did we get here?  */

  /* Extract the contents of the dynamic section for easy access.  */
  elf_get_dynamic_info (l->l_ld, l->l_info);
  if (l->l_info[DT_HASH])
    /* Set up our cache of pointers into the hash table.  */
    _dl_setup_hash (l);

  /* Put the link_map for ourselves on the chain so it can be found by
     name.  */
  _dl_rtld_map.l_name = (char *) _dl_rtld_map.l_libname = interpreter_name;
  _dl_rtld_map.l_type = lt_library;
  while (l->l_next)
    l = l->l_next;
  l->l_next = &_dl_rtld_map;
  _dl_rtld_map.l_prev = l;

  /* Load all the libraries specified by DT_NEEDED entries.  */
  _dl_map_object_deps (l);

  /* We are done mapping things, so close the zero-fill descriptor.  */
  __close (_dl_zerofd);
  _dl_zerofd = -1;

  /* XXX if kept, move it so l_next list is in dep order because
     it will determine gdb's search order.
     Perhaps do this always, so later dlopen by name finds it?
     XXX But then gdb always considers it present.  */
  if (_dl_rtld_map.l_opencount == 0)
    {
      /* No DT_NEEDED entry referred to the interpreter object itself,
	 so remove it from the list of visible objects.  */
      _dl_rtld_map.l_prev->l_next = _dl_rtld_map.l_next;
      if (_dl_rtld_map.l_next)
	_dl_rtld_map.l_next->l_prev = _dl_rtld_map.l_prev;
    }

  if (list_only)
    {
      /* We were run just to list the shared libraries.  It is
	 important that we do this before real relocation, because the
	 functions we call below for output may no longer work properly
	 after relocation.  */

      int i;

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

      for (i = 1; i < _dl_argc; ++i)
	{
	  const ElfW(Sym) *ref = NULL;
	  ElfW(Addr) loadbase = _dl_lookup_symbol (_dl_argv[i], &ref,
						   &_dl_default_scope[2],
						   "argument", 0, 0);
	  char buf[20], *bp;
	  buf[sizeof buf - 1] = '\0';
	  bp = _itoa (ref->st_value, &buf[sizeof buf - 1], 16, 0);
	  while (&buf[sizeof buf - 1] - bp < sizeof loadbase * 2)
	    *--bp = '0';
	  _dl_sysdep_message (_dl_argv[i], " found at 0x", bp, NULL);
	  buf[sizeof buf - 1] = '\0';
	  bp = _itoa (loadbase, &buf[sizeof buf - 1], 16, 0);
	  while (&buf[sizeof buf - 1] - bp < sizeof loadbase * 2)
	    *--bp = '0';
	  _dl_sysdep_message (" in object at 0x", bp, "\n", NULL);
	}

      _exit (0);
    }

  lazy = !_dl_secure && *(getenv ("LD_BIND_NOW") ?: "") == '\0';

  {
    /* Now we have all the objects loaded.  Relocate them all except for
       the dynamic linker itself.  We do this in reverse order so that copy
       relocs of earlier objects overwrite the data written by later
       objects.  We do not re-relocate the dynamic linker itself in this
       loop because that could result in the GOT entries for functions we
       call being changed, and that would break us.  It is safe to relocate
       the dynamic linker out of order because it has no copy relocs (we
       know that because it is self-contained).  */

    l = _dl_loaded;
    while (l->l_next)
      l = l->l_next;
    do
      {
	if (l != &_dl_rtld_map)
	  {
	    _dl_relocate_object (l, _dl_object_relocation_scope (l), lazy);
	    *_dl_global_scope_end = NULL;
	  }
	l = l->l_prev;
      } while (l);

    /* Do any necessary cleanups for the startup OS interface code.
       We do these now so that no calls are made after rtld re-relocation
       which might be resolved to different functions than we expect.
       We cannot do this before relocating the other objects because
       _dl_relocate_object might need to call `mprotect' for DT_TEXTREL.  */
    _dl_sysdep_start_cleanup ();

    if (_dl_rtld_map.l_opencount > 0)
      /* There was an explicit ref to the dynamic linker as a shared lib.
	 Re-relocate ourselves with user-controlled symbol definitions.  */
      _dl_relocate_object (&_dl_rtld_map, &_dl_default_scope[2], 0);
  }

  {
    /* Initialize _r_debug.  */
    struct r_debug *r = _dl_debug_initialize (_dl_rtld_map.l_addr);

    l = _dl_loaded;
    if (l->l_info[DT_DEBUG])
      /* There is a DT_DEBUG entry in the dynamic section.  Fill it in
	 with the run-time address of the r_debug structure  */
      l->l_info[DT_DEBUG]->d_un.d_ptr = (ElfW(Addr)) r;

    /* Notify the debugger that all objects are now mapped in.  */
    r->r_state = RT_ADD;
    _dl_debug_state ();
  }

  if (_dl_rtld_map.l_info[DT_INIT])
    {
      /* Call the initializer for the compatibility version of the
	 dynamic linker.  There is no additional initialization
	 required for the ABI-compliant dynamic linker.  */

      (*(void (*) (void)) (_dl_rtld_map.l_addr +
			   _dl_rtld_map.l_info[DT_INIT]->d_un.d_ptr)) ();

      /* Clear the field so a future dlopen won't run it again.  */
      _dl_rtld_map.l_info[DT_INIT] = NULL;
    }

  /* Once we return, _dl_sysdep_start will invoke
     the DT_INIT functions and then *USER_ENTRY.  */
}
