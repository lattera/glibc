/* Set FP exception mask and rounding mode.
   Copyright (C) 1996, 1997, 1998, 2003 Free Software Foundation, Inc.
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

#include <fpu_control.h>
#include <asm/fpu.h>

extern void		__ieee_set_fp_control (unsigned long);
libc_hidden_proto(__ieee_set_fp_control)

extern unsigned long	__ieee_get_fp_control (void);
libc_hidden_proto(__ieee_get_fp_control)

static inline unsigned long
rdfpcr (void)
{
  unsigned long fpcr;
  asm ("excb; mf_fpcr %0" : "=f"(fpcr));
  return fpcr;
}


static inline void
wrfpcr (unsigned long fpcr)
{
  asm volatile ("mt_fpcr %0; excb" : : "f"(fpcr));
}


void
__setfpucw (fpu_control_t fpu_control)
{
  unsigned long fpcr = 0, fpcw = 0;

  if (!fpu_control)
    fpu_control = _FPU_DEFAULT;

  /* first, set dynamic rounding mode: */

  fpcr = rdfpcr();
  fpcr &= ~FPCR_DYN_MASK;
  switch (fpu_control & 0xc00)
    {
    case _FPU_RC_NEAREST:	fpcr |= FPCR_DYN_NORMAL; break;
    case _FPU_RC_DOWN:		fpcr |= FPCR_DYN_MINUS; break;
    case _FPU_RC_UP:		fpcr |= FPCR_DYN_PLUS; break;
    case _FPU_RC_ZERO:		fpcr |= FPCR_DYN_CHOPPED; break;
    }
  wrfpcr(fpcr);

  /* now tell kernel about traps that we like to hear about: */

  fpcw = __ieee_get_fp_control();
  fpcw &= ~IEEE_TRAP_ENABLE_MASK;

  if (!(fpu_control & _FPU_MASK_IM)) fpcw |= IEEE_TRAP_ENABLE_INV;
  if (!(fpu_control & _FPU_MASK_DM)) fpcw |= IEEE_TRAP_ENABLE_UNF;
  if (!(fpu_control & _FPU_MASK_ZM)) fpcw |= IEEE_TRAP_ENABLE_DZE;
  if (!(fpu_control & _FPU_MASK_OM)) fpcw |= IEEE_TRAP_ENABLE_OVF;
  if (!(fpu_control & _FPU_MASK_PM)) fpcw |= IEEE_TRAP_ENABLE_INE;

  __fpu_control = fpu_control;	/* update global copy */

  __ieee_set_fp_control(fpcw);
}
