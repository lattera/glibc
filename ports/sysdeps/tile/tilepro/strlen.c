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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <stdint.h>

size_t
strlen (const char *s)
{
  /* Get an aligned pointer. */
  const uintptr_t s_int = (uintptr_t) s;
  const uint32_t *p = (const uint32_t *) (s_int & -4);

  /* Read the first word, but force bytes before the string to be nonzero.
     This expression works because we know shift counts are taken mod 32.  */
  uint32_t v = *p | ((1 << (s_int << 3)) - 1);

  uint32_t bits;
  while ((bits = __insn_seqb (v, 0)) == 0)
    v = *++p;

  return ((const char *) p) + (__insn_ctz (bits) >> 3) - s;
}
libc_hidden_builtin_def (strlen)
