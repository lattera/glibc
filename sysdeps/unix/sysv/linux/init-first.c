/* Initialization code run first thing by the ELF startup code.  Linux version.
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

#include <unistd.h>
#include <sysdep.h>
#include "fpu_control.h"

/* This code is mostly the same for all machines.  This version works at
   least for i386 and m68k, and probably any CISCy machine with a normal
   stack arrangement.  */

extern void __libc_init (int, char **, char **);
extern void __libc_global_ctors (void);


static void
init (int *data)
{
  int argc = *data;
  char **argv = (void *) (data + 1);
  char **envp = &argv[argc + 1];

#ifdef __i386__
  /* Make sure we are not using the iBSC2 personality.  The `personality'
     syscall takes one argument; zero means the Linux personality.  The
     argument arrives in %ebx; we have to save and restore %ebx by hand
     here, because GCC (as of 2.7.0) cannot handle saving and restoring it
     for us when it is the dedicated GOT register for PIC.  */
  asm ("pushl %%ebx\n"
       "xorl %%ebx, %%ebx\n"
       "int $0x80 # syscall no %0\n"
       "popl %%ebx"
       : : "a" (SYS_ify (personality)));
#endif

  /* Set the FPU control word to the proper default value.  */
  __setfpucw (__fpu_control);

  __environ = envp;
  __libc_init (argc, argv, envp);
}

#ifdef PIC
/* This function is called to initialize the shared C library.
   It is called just before the user _start code from i386/elf/start.S,
   with the stack set up as that code gets it.  */

/* NOTE!  The linker notices the magical name `_init' and sets the DT_INIT
   pointer in the dynamic section based solely on that.  It is convention
   for this function to be in the `.init' section, but the symbol name is
   the only thing that really matters!!  */
/*void _init (int argc, ...) __attribute__ ((unused, section (".init")));*/

void
_init (int argc, ...)
{
  init (&argc);

  __libc_global_ctors ();
}
#endif


void
__libc_init_first (int argc __attribute__ ((unused)), ...)
{
#ifndef PIC
  init (&argc);
#endif
}
