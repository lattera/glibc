/* Software floating-point emulation: float to integer conversion.
   Copyright (C) 1997,1999,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@cygnus.com) and
		  Jakub Jelinek (jj@ultra.linux.cz).

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

#include "local-soft-fp.h"

long
_OtsCvtXQ (long al, long ah, long _round)
{
  FP_DECL_EX;
  FP_DECL_Q(A);
  long r, s;

  /* If bit 3 is set, then integer overflow detection is requested.  */
  s = _round & 8 ? 1 : -1;
  _round = _round & 3;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_Q(A, a);
  FP_TO_INT_Q(r, A, 64, s);

  if (s > 0 && (_fex &= FP_EX_INVALID))
    FP_HANDLE_EXCEPTIONS;

  return r;
}
