/* Store current floating-point environment and clear exceptions.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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
#define _FPU_MASK_ALL (_FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_UM | _FPU_MASK_XM | _FPU_MASK_IM)

int
__feholdexcept (fenv_t *envp)
{
  fenv_union_t old, new;

  /* Save the currently set exceptions.  */
  old.fenv = *envp = fegetenv_register ();

  /* Clear everything except for the rounding modes and non-IEEE arithmetic
     flag.  */
  new.l = old.l & 0xffffffff00000007LL;

  if (new.l == old.l)
    return 0;

  /* If the old env had any enabled exceptions, then mask SIGFPE in the
     MSR FE0/FE1 bits.  This may allow the FPU to run faster because it
     always takes the default action and can not generate SIGFPE. */
  if ((old.l & _FPU_MASK_ALL) != 0)
    (void)__fe_mask_env ();

  /* Put the new state in effect.  */
  fesetenv_register (new.fenv);

  return 0;
}
libm_hidden_def (__feholdexcept)
weak_alias (__feholdexcept, feholdexcept)
libm_hidden_weak (feholdexcept)
