/* Software floating-point emulation.
   Return sqrt(a)
   Copyright (C) 1997,1999 Free Software Foundation, Inc.
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

#include "soft-fp.h"
#include "quad.h"

long double __sqrttf2(long double a)
{
  FP_DECL_EX;
  FP_DECL_Q(A); FP_DECL_Q(R);
  long double r;

  FP_INIT_ROUNDMODE;
  FP_UNPACK_Q(A, a);
  FP_SQRT_Q(R, A);
  FP_PACK_Q(r, R);
  FP_HANDLE_EXCEPTIONS;

  return r;
}
