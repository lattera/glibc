/* Test exception in saved exception state.  S/390 version.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <fenv_libc.h>

int
fetestexceptflag (const fexcept_t *flagp, int excepts)
{
  /* As *flagp is obtained by an earlier call of fegetexceptflag the
     bits 0-5 of dxc-byte are either zero or correspond to the
     flag-bits.  Evaluate flags and last dxc-exception-code.  */
  return (((*flagp >> FPC_FLAGS_SHIFT) | (*flagp >> FPC_DXC_SHIFT))
	  & excepts
	  & FE_ALL_EXCEPT);
}
