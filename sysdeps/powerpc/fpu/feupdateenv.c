/* Install given floating-point environment and raise exceptions.
   Copyright (C) 1997, 1999, 2000, 2001, 2007, 2008, 2010
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include <bp-sym.h>

#define _FPU_MASK_ALL (_FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_UM | _FPU_MASK_XM | _FPU_MASK_IM)

int
__feupdateenv (const fenv_t *envp)
{
  fenv_union_t old, new;

  /* Save the currently set exceptions.  */
  new.fenv = *envp;
  old.fenv = fegetenv_register ();

  /* Restore rounding mode and exception enable from *envp and merge
     exceptions.  Leave fraction rounded/inexact and FP result/CC bits
     unchanged.  */
  new.l[1] = (old.l[1] & 0x1FFFFF00) | (new.l[1] & 0x1FF80FFF);
  
  /* If the old env has no eabled exceptions and the new env has any enabled
     exceptions, then unmask SIGFPE in the MSR FE0/FE1 bits.  This will put
     the hardware into "precise mode" and may cause the FPU to run slower on
     some hardware.  */
  if ((old.l[1] & _FPU_MASK_ALL) == 0 && (new.l[1] & _FPU_MASK_ALL) != 0)
    (void)__fe_nomask_env ();
  
  /* If the old env had any eabled exceptions and the new env has no enabled
     exceptions, then mask SIGFPE in the MSR FE0/FE1 bits.  This may allow the
     FPU to run faster because it always takes the default action and can not 
     generate SIGFPE. */
  if ((old.l[1] & _FPU_MASK_ALL) != 0 && (new.l[1] & _FPU_MASK_ALL) == 0)
    (void)__fe_mask_env ();

  /* Atomically enable and raise (if appropriate) exceptions set in `new'. */
  fesetenv_register (new.fenv);

  /* Success.  */
  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feupdateenv, __old_feupdateenv)
compat_symbol (libm, BP_SYM (__old_feupdateenv), BP_SYM (feupdateenv), GLIBC_2_1);
#endif

libm_hidden_ver (__feupdateenv, feupdateenv)
versioned_symbol (libm, BP_SYM (__feupdateenv), BP_SYM (feupdateenv), GLIBC_2_2);
