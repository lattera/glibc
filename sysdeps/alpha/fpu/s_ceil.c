/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

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

#include <math.h>

double
__ceil (double x)
{
  if (x != 0 && fabs (x) < 9007199254740992.0)  /* 1 << DBL_MANT_DIG */
    {
      double tmp1;
      unsigned long fpcr0, fpcr1;
      unsigned long pinf = 3UL << 58;

      /* Set round to +inf.  */
      __asm __volatile("excb; mf_fpcr %0" : "=f"(fpcr0));
      __asm __volatile("mt_fpcr %0; excb" : : "f"(fpcr0 | pinf));

      /* Calculate!  */
#ifdef _IEEE_FP_INEXACT
      __asm("cvttq/svid %2,%1\n\tcvtqt/suid %1,%0"
	    : "=f"(x), "=&f"(tmp1)
	    : "f"(x));
#else
      __asm("cvttq/svd %2,%1\n\tcvtqt/d %1,%0"
	    : "=f"(x), "=&f"(tmp1)
	    : "f"(x));
#endif

      /* Reset rounding mode, while retaining new exception bits.  */
      __asm __volatile("excb; mf_fpcr %0" : "=f"(fpcr1));
      fpcr0 = (fpcr0 & pinf) | (fpcr1 & ~pinf);
      __asm __volatile("mt_fpcr %0; excb" : : "f"(fpcr0));
    }
  return x;
}

weak_alias (__ceil, ceil)
#ifdef NO_LONG_DOUBLE
strong_alias (__ceil, __ceill)
weak_alias (__ceil, ceill)
#endif
