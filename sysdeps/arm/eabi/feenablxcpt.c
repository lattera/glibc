/* Enable floating-point exceptions.
   Copyright (C) 2001, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>, 2001.

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
feenableexcept (int excepts)
{
  if (GLRO (dl_hwcap) & HWCAP_ARM_VFP)
    {
      unsigned long int new_exc, old_exc;

      _FPU_GETCW(new_exc);

      old_exc = (new_exc >> FE_EXCEPT_SHIFT) & FE_ALL_EXCEPT;

      excepts &= FE_ALL_EXCEPT;

      new_exc |= (excepts << FE_EXCEPT_SHIFT);

      _FPU_SETCW(new_exc);

      return old_exc;
    }

  /* Unsupported, so return -1 for failure.  */
  return -1;
}
