/* Copyright (C) 1995, 1997, 1998, 2001 Free Software Foundation, Inc.
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

int
__jrand48_r (xsubi, buffer, result)
     unsigned short int xsubi[3];
     struct drand48_data *buffer;
     long int *result;
{
  /* Compute next state.  */
  if (__drand48_iterate (xsubi, buffer) < 0)
    return -1;

  /* Store the result.  */
  if (sizeof (unsigned short int) == 2)
    *result = ((xsubi[2] << 16) | xsubi[1]) & 0xffffffffl;
  else
    *result = xsubi[2] & 0xffffffffl;

  return 0;
}
weak_alias (__jrand48_r, jrand48_r)
