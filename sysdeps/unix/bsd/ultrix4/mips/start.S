/* Copyright (C) 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <sysdep.h>

ENTRY(__start)
  .set noreorder

  /* The first thing on the stack is argc.  */
  lw s0, 0(sp)
  nop

  /* Set up the global pointer.  */
  la gp, _gp

  /* Then set up argv.  */
  addiu s1, sp, 4

  /* To compute where envp is, first we have to jump ahead four
     bytes from what argv was.  This will bring us ahead, so we don't
     need to compute the NULL at the end of argv later.  */
  addiu v1, s1, 4

  /* Now, compute the space to skip given the number of arguments
     we've got.  We do this by multiplying argc by 4.  */
  sll v0, s0, 2

  /* Now, add (argv+4) with the space to skip...that's envp.  */
  addu s2, v1, v0

  /* __environ = envp; */
  sw s2, __environ

  addiu sp, sp, -24

  /* __libc_init (argc, argv, envp); */
  move a0, s0
  move a1, s1
  jal __libc_init
  move a2, s2

  /* errno = 0; */
  sw zero, errno

  /* exit (main (argc, argv, envp)); */
  move a0, s0
  move a1, s1
  jal main
  move a2, s2

  /* Make the value returned by main be the argument to exit.  */
  jal exit
  move a0, v0
  .end __start
