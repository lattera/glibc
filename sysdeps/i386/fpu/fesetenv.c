/* Install given floating-point environment.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#include <assert.h>
#include <bp-sym.h>


int
__fesetenv (const fenv_t *envp)
{
  fenv_t temp;

  /* The memory block used by fstenv/fldenv has a size of 28 bytes.  */
  assert (sizeof (fenv_t) == 28);

  /* Install the environment specified by ENVP.  But there are a few
     values which we do not want to come from the saved environment.
     Therefore, we get the current environment and replace the values
     we want to use from the environment specified by the parameter.  */
  __asm__ ("fnstenv %0" : "=m" (*&temp));

  if (envp == FE_DFL_ENV)
    {
      temp.__control_word |= FE_ALL_EXCEPT;
      temp.__control_word &= ~FE_TOWARDZERO;
      temp.__status_word &= ~FE_ALL_EXCEPT;
      temp.__eip = 0;
      temp.__cs_selector = 0;
      temp.__opcode = 0;
      temp.__data_offset = 0;
      temp.__data_selector = 0;
    }
  else if (envp == FE_NOMASK_ENV)
    {
      temp.__control_word &= ~(FE_ALL_EXCEPT | FE_TOWARDZERO);
      temp.__status_word &= ~FE_ALL_EXCEPT;
      temp.__eip = 0;
      temp.__cs_selector = 0;
      temp.__opcode = 0;
      temp.__data_offset = 0;
      temp.__data_selector = 0;
    }
  else
    {
      temp.__control_word &= ~(FE_ALL_EXCEPT | FE_TOWARDZERO);
      temp.__control_word |= (envp->__control_word
			      & (FE_ALL_EXCEPT | FE_TOWARDZERO));
      temp.__status_word &= ~FE_ALL_EXCEPT;
      temp.__status_word |= envp->__status_word & FE_ALL_EXCEPT;
      temp.__eip = envp->__eip;
      temp.__cs_selector = envp->__cs_selector;
      temp.__opcode = envp->__opcode;
      temp.__data_offset = envp->__data_offset;
      temp.__data_selector = envp->__data_selector;
    }

  __asm__ ("fldenv %0" : : "m" (temp));

  /* Success.  */
  return 0;
}
strong_alias (__fesetenv, __old_fesetenv)
symbol_version (BP_SYM (__old_fesetenv), BP_SYM (fesetenv), GLIBC_2.1);
default_symbol_version (BP_SYM (__fesetenv), BP_SYM (fesetenv), GLIBC_2.2);
