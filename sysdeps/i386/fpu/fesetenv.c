/* Install given floating-point environment.
   Copyright (C) 1997 Free Software Foundation, Inc.
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


void
fesetenv (const fenv_t *envp)
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
      temp.control_word |= FE_ALL_EXCEPT;
      temp.control_word &= ~FE_TOWARDSZERO;
      temp.status_word &= ~FE_ALL_EXCEPT;
      temp.eip = 0;
      temp.cs_selector = 0;
      temp.opcode = 0;
      temp.data_offset = 0;
      temp.data_selector = 0;
    }
  else if (envp == FE_NOMASK_ENV)
    {
      temp.control_word &= ~(FE_ALL_EXCEPT | FE_TOWARDSZERO);
      temp.status_word &= ~FE_ALL_EXCEPT;
      temp.eip = 0;
      temp.cs_selector = 0;
      temp.opcode = 0;
      temp.data_offset = 0;
      temp.data_selector = 0;
    }
  else
    {
      temp.control_word &= ~(FE_ALL_EXCEPT | FE_TOWARDSZERO);
      temp.control_word |= (envp->control_word
			    & (FE_ALL_EXCEPT | FE_TOWARDSZERO));
      temp.status_word &= ~FE_ALL_EXCEPT;
      temp.status_word |= envp->status_word & FE_ALL_EXCEPT;
      temp.eip = envp->eip;
      temp.cs_selector = envp->cs_selector;
      temp.opcode = envp->opcode;
      temp.data_offset = envp->data_offset;
      temp.data_selector = envp->data_selector;
    }

  __asm__ ("fldenv %0" : : "m" (temp));
}
