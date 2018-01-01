/* Enumerate available IFUNC implementations of a function.  AARCH64 version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
#include <init-arch.h>
#include <stdio.h>

/* Maximum number of IFUNC implementations.  */
#define MAX_IFUNC	3

size_t
__libc_ifunc_impl_list (const char *name, struct libc_ifunc_impl *array,
			size_t max)
{
  assert (max >= MAX_IFUNC);

  size_t i = 0;

  INIT_ARCH ();

  /* Support sysdeps/aarch64/multiarch/memcpy.c and memmove.c.  */
  IFUNC_IMPL (i, name, memcpy,
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_thunderx)
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_falkor)
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_generic))
  IFUNC_IMPL (i, name, memmove,
	      IFUNC_IMPL_ADD (array, i, memmove, 1, __memmove_thunderx)
	      IFUNC_IMPL_ADD (array, i, memmove, 1, __memmove_falkor)
	      IFUNC_IMPL_ADD (array, i, memmove, 1, __memmove_generic))
  IFUNC_IMPL (i, name, memset,
	      /* Enable this on non-falkor processors too so that other cores
		 can do a comparative analysis with __memset_generic.  */
	      IFUNC_IMPL_ADD (array, i, memset, (zva_size == 64), __memset_falkor)
	      IFUNC_IMPL_ADD (array, i, memset, 1, __memset_generic))

  return i;
}
