/* Copyright (C) 1994, 1995, 1997, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <alloca.h>

extern int __sco_getgroups (int size, unsigned short int *list);

int
__getgroups (size, list)
     int size; gid_t *list;
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

weak_alias (__getgroups, getgroups)
