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

int
__libc_start_main (int (*main) (int, char **, char **), int argc,
		   char **argv, void (*init) (void), void (*fini) (void),
		   void (*rtld_fini) (void))
{
  /* Register the destructor of the dynamic linker if there is any.  */
  if (rtld_fini != NULL)
    atexit (rtld_fini);

  /* Call the initializer of the libc.  */
  __libc_init_first ();

  /* Set the global _environ variable correctly.  */
  __environ = &argv[argc + 1];

  /* Call the initializer of the program.  */
  (*init) ();

  /* Register the destructor of the program.  */
  atexit (fini);

  exit ((*main) (argc, argv, envp));
}
