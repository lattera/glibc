/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <unistd.h>
#include <elf/ldsodefs.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

extern int _dl_starting_up;
weak_extern (_dl_starting_up)
extern int __libc_multiple_libcs;
extern void *__libc_stack_end;

int
__libc_start_main (int (*main) (int, char **, char **), int argc,
		   char **argv, void (*init) (void), void (*fini) (void),
		   void (*rtld_fini) (void), void *stack_end)
{
#ifndef PIC
  /* The next variable is only here to work around a bug in gcc <= 2.7.2.2.
     If the address would be taken inside the expression the optimizer
     would try to be too smart and throws it away.  Grrr.  */
  int *dummy_addr = &_dl_starting_up;

  __libc_multiple_libcs = dummy_addr && !_dl_starting_up;

  /* Store the lowest stack address.  */
  __libc_stack_end = stack_end;
#endif

  /* Set the global _environ variable correctly.  */
  __environ = &argv[argc + 1];

  /* Register the destructor of the dynamic linker if there is any.  */
  if (rtld_fini != NULL)
    atexit (rtld_fini);

  /* Call the initializer of the libc.  */
#ifdef PIC
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ninitialize libc\n\n", NULL);
#endif
  __libc_init_first (argc, argv, __environ);

  /* Register the destructor of the program, if any.  */
  if (fini)
    atexit (fini);

  /* Call the initializer of the program, if any.  */
#ifdef PIC
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ninitialize program: ", argv[0], "\n\n", NULL);
#endif
  if (init)
    (*init) ();

#ifdef PIC
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ntransferring control: ", argv[0], "\n\n", NULL);
#endif

  exit ((*main) (argc, argv, __environ));
}
