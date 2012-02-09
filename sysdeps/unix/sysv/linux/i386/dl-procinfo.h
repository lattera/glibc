/* Linux/i386 version of processor capability information handling macros.
   Copyright (C) 1998-2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <sysdeps/i386/dl-procinfo.h>
#include <ldsodefs.h>


#undef _dl_procinfo
static inline int
__attribute__ ((unused))
_dl_procinfo (int word)
{
  /* This table should match the information from arch/i386/kernel/setup.c
     in the kernel sources.  */
  int i;

  _dl_printf ("AT_HWCAP:   ");

  for (i = 0; i < _DL_HWCAP_COUNT; ++i)
    if (word & (1 << i))
      _dl_printf (" %s", GLRO(dl_x86_cap_flags)[i]);

  _dl_printf ("\n");

  return 0;
}
