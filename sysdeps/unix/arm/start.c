/* Special startup code for ARM a.out binaries.
   Copyright (C) 1998, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysdep.h>

/* The first piece of initialized data.  */
int __data_start = 0;
#ifdef HAVE_WEAK_SYMBOLS
weak_alias (__data_start, data_start)
#endif

extern void __libc_init (int argc, char **argv, char **envp);
extern int main (int argc, char **argv, char **envp);

/* N.B.: It is important that this be the first function.
   This file is the first thing in the text section.  */

/* If this was in C it might create its own stack frame and
   screw up the arguments.  */
#ifdef NO_UNDERSCORES
asm (".text; .globl _start; _start: B start1");
#else
asm (".text; .globl __start; __start: B _start1");

/* Make an alias called `start' (no leading underscore, so it can't
   conflict with C symbols) for `_start'.  This is the name vendor crt0.o's
   tend to use, and thus the name most linkers expect.  */
asm (".set start, __start");
#endif

/* Fool gcc into thinking that more args are passed.  This makes it look
   on the stack (correctly) for the real arguments.  It causes somewhat
   strange register usage in start1(), but we aren't too bothered about
   that at the moment. */
#define DUMMIES a1, a2, a3, a4

#ifdef	DUMMIES
#define	ARG_DUMMIES	DUMMIES,
#define	DECL_DUMMIES	int DUMMIES;
#else
#define	ARG_DUMMIES
#define	DECL_DUMMIES
#endif

/* ARGSUSED */
static void
start1 (ARG_DUMMIES argc, argv, envp)
     DECL_DUMMIES
     int argc;
     char **argv;
     char **envp;
{
  /* Store a pointer to the environment.  */
  __environ = envp;

  /* Do C library initializations.  */
  __libc_init (argc, argv, __environ);

  /* Call the user program.  */
  exit (main (argc, argv, __environ));
}
