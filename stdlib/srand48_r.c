/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

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
#include <limits.h>

int
__srand48_r (seedval, buffer)
     long int seedval;
     struct drand48_data *buffer;
{
  /* The standards say we only have 32 bits.  */
  if (sizeof (long int) > 4)
    seedval &= 0xffffffffl;

#if USHRT_MAX == 0xffffU
  buffer->X[2] = seedval >> 16;
  buffer->X[1] = seedval & 0xffffl;
  buffer->X[0] = 0x330e;

  buffer->a[2] = 0x5;
  buffer->a[1] = 0xdeec;
  buffer->a[0] = 0xe66d;
#else
  buffer->X[2] = seedval;
  buffer->X[1] = 0x330e0000UL;
  buffer->X[0] = 0;

  buffer->a[2] = 0x5deecUL;
  buffer->a[1] = 0xe66d0000UL;
  buffer->a[0] = 0;
#endif
  buffer->c = 0xb;
  buffer->init = 1;

  return 0;
}
weak_alias (__srand48_r, srand48_r)
