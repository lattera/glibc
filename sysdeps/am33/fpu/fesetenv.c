/* Install given floating-point environment.
   Copyright (C) 1998, 1999, 2000, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   based on corresponding file in the MIPS port.

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

#include <fenv.h>
#include <fpu_control.h>
#include <shlib-compat.h>

int
__fesetenv (const fenv_t *envp)
{
  fpu_control_t cw;

  /* We want to clear all EF bits for the default end IEEE.  */

  if (envp == FE_DFL_ENV)
    _FPU_SETFCW (_FPU_DEFAULT|FE_ALL_EXCEPT);
  else if (envp == FE_NOMASK_ENV)
    _FPU_SETFCW (_FPU_IEEE|FE_ALL_EXCEPT);
  else
    {
      fpu_control_t temp;

      _FPU_GETCW (temp);
      cw = *envp;

      /* If EF bits are cleared and the user requests them to be set,
	 we have to fail, because there's no way to do it.  */
      if (~temp & cw & FE_ALL_EXCEPT)
	return -1;

      /* We clear EF bits by storing a 1 in them, so flip the
	 FE_ALL_EXCEPT bits.  */
      cw = (cw & ~FE_ALL_EXCEPT) | (~cw & FE_ALL_EXCEPT);
      _FPU_SETFCW (cw);
    }

  /* Success.  */
  return 0;
}

libm_hidden_ver (__fesetenv, fesetenv)
versioned_symbol (libm, __fesetenv, fesetenv, GLIBC_2_2);
