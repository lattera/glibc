/* Return current rounding direction.
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

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include <fenv.h>
#include <fpu_control.h>

#include <unistd.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>
#include <sysdep.h>

int
fegetround (void)
{
  if (GLRO (dl_hwcap) & HWCAP_ARM_VFP)
    {
      unsigned int temp;

      /* Get the current environment.  */
      _FPU_GETCW (temp);

      return temp & FE_TOWARDZERO;
    }

  /* The current soft-float implementation only handles TONEAREST.  */
  return FE_TONEAREST;
}
