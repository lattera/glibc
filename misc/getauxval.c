/* Copyright (C) 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/auxv.h>
#include <ldsodefs.h>


unsigned long int
__getauxval (unsigned long int type)
{
  ElfW(auxv_t) *p;

  if (type == AT_HWCAP)
    return GLRO(dl_hwcap);

  for (p = GLRO(dl_auxv); p->a_type != AT_NULL; p++)
    if (p->a_type == type)
      return p->a_un.a_val;
  return 0;
}

weak_alias (__getauxval, getauxval)
