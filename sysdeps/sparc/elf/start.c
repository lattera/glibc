/* Copyright (C) 1991, 1992, 1993, 1994, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

extern char **__environ;

extern void __libc_init_first __P ((int argc, char **argv, char **envp));
extern int main __P ((int argc, char **argv, char **envp));

register long int sp asm("%sp"), fp asm("%fp");

void
_start (void)
{
  /* It is important that these be declared `register'.
     Otherwise, when compiled without optimization, they are put on the
     stack, which loses completely after we zero the FP.  */
  register int argc;
  register char **argv, **envp;
  register long int g1 asm ("%g1");
  unsigned long int copy_g1 = g1;

  /* Unwind the frame built when we entered the function.  */
  asm("restore");
  if (copy_g1)
    atexit (copy_g1);

  /* And clear the frame pointer.  */
  fp = 0;

  /* The argument info starts after one register
     window (64 bytes) past the SP.  */
  argc = ((int *) sp)[16];
  argv = (char **) &((int *) sp)[17];
  envp = &argv[argc + 1];
  __environ = envp;

  /* Allocate 24 bytes of stack space for the register save area.  */
  sp -= 24;
  __libc_init_first (argc, argv, envp);
#ifdef ELF_INIT_FINI
  {
    extern void _fini (void);
    _init ();
    atexit (_fini);
  }
#endif
  exit (main (argc, argv, envp));
}
