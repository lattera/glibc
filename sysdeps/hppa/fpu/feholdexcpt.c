/* Store current floating-point environment and clear exceptions.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Huggins-Daines <dhd@debian.org>, 2000

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
#include <string.h>

int
feholdexcept (fenv_t *envp)
{
  fenv_t clear;
  fenv_t * _regs = envp;

  /* Store the environment.  */
  __asm__ (
	   "fstd,ma %%fr0,8(%1)\n"
	   "fstd,ma %%fr1,8(%1)\n"
	   "fstd,ma %%fr2,8(%1)\n"
	   "fstd %%fr3,0(%1)\n"
	   : "=m" (*_regs), "+r" (_regs));
  memcpy (&clear, envp, sizeof (clear));

  /* Now clear all exceptions.  */
  clear.__status_word &= ~(FE_ALL_EXCEPT << 27);
  memset (clear.__exception, 0, sizeof (clear.__exception));

  /* And set all exceptions to non-stop.  */
  clear.__status_word &= ~FE_ALL_EXCEPT;

  /* Load the new environment. */
  _regs = &clear;
  __asm__ (
	   "fldd,ma 8(%0),%%fr0\n"
	   "fldd,ma 8(%0),%%fr1\n"
	   "fldd,ma 8(%0),%%fr2\n"
	   "fldd 0(%0),%%fr3\n"
	   : : "r" (_regs));

  return 0;
}
