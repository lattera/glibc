/* Store current floating-point environment and clear exceptions.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv.h>
#include <fpu_control.h>
#include <arm-features.h>


int
feholdexcept (fenv_t *envp)
{
  if (ARM_HAVE_VFP)
    {
      unsigned long int temp;

      /* Store the environment.  */
      _FPU_GETCW(temp);
      envp->__cw = temp;

      /* Now set all exceptions to non-stop.  */
      temp &= ~(FE_ALL_EXCEPT << FE_EXCEPT_SHIFT);

      /* And clear all exception flags.  */
      temp &= ~FE_ALL_EXCEPT;

      _FPU_SETCW(temp);

      return 0;
    }

  /* Unsupported, so fail.  */
  return 1;
}

libm_hidden_def (feholdexcept)
