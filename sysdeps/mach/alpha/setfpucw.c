/* Set FP exception mask and rounding mode.  Mach/Alpha version.
   Copyright (C) 2002 Free Software Foundation, Inc.
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


#define FPCR_DYN_SHIFT	58		/* first dynamic rounding mode bit */
#define FPCR_DYN_CHOPPED (0x0UL << FPCR_DYN_SHIFT)	/* towards 0 */
#define FPCR_DYN_MINUS	 (0x1UL << FPCR_DYN_SHIFT)	/* towards -INF */
#define FPCR_DYN_NORMAL	 (0x2UL << FPCR_DYN_SHIFT)	/* towards nearest */
#define FPCR_DYN_PLUS	 (0x3UL << FPCR_DYN_SHIFT)	/* towards +INF */
#define FPCR_DYN_MASK	 (0x3UL << FPCR_DYN_SHIFT)

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
  unsigned long fpcr;

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

  /* XXX trap bits? */

  __fpu_control = fpu_control;	/* update global copy */
}
