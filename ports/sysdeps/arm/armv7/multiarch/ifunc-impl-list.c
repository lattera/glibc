/* Enumerate available IFUNC implementations of a function.  ARM version.
   Copyright (C) 2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <ldsodefs.h>
#include <sysdep.h>
#include <ifunc-impl-list.h>

/* Fill ARRAY of MAX elements with IFUNC implementations for function
   NAME and return the number of valid entries.  */

size_t
__libc_ifunc_impl_list (const char *name, struct libc_ifunc_impl *array,
			size_t max)
{
  size_t i = 0;
  int hwcap;

  hwcap = GLRO(dl_hwcap);

  IFUNC_IMPL (i, name, memcpy,
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & HWCAP_ARM_NEON,
#ifdef __ARM_NEON__
                              memcpy
#else
			      __memcpy_neon
#endif
                              )
#ifndef __ARM_NEON__
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & HWCAP_ARM_VFP,
			      __memcpy_vfp)
#endif
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_arm));

  return i;
}
