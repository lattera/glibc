/* Startup code compliant to the ELF PowerPC ABI.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

/* This is SVR4/PPC ABI compliant, and works under Linux when
   statically linked.  */

#include <unistd.h>
#include <stdlib.h>

/* Just a little assembler stub before gcc gets its hands on our
   stack pointer... */
asm ("\
	.text
	.globl _start
_start:
 # save the stack pointer, in case we're statically linked under Linux
        mr 8,1
 # set up an initial stack frame, and clear the LR
        addi 1,1,-16
        clrrwi 1,1,4
        li 0,0
        stw 0,0(1)
        mtlr 0
 # set r13 to point at the 'small data area'
        lis 13,_SDA_BASE_@ha
        addi 13,13,_SDA_BASE_@l
 # and continue below.
        b __start1
");

/* Define a symbol for the first piece of initialized data.  */
int __data_start = 0;
weak_alias (__data_start, data_start)

/* these probably should go, at least go somewhere else
   (sysdeps/mach/something?). */
void (*_mach_init_routine) (void);
void (*_thread_init_routine) (void);

void __libc_init_first (int argc, char **argv, char **envp);
int main (int argc, char **argv, char **envp, void *auxvec);
#ifdef HAVE_INITFINI
void _init (void);
void _fini (void);
#endif


static void __start1(int argc, char **argv, char **envp,
		     void *auxvec, void (*exitfn) (void), char **arguments)
     __attribute__ ((unused));
static void
__start1(int argc, char **argv, char **envp,
	 void *auxvec, void (*exitfn) (void),
	 char **arguments)
{
  /* the PPC SVR4 ABI says that the top thing on the stack will
     be a NULL pointer, so if not we assume that we're being called
     as a statically-linked program by Linux. */
  int abi_compliant_startup = *arguments == NULL;

  if (!abi_compliant_startup)
  {
    argc = *(int *) arguments;
    argv = arguments+1;
    envp = argv+argc+1;
    auxvec = envp;
    while (auxvec != NULL)
      auxvec++;
    auxvec++;
    exitfn = NULL;
  }

  if (exitfn != NULL)
    atexit (exitfn);

  /* libc init routine, in case we are statically linked
     (otherwise ld.so will have called it when it loaded libc, but
     calling it twice doesn't hurt). */
  __libc_init_first (argc, argv, envp);

#ifdef HAVE_INITFINI
  /* ELF constructors/destructors */
  atexit (_fini);
  _init ();
#endif

  /* Stuff so we can build Mach/Linux executables (like vmlinux).  */
  if (_mach_init_routine != 0)
    _mach_init_routine ();
  if (_thread_init_routine != 0)
    _thread_init_routine ();

  /* the rest of the program */
  exit (main (argc, argv, envp, auxvec));
}
