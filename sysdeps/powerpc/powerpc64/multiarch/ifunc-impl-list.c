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

  /* Support sysdeps/powerpc/powerpc64/multiarch/memset.c.  */
  IFUNC_IMPL (i, name, memset,
	      IFUNC_IMPL_ADD (array, i, memset, hwcap & PPC_FEATURE_HAS_VSX,
			      __memset_power7)
	      IFUNC_IMPL_ADD (array, i, memset, hwcap & PPC_FEATURE_ARCH_2_05,
			      __memset_power6)
	      IFUNC_IMPL_ADD (array, i, memset, hwcap & PPC_FEATURE_POWER4,
			      __memset_power4)
	      IFUNC_IMPL_ADD (array, i, memset, 1, __memset_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/strlen.c.  */
  IFUNC_IMPL (i, name, strlen,
	      IFUNC_IMPL_ADD (array, i, strlen, hwcap & PPC_FEATURE_HAS_VSX,
			      __strlen_power7)
	      IFUNC_IMPL_ADD (array, i, strlen, 1,
			      __strlen_ppc))
#endif

  /* Support sysdeps/powerpc/powerpc64/multiarch/memcmp.c.  */
  IFUNC_IMPL (i, name, memcmp,
	      IFUNC_IMPL_ADD (array, i, memcmp, hwcap & PPC_FEATURE_HAS_VSX,
			      __memcmp_power7)
	      IFUNC_IMPL_ADD (array, i, memcmp, hwcap & PPC_FEATURE_POWER4,
			      __memcmp_power4)
	      IFUNC_IMPL_ADD (array, i, memcmp, 1, __memcmp_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/bzero.c.  */
  IFUNC_IMPL (i, name, bzero,
	      IFUNC_IMPL_ADD (array, i, bzero, hwcap & PPC_FEATURE_HAS_VSX,
			      __bzero_power7)
	      IFUNC_IMPL_ADD (array, i, bzero, hwcap & PPC_FEATURE_ARCH_2_05,
			      __bzero_power6)
	      IFUNC_IMPL_ADD (array, i, bzero, hwcap & PPC_FEATURE_POWER4,
			      __bzero_power4)
	      IFUNC_IMPL_ADD (array, i, bzero, 1, __bzero_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/mempcpy.c.  */
  IFUNC_IMPL (i, name, mempcpy,
	      IFUNC_IMPL_ADD (array, i, mempcpy,
			      hwcap & PPC_FEATURE_HAS_VSX,
			      __mempcpy_power7)
	      IFUNC_IMPL_ADD (array, i, mempcpy, 1,
			      __mempcpy_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/memchr.c.  */
  IFUNC_IMPL (i, name, memchr,
	      IFUNC_IMPL_ADD (array, i, memchr,
			      hwcap & PPC_FEATURE_HAS_VSX,
			      __memchr_power7)
	      IFUNC_IMPL_ADD (array, i, memchr, 1,
			      __memchr_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/memrchr.c.  */
  IFUNC_IMPL (i, name, memrchr,
	      IFUNC_IMPL_ADD (array, i, memrchr,
			      hwcap & PPC_FEATURE_HAS_VSX,
			      __memrchr_power7)
	      IFUNC_IMPL_ADD (array, i, memrchr, 1,
			      __memrchr_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/rawmemchr.c.  */
  IFUNC_IMPL (i, name, rawmemchr,
	      IFUNC_IMPL_ADD (array, i, rawmemchr,
			      hwcap & PPC_FEATURE_HAS_VSX,
			      __rawmemchr_power7)
	      IFUNC_IMPL_ADD (array, i, rawmemchr, 1,
			      __rawmemchr_ppc))

  /* Support sysdeps/powerpc/powerpc64/multiarch/strnlen.c.  */
  IFUNC_IMPL (i, name, strnlen,
	      IFUNC_IMPL_ADD (array, i, strnlen, hwcap & PPC_FEATURE_HAS_VSX,
			      __strnlen_power7)
	      IFUNC_IMPL_ADD (array, i, strnlen, 1,
			      __strnlen_ppc))

  return i;
}
