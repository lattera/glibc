/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <ansidecl.h>

/* Defined in brk.c.  */
extern PTR __curbrk;
extern int EXFUN(__brk, (PTR addr));

/* Extend the process's data space by INCREMENT.
   If INCREMENT is negative, shrink data space by - INCREMENT.
   Return start of new space allocated, or -1 for errors.  */
PTR
DEFUN(__sbrk, (increment), int increment)
{
  char *oldbrk;

  if (increment == 0)
    return __curbrk;

  oldbrk = __curbrk;
  if (__brk(oldbrk + increment) < 0)
    return (PTR) -1;

  return oldbrk;
}

weak_alias (__sbrk, sbrk)
