/* Copyright (C) 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv_libc.h>
#include <sysdep.h>
#include <float.h>
#include "kernel-features.h"
#include "kernel_sysinfo.h"


int
__feraiseexcept (int excepts)
{
  INTERNAL_SYSCALL_DECL (err);
  unsigned long t = excepts;
  long r;

  r = INTERNAL_SYSCALL (osf_setsysinfo, err, 2, SSI_IEEE_RAISE_EXCEPTION, &t);

#ifndef __ASSUME_IEEE_RAISE_EXCEPTION
  if (!INTERNAL_SYSCALL_ERROR_P (r, err))
    return 0;

  double d;

  /* If we got an error from SSI_IEEE_RAISE_EXCEPTION, assume it means that
     the system call isn't actually implemented.  Do the best we can.  */

  /* Invalid implemented with 0 / 0 -> NaN.  */
  if (excepts & FE_INVALID)
    __asm__ __volatile__ ("divs/su $f31,$f31,%0; trapb" : "=f"(d) : );

  /* Division By Zero implemented with 1 / 0 -> NaN.  */
  if (excepts & FE_DIVBYZERO)
    __asm__ __volatile__ ("divs/su %1,$f31,%0; trapb" : "=&f"(d) : "f"(1.0f));

  /* Overflow and underflow cannot be had all by themselves.  We can
     generate them with arithmetic, but we always get INEXACT raised
     at the same time.  Prepare to undo.  */
  if ((excepts & (FE_OVERFLOW | FE_UNDERFLOW)) && !(excepts & FE_INEXACT))
    INTERNAL_SYSCALL (osf_getsysinfo, err, 2, GSI_IEEE_FP_CONTROL, &t);

  /* Overflow implemented with FLT_MAX + FLT_MAX -> Inf.  */
  if (excepts & FE_OVERFLOW)
    __asm__ __volatile__ ("adds/sui %1,%1,%0; trapb"
			  : "=&f"(d) : "f"(FLT_MAX));

  /* Underflow implemented with FLT_MIN * FLT_MIN -> 0.  */
  if (excepts & FE_UNDERFLOW)
    __asm__ __volatile__ ("muls/sui %1,%1,%0; trapb"
			  : "=&f"(d) : "f"(FLT_MIN));

  /* Inexact implemented with (long)0.5 -> 0.  */
  if ((excepts & (FE_OVERFLOW | FE_UNDERFLOW | FE_INEXACT)) == FE_INEXACT)
    __asm__ __volatile__ ("cvttq/svi %1,%0; trapb" : "=&f"(d) : "f"(0.5f));

  /* If we raised inexact when not asked, and inexact was not previously
     raised, then clear that exception.  */
  if ((excepts & (FE_OVERFLOW | FE_UNDERFLOW))
      && !((excepts | t) & FE_INEXACT))
    {
      t |= excepts & SWCR_STATUS_MASK;
      INTERNAL_SYSCALL (osf_setsysinfo, err, 2, SSI_IEEE_FP_CONTROL, &t);
    }
#endif /* !__ASSUME_IEEE_RAISE_EXCEPTION */

  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feraiseexcept, __old_feraiseexcept)
compat_symbol (libm, __old_feraiseexcept, feraiseexcept, GLIBC_2_1);
#endif

libm_hidden_ver (__feraiseexcept, feraiseexcept)
versioned_symbol (libm, __feraiseexcept, feraiseexcept, GLIBC_2_2);
