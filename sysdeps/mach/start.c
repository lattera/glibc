/* Copyright (C) 1992, 1993, 1994, 1995, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sysdep.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* The first piece of initialized data.  */
int __data_start = 0;

#ifndef _HURD_THREADVAR_H
volatile int errno;
#endif

extern void __mach_init (void);
extern void __libc_init (int argc, char **argv, char **envp);
extern int main (int argc, char **argv, char **envp);

/* These are uninitialized common definitions so they will be zero
   by default.  If the user links in C threads, that will provide initialized
   definitions that override these.  */
void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
void (*_cthread_exit_routine) (int status);


/* These are for communication from _start to start1,
   where we cannot use the stack for anything.  */
static int start_argc;
static char **start_argv;

/* _start calls this on the new stack.  */
static volatile void
start1 (void)
{
  __libc_init (start_argc, start_argv, __environ);

  (_cthread_exit_routine != NULL ? *_cthread_exit_routine : exit)
    (main (start_argc, start_argv, __environ));

  /* Should never get here.  */
  LOSE;
}

#ifndef START_ARGS
#define START_ARGS void
#endif
#ifdef START_MACHDEP
START_MACHDEP
#define _start _start0
#endif

void
_start (START_ARGS)
{
  SNARF_ARGS (start_argc, start_argv, __environ);

  __mach_init ();

  if (_cthread_init_routine != NULL)
    CALL_WITH_SP (start1, (*_cthread_init_routine) ());
  else
    start1 ();

  /* Should never get here.  */
  LOSE;
}
