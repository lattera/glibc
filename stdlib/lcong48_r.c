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

int
__lcong48_r (param, buffer)
     unsigned short int param[7];
     struct drand48_data *buffer;
{
  /* Store the given values.  */
#if USHRT_MAX == 0xffffU
  memcpy (buffer->X, &param[0], sizeof (buffer->X));
  memcpy (buffer->a, &param[3], sizeof (buffer->a));
#else
  buffer->X[2] = (param[2] << 16) | param[1];
  buffer->X[1] = param[0] << 16;
  buffer->X[0] = 0;

  buffer->a[2] = (param[5] << 16) | param[4];
  buffer->a[1] = param[3] << 16;
  buffer->a[0] = 0;
#endif
  buffer->c = param[6];
  buffer->init = 1;

  return 0;
}
weak_alias (__lcong48_r, lcong48_r)
