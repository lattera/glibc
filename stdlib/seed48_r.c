/* Copyright (C) 1995, 1997 Free Software Foundation, Inc.
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
#include <string.h>
#include <limits.h>

int
__seed48_r (seed16v, buffer)
     unsigned short int seed16v[3];
     struct drand48_data *buffer;
{
  /* Save old value at a private place to be used as return value.  */
  memcpy (buffer->old_X, buffer->X, sizeof (buffer->X));

  /* Install new state.  */
#if USHRT_MAX == 0xffffU
  buffer->X[2] = seed16v[2];
  buffer->X[1] = seed16v[1];
  buffer->X[0] = seed16v[0];

  buffer->a[2] = 0x5;
  buffer->a[1] = 0xdeec;
  buffer->a[0] = 0xe66d;
#else
  buffer->X[2] = (seed16v[2] << 16) | seed16v[1];
  buffer->X[1] = seed16v[0] << 16;
  buffer->X[0] = 0;

  buffer->a[2] = 0x5deecUL;
  buffer->a[1] = 0xe66d0000UL;
  buffer->a[0] = 0;
#endif
  buffer->c = 0xb;
  buffer->init = 1;

  return 0;
}
weak_alias (__seed48_r, seed48_r)
