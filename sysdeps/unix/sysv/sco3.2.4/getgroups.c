/* Copyright (C) 1994 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <alloca.h>

extern int __sco_getgroups __P ((int size, unsigned short int *list));

int
DEFUN(__getgroups, (size, list), int size AND gid_t *list)
{
  int i;
  unsigned short int *shortlist;

  if (size <= 0)
    return __sco_getgroups (size, NULL);

  shortlist = __alloca (size * sizeof (*shortlist));

  size = __sco_getgroups (size, shortlist);
  for (i = 0; i < size; ++i)
    list[i] = shortlist[i];

  return size;
}
