/* Initial program startup for running under the GNU Hurd.
Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hurd.h>
#include <hurd/exec.h>
#include <sysdep.h>
#include <hurd/threadvar.h>
#include <unistd.h>
#include <elf.h>
#include "set-hooks.h"
#include "hurdmalloc.h"		/* XXX */
#include "hurdstartup.h"

mach_port_t *_hurd_init_dtable;
mach_msg_type_number_t _hurd_init_dtablesize;

unsigned int __hurd_threadvar_max;
unsigned long int __hurd_threadvar_stack_mask;
unsigned long int __hurd_threadvar_stack_offset;

/* These are set up by _hurdsig_init.  */
unsigned long int __hurd_sigthread_stack_base;
unsigned long int __hurd_sigthread_stack_end;
unsigned long int *__hurd_sigthread_variables;

extern void __mach_init (void);

int _hurd_split_args (char *, size_t, char **);


/* Entry point.  This is the first thing in the text segment.

   The exec server started the initial thread in our task with this spot the
   PC, and a stack that is presumably big enough.  We do basic Mach
   initialization so mig-generated stubs work, and then do an exec_startup
   RPC on our bootstrap port, to which the exec server responds with the
   information passed in the exec call, as well as our original bootstrap
   port, and the base address and size of the preallocated stack.

   If using cthreads, we are given a new stack by cthreads initialization and
   deallocate the stack set up by the exec server.  On the new stack we call
   `start1' (above) to do the rest of the startup work.  Since the stack may
   disappear out from under us in a machine-dependent way, we use a pile of
   static variables to communicate the information from exec_startup to start1.
   This is unfortunate but preferable to machine-dependent frobnication to copy
   the state from the old stack to the new one.  */


void
_hurd_startup (void **argptr, void (*main) (int *data))
{
  error_t err;
  mach_port_t in_bootstrap;
  char *args, *env;
  mach_msg_type_number_t argslen, envlen;
  struct hurd_startup_data data;
  char **argv, **envp;
  int argc, envc;
  int *argcptr;

  if (err = __task_get_special_port (__mach_task_self (), TASK_BOOTSTRAP_PORT,
				     &in_bootstrap))
    LOSE;

  if (in_bootstrap != MACH_PORT_NULL)
    {
      /* Call the exec server on our bootstrap port and
	 get all our standard information from it.  */

      argslen = envlen = 0;
      data.dtablesize = data.portarraysize = data.intarraysize = 0;

      err = __exec_startup (in_bootstrap,
			    &data.stack_base, &data.stack_size,
			    &data.flags, &args, &argslen, &env, &envlen,
			    &data.dtable, &data.dtablesize,
			    &data.portarray, &data.portarraysize,
			    &data.intarray, &data.intarraysize);
      __mach_port_deallocate (__mach_task_self (), in_bootstrap);
    }

  if (err || in_bootstrap == MACH_PORT_NULL)
    {
      /* Either we have no bootstrap port, or the RPC to the exec server
	 failed.  Try to snarf the args in the canonical Mach way.
	 Hopefully either they will be on the stack as expected, or the
	 stack will be zeros so we don't crash.  Set all our other
	 variables to have empty information.  */

      argcptr = (int *) argptr;
      argc = argcptr[0];
      argv = (char **) &argcptr[1];
      envp = &argv[argc + 1];
      envc = 0;
      while (envp[envc])
	++envc;

      data.flags = 0;
      args = env = NULL;
      argslen = envlen = 0;
      data.dtable = NULL;
      data.dtablesize = 0;
      data.portarray = NULL;
      data.portarraysize = 0;
      data.intarray = NULL;
      data.intarraysize = 0;
    }
  else
    argv = envp = NULL;


  /* Turn the block of null-separated strings we were passed for the
     arguments and environment into vectors of pointers to strings.  */

  if (! argv)
    {
      /* Count up the arguments so we can allocate ARGV.  */
      argc = _hurd_split_args (args, argslen, NULL);
      /* Count up the environment variables so we can allocate ENVP.  */
      envc = _hurd_split_args (env, envlen, NULL);

      /* There were some arguments.  Allocate space for the vectors of
	 pointers and fill them in.  We allocate the space for the
	 environment pointers immediately after the argv pointers because
	 the ELF ABI will expect it.  */
      argcptr = __alloca (sizeof (int) +
			  (argc + 1 + envc + 1) * sizeof (char *) +
			  sizeof (struct hurd_startup_data));
      *argcptr = argc;
      argv = (void *) (argcptr + 1);
      _hurd_split_args (args, argslen, argv);

      /* There was some environment.  */
      envp = &argv[argc + 1];
      _hurd_split_args (env, envlen, envp);
    }

  {
    struct hurd_startup_data *d = (void *) &envp[envc + 1];

    if ((void *) d != argv[0])
      {
	*d = data;
	_hurd_init_dtable = d->dtable;
	_hurd_init_dtablesize = d->dtablesize;

    /* XXX hardcoded kludge until exec_startup changes */
	{
	  extern void _start();
	  vm_address_t page = 0;
	  vm_size_t size = 0;
	  if (__vm_read (__mach_task_self (),
			 0x08000000, __vm_page_size, &page, &size) == 0)
	    {
	      const Elf32_Ehdr *ehdr = (const void *) 0x08000000;
	      d->phdr = 0x08000000 + ehdr->e_phoff;
	      d->phdrsz = ehdr->e_phnum * ehdr->e_phentsize;
	      d->user_entry = ehdr->e_entry;
	      __vm_deallocate (__mach_task_self (), page, size);
	    }
	  else
	    d->user_entry = (Elf32_Addr) &_start;
	}
      }

    (*main) (argcptr);
  }

  /* Should never get here.  */
  LOSE;
  abort ();
}

/* Split ARGSLEN bytes at ARGS into words, breaking at NUL characters.  If
   ARGV is not a null pointer, store a pointer to the start of each word in
   ARGV[n], and null-terminate ARGV.  Return the number of words split.  */

int
_hurd_split_args (char *args, size_t argslen, char **argv)
{
  char *p = args;
  size_t n = argslen;
  int argc = 0;

  while (n > 0)
    {
      char *end = memchr (p, '\0', n);

      if (argv)
	argv[argc] = p;
      ++argc;

      if (end == NULL)
	/* The last argument is unterminated.  */
	break;

      n -= end + 1 - p;
      p = end + 1;
    }

  if (argv)
    argv[argc] = NULL;
  return argc;
}
