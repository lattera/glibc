/* Copyright (C) 1991, 1993, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysdep.h>		/* In case it wants to define anything.  */

/* The first piece of initialized data.  */
int __data_start = 0;
#ifdef HAVE_WEAK_SYMBOLS
weak_alias (__data_start, data_start)
#endif

#ifdef	DUMMIES
#define	ARG_DUMMIES	DUMMIES,
#define	DECL_DUMMIES	int DUMMIES;
#else
#define	ARG_DUMMIES
#define	DECL_DUMMIES
#endif

#ifndef errno
volatile int errno;
#endif

extern void EXFUN(__libc_init, (int argc, char **argv, char **envp));
extern int EXFUN(main, (int argc, char **argv, char **envp));


/* Not a prototype because it gets called strangely.  */
static void start1();

#ifndef	HAVE__start

/* N.B.: It is important that this be the first function.
   This file is the first thing in the text section.  */
void
DEFUN_VOID(_start)
{
  start1();
}

#ifndef NO_UNDERSCORES
/* Make an alias called `start' (no leading underscore, so it can't
   conflict with C symbols) for `_start'.  This is the name vendor crt0.o's
   tend to use, and thus the name most linkers expect.  */
asm (".set start, __start");
#endif

#endif

/* ARGSUSED */
static void
start1 (ARG_DUMMIES argc, argp)
     DECL_DUMMIES
     int argc;
     char *argp;
{
  char **argv = &argp;

  /* The environment starts just after ARGV.  */
  __environ = &argv[argc + 1];

  /* If the first thing after ARGV is the arguments
     themselves, there is no environment.  */
  if ((char *) __environ == *argv)
    /* The environment is empty.  Make __environ
       point at ARGV[ARGC], which is NULL.  */
    --__environ;

  /* Do C library initializations.  */
  __libc_init (argc, argv, __environ);

  /* Call the user program.  */
  exit (main (argc, argv, __environ));
}
