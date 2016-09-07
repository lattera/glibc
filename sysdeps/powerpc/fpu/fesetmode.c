/* Install given floating-point control modes.  PowerPC version.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <fenv_libc.h>
#include <fpu_control.h>

#define _FPU_MASK_ALL (_FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_UM	\
		       | _FPU_MASK_XM | _FPU_MASK_IM)

#define FPU_STATUS 0xbffff700ULL

int
fesetmode (const femode_t *modep)
{
  fenv_union_t old, new;

  /* Logic regarding enabled exceptions as in fesetenv.  */

  new.fenv = *modep;
  old.fenv = fegetenv_register ();
  new.l = (new.l & ~FPU_STATUS) | (old.l & FPU_STATUS);

  if (old.l == new.l)
    return 0;

  if ((old.l & _FPU_MASK_ALL) == 0 && (new.l & _FPU_MASK_ALL) != 0)
    (void) __fe_nomask_env_priv ();

  if ((old.l & _FPU_MASK_ALL) != 0 && (new.l & _FPU_MASK_ALL) == 0)
    (void) __fe_mask_env ();

  fesetenv_register (new.fenv);
  return 0;
}
