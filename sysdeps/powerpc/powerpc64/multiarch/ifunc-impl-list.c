/* Enumerate available IFUNC implementations of a function.  PowerPC64 version.
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

#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <ldsodefs.h>
#include <ifunc-impl-list.h>

/* Maximum number of IFUNC implementations.  */
#define MAX_IFUNC	6

size_t
__libc_ifunc_impl_list (const char *name, struct libc_ifunc_impl *array,
			size_t max)
{
  assert (max >= MAX_IFUNC);

  size_t i = 0;

  unsigned long int hwcap = GLRO(dl_hwcap);
  /* hwcap contains only the latest supported ISA, the code checks which is
     and fills the previous supported ones.  */
  if (hwcap & PPC_FEATURE_ARCH_2_06)
    hwcap |= PPC_FEATURE_ARCH_2_05 | PPC_FEATURE_POWER5_PLUS |
             PPC_FEATURE_POWER5 | PPC_FEATURE_POWER4;
  else if (hwcap & PPC_FEATURE_ARCH_2_05)
    hwcap |= PPC_FEATURE_POWER5_PLUS | PPC_FEATURE_POWER5 | PPC_FEATURE_POWER4;
  else if (hwcap & PPC_FEATURE_POWER5_PLUS)
    hwcap |= PPC_FEATURE_POWER5 | PPC_FEATURE_POWER4;
  else if (hwcap & PPC_FEATURE_POWER5)
    hwcap |= PPC_FEATURE_POWER4;

#ifdef SHARED
  /* Support sysdeps/powerpc/powerpc64/multiarch/memcpy.c.  */
  IFUNC_IMPL (i, name, memcpy,
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & PPC_FEATURE_HAS_VSX,
			      __memcpy_power7)
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & PPC_FEATURE_ARCH_2_06,
			      __memcpy_a2)
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & PPC_FEATURE_ARCH_2_05,
			      __memcpy_power6)
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & PPC_FEATURE_CELL_BE,
			      __memcpy_cell)
	      IFUNC_IMPL_ADD (array, i, memcpy, hwcap & PPC_FEATURE_POWER4,
			      __memcpy_power4)
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_ppc))
#endif

  return i;
}
