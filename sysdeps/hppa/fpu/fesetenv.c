/* Install given floating-point environment.
   Copyright (C) 1997, 1999, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Huggins-Daines <dhd@debian.org>, 2000
   Based on the m68k version by
   Andreas Schwab <schwab@suse.de>

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

int
fesetenv (const fenv_t *envp)
{
  fenv_t temp;
  fenv_t * _regs = &temp;

  /* Install the environment specified by ENVP.  But there are a few
     values which we do not want to come from the saved environment.
     Therefore, we get the current environment and replace the values
     we want to use from the environment specified by the parameter.  */
  __asm__ (
	   "fstd,ma %%fr0,8(%1)\n"
	   "fstd,ma %%fr1,8(%1)\n"
	   "fstd,ma %%fr2,8(%1)\n"
	   "fstd %%fr3,0(%1)\n"
	   : "=m" (*_regs), "+r" (_regs));

  temp.__status_word &= ~(FE_ALL_EXCEPT
			  | (FE_ALL_EXCEPT << 27)
			  | FE_DOWNWARD);
  if (envp == FE_DFL_ENV)
    ;
  else if (envp == FE_NOMASK_ENV)
    temp.__status_word |= FE_ALL_EXCEPT;
  else
    temp.__status_word |= (envp->__status_word
			   & (FE_ALL_EXCEPT
			      | FE_DOWNWARD
			      | (FE_ALL_EXCEPT << 27)));

  /* Load the new environment. */
  __asm__ (
	   "fldd,ma -8(%1),%%fr3\n"
	   "fldd,ma -8(%1),%%fr2\n"
	   "fldd,ma -8(%1),%%fr1\n"
	   "fldd 0(%1),%%fr0\n"
	   : "=m" (*_regs), "+r" (_regs));

  /* Success.  */
  return 0;
}
libm_hidden_def (fesetenv)
