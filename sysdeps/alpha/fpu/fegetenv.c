/* Store current floating-point environment.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>, 1997

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
fegetenv (fenv_t *envp)
{
  unsigned long fpcr, swcr;

  /* Get status from software and hardware.  Note that we don't need an
     excb because the callsys is an implied trap barrier.  */
  swcr = __ieee_get_fp_control();
  __asm__ __volatile__("mf_fpcr %0" : "=f"(fpcr));

  /* Merge the two bits of information.  The magic number at the end is
     the exception enable mask.  */

  *envp = (fpcr & (3UL << 58)) | (swcr & (FE_ALL_EXCEPT | 0x3e));
}
