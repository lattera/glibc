/* Set current rounding direction.
   Copyright (C) 2004, 2005 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <fpu_control.h>

#include <unistd.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>
#include <sysdep.h>

int
fesetround (int round)
{
  if (GLRO (dl_hwcap) & HWCAP_ARM_VFP)
    {
      fpu_control_t temp;

      switch (round)
	{
	case FE_TONEAREST:
	case FE_UPWARD:
	case FE_DOWNWARD:
	case FE_TOWARDZERO:
	  _FPU_GETCW (temp);
	  temp = (temp & ~FE_TOWARDZERO) | round;
	  _FPU_SETCW (temp);
	  return 0;
	default:
	  return 1;
	}
    }
  else if (round == FE_TONEAREST)
    /* This is the only supported rounding mode for soft-fp.  */
    return 0;

  /* Unsupported, so fail.  */
  return 1;
}

libm_hidden_def (fesetround)
