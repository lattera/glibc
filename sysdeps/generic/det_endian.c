/* Determine the "endianness" of the CPU.
   Copyright (C) 1991, 1992 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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

#include <stdio.h>

main ()
{
  unsigned long int i;

  if (sizeof (i) != 4)
    puts ("#error \"Not a 32-bit machine!\"");

  i = (((((('4' << 8) + '3') << 8) + '2') << 8) + '1');

  printf ("#define __BYTE_ORDER %.4s\n", (char *) &i);

  exit (0);
}
