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

int _dl_secure;
int _dl_argc;
char **_dl_argv;

struct r_debug dl_r_debug;

static void dl_main (const Elf32_Phdr *phdr,
		     Elf32_Word phent,
		     Elf32_Addr *user_entry);

Elf32_Addr
_dl_start (void *arg)
{
  struct link_map rtld_map;

  /* Figure out the run-time load address of the dynamic linker itself.  */
  rtld_map.l_addr = elf_machine_load_address ();

  /* Read our own dynamic section and fill in the info array.
     Conveniently, the first element of the GOT contains the
     offset of _DYNAMIC relative to the run-time load address.  */
  rtld_map.l_ld = (void *) rtld_map.l_addr + *elf_machine_got ();
  elf_get_dynamic_info (rtld_map.l_ld, rtld_map.l_info);

#ifdef ELF_MACHINE_BEFORE_RTLD_RELOC
  ELF_MACHINE_BEFORE_RTLD_RELOC (rtld_map.l_info);
#endif

  /* Relocate ourselves so we can do normal function calls and
     data access using the global offset table.  */

  ELF_DYNAMIC_RELOCATE (&rtld_map, 0, NULL);


  /* Now life is sane; we can call functions and access global data.
     Set up to use the operating system facilities, and find out from
     the operating system's program loader where to find the program
     header table in core.  */

  dl_r_debug.r_ldbase = rtld_map.l_addr; /* Record our load address.  */

  /* Call the OS-dependent function to set up life so we can do things like
     file access.  It will call `dl_main' (below) to do all the real work
     of the dynamic linker, and then unwind our frame and run the user
     entry point on the same stack we entered on.  */
  return _dl_sysdep_start (&arg, &dl_main);
}


/* Now life is peachy; we can do all normal operations.
   On to the real work.  */

void _start (void);

static void
dl_main (const Elf32_Phdr *phdr,
	 Elf32_Word phent,
	 Elf32_Addr *user_entry)
{
  void doit (void)
    {
      const Elf32_Phdr *ph;
      struct link_map *l;
      const char *interpreter_name;
      int lazy;

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
Usage: ld.so EXECUTABLE-FILE [ARGS-FOR-PROGRAM...]\n\
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
of this helper program; chances are you did not intend to run this program.\n"
			      );

	  interpreter_name = _dl_argv[0];
	  --_dl_argc;
	  ++_dl_argv;
	  l = _dl_map_object (NULL, _dl_argv[0], user_entry);
	  phdr = l->l_phdr;
	  phent = l->l_phnum;
	  l->l_type = lt_executable;
	  l->l_libname = (char *) "";
	}
      else
	{
	  /* Create a link_map for the executable itself.
	     This will be what dlopen on "" returns.  */
	  l = _dl_new_object ((char *) "", "", lt_executable);
	  l->l_phdr = phdr;
	  l->l_phnum = phent;
	  interpreter_name = 0;
	}

      /* Scan the program header table for the dynamic section.  */
      for (ph = phdr; ph < &phdr[phent]; ++ph)
	switch (ph->p_type)
	  {
	  case PT_DYNAMIC:
	    /* This tells us where to find the dynamic section,
	       which tells us everything we need to do.  */
	    l->l_ld = (void *) ph->p_vaddr;
	    break;
	  case PT_INTERP:
	    /* This "interpreter segment" was used by the program loader to
	       find the program interpreter, which is this program itself, the
	       dynamic linker.  We note what name finds us, so that a future
	       dlopen call or DT_NEEDED entry, for something that wants to link
	       against the dynamic linker as a shared library, will know that
	       the shared object is already loaded.  */
	    interpreter_name = (void *) ph->p_vaddr;
	    break;
	  }
      assert (interpreter_name); /* How else did we get here?  */

      /* Extract the contents of the dynamic section for easy access.  */
      elf_get_dynamic_info (l->l_ld, l->l_info);
      /* Set up our cache of pointers into the hash table.  */
      _dl_setup_hash (l);

      if (l->l_info[DT_DEBUG])
	/* There is a DT_DEBUG entry in the dynamic section.  Fill it in
	   with the run-time address of the r_debug structure, which we
	   will set up later to communicate with the debugger.  */
	l->l_info[DT_DEBUG]->d_un.d_ptr = (Elf32_Addr) &dl_r_debug;

      l = _dl_new_object ((char *) interpreter_name, interpreter_name,
			  lt_interpreter);

      /* Now process all the DT_NEEDED entries and map in the objects.
	 Each new link_map will go on the end of the chain, so we will
	 come across it later in the loop to map in its dependencies.  */
      for (l = _dl_loaded; l; l = l->l_next)
	{
	  if (l->l_info[DT_NEEDED])
	    {
	      const char *strtab
		= (void *) l->l_addr + l->l_info[DT_STRTAB]->d_un.d_ptr;
	      const Elf32_Dyn *d;
	      for (d = l->l_ld; d->d_tag != DT_NULL; ++d)
		if (d->d_tag == DT_NEEDED)
		  _dl_map_object (l, strtab + d->d_un.d_val, NULL);
	    }
	  l->l_deps_loaded = 1;
	}

      l = _dl_loaded->l_next;
      assert (l->l_type == lt_interpreter);
      if (l->l_opencount == 0)
	{
	  /* No DT_NEEDED entry referred to the interpreter object itself.
	     Remove it from the maps we will use for symbol resolution.  */
	  l->l_prev->l_next = l->l_next;
	  if (l->l_next)
	    l->l_next->l_prev = l->l_prev;
	}

      lazy = _dl_secure || *(getenv ("LD_BIND_NOW") ?: "");

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
      dl_r_debug.r_map = _dl_loaded;
      dl_r_debug.r_brk = (Elf32_Addr) &_dl_r_debug_state;
    }
  const char *errstring;
  const char *errobj;
  int err;

  err = _dl_catch_error (&errstring, &errobj, &doit);
  if (errstring)
    _dl_sysdep_fatal (_dl_argv[0] ?: "<program name unknown>",
		      ": error in loading shared libraries\n",
		      errobj ?: "", errobj ? ": " : "",
		      errstring, err ? ": " : NULL,
		      err ? strerror (err) : NULL, NULL);

  /* Once we return, _dl_sysdep_start will invoke
     the DT_INIT functions and then *USER_ENTRY.  */
}

/* This function exists solely to have a breakpoint set on it by the 
   debugger.  */
void
_dl_r_debug_state (void)
{
}

#ifndef NDEBUG

/* Define (weakly) our own assert failure function which doesn't use stdio.
   If we are linked into the user program (-ldl), the normal __assert_fail
   defn can override this one.  */

#include "../stdio/_itoa.h"

void
__assert_fail (const char *assertion,
	       const char *file, unsigned int line, const char *function)
{
  char buf[64];
  buf[sizeof buf - 1] = '\0';
  _dl_sysdep_fatal ("BUG IN DYNAMIC LINKER ld.so: ",
		    file, ": ", _itoa (line, buf + sizeof buf - 1, 10, 0),
		    ": ", function ?: "", function ? ": " : "",
		    "Assertion `", assertion, "' failed!\n");

}
weak_symbol (__assert_fail)

#endif
