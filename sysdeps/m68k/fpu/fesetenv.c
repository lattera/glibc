/* Install given floating-point environment.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fenv.h>

void
fesetenv (const fenv_t *envp)
{
  fenv_t temp;

  /* Install the environment specified by ENVP.  But there are a few
     values which we do not want to come from the saved environment.
     Therefore, we get the current environment and replace the values
     we want to use from the environment specified by the parameter.  */
  __asm__ ("fmovem%.l %/fpcr/%/fpsr,%0" : "=m" (*&temp));

  temp.status_register &= ~FE_ALL_EXCEPT;
  temp.control_register &= ~((FE_ALL_EXCEPT << 5) | FE_UPWARD);
  if (envp == FE_DFL_ENV)
    ;
  else if (envp == FE_NOMASK_ENV)
    temp.control_register |= FE_ALL_EXCEPT << 5;
  else
    {
      temp.control_register |= (envp->control_register
				& ((FE_ALL_EXCEPT << 5) | FE_UPWARD));
      temp.status_register |= envp->status_register & FE_ALL_EXCEPT;
    }

  __asm__ __volatile__ ("fmovem%.l %0,%/fpcr/%/fpsr" : : "m" (temp));
}
