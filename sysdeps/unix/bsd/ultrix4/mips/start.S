/* Copyright (C) 1993 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <sysdep.h>

#ifndef HAVE_GNU_LD
#define __environ environ
#endif

.comm __environ,	4
.comm errno,		4

ENTRY(__start)
  .set noreorder

  /* The first thing on the stack is argc.  */
  lw t0, 0(sp)
  nop

  /* Set up the global pointer.  */
  la gp, _gp

  /* Then set up argv.  */
  addiu t1, sp, 4

  /* To compute where envp is, first we have to jump ahead four
     bytes from what argv was.  This will bring us ahead, so we don't
     need to compute the NULL at the end of argv later.  */
  addiu v1, t1, 4

  /* Now, compute the space to skip given the number of arguments
     we've got.  We do this by multiplying argc by 4.  */
  sll v0, t0, 2

  /* Now, add (argv+4) with the space to skip...that's envp.  */
  addu v1, v1, v0
  move t2, v1

  /* __environ = envp; */
  sw t2, __environ

  addiu sp, sp, -24

  /* __libc_init (argc, argv, envp); */
  move a0, t0
  move a1, t1
  move a2, t2
  jal __libc_init
  nop

  /* errno = 0; */
  sw zero, errno

  /* exit (main (argc, argv, envp)); */
  move a0, t0
  move a1, t1
  move a2, t2
  jal main
  nop

  /* Make the value returned by main be the argument to exit.  */
  jal exit
  move a0, v0
