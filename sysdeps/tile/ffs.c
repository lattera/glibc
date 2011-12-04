/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <limits.h>
#define ffsl __something_else
#include <string.h>

#undef ffs
int
__ffs (int x)
{
  return __builtin_ffs (x);
}
weak_alias (__ffs, ffs)
libc_hidden_builtin_def (ffs)

#undef ffsll
int
ffsll (long long x)
{
  return __builtin_ffsll (x);
}

#undef ffsl
#if ULONG_MAX == UINT_MAX
weak_alias (__ffs, ffsl)
#else
weak_alias (ffsll, ffsl)
#endif
