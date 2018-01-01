/* Enumerate available IFUNC implementations of a function. s390/s390x version.
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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
#include <ifunc-impl-list.h>
#include <ifunc-resolve.h>

/* Maximum number of IFUNC implementations.  */
#define MAX_IFUNC	3

/* Fill ARRAY of MAX elements with IFUNC implementations for function
   NAME supported on target machine and return the number of valid
   entries.  */
size_t
__libc_ifunc_impl_list (const char *name, struct libc_ifunc_impl *array,
			size_t max)
{
  assert (max >= MAX_IFUNC);

  size_t i = 0;

  /* Get hardware information.  */
  unsigned long int dl_hwcap = GLRO (dl_hwcap);
  unsigned long long stfle_bits = 0ULL;
  if ((dl_hwcap & HWCAP_S390_STFLE)
	&& (dl_hwcap & HWCAP_S390_ZARCH)
	&& (dl_hwcap & HWCAP_S390_HIGH_GPRS))
    {
      S390_STORE_STFLE (stfle_bits);
    }

  IFUNC_IMPL (i, name, memset,
	      IFUNC_IMPL_ADD (array, i, memset,
			      S390_IS_Z196 (stfle_bits), __memset_z196)
	      IFUNC_IMPL_ADD (array, i, memset,
			      S390_IS_Z10 (stfle_bits), __memset_z10)
	      IFUNC_IMPL_ADD (array, i, memset, 1, __memset_default))

  IFUNC_IMPL (i, name, memcmp,
	      IFUNC_IMPL_ADD (array, i, memcmp,
			      S390_IS_Z196 (stfle_bits), __memcmp_z196)
	      IFUNC_IMPL_ADD (array, i, memcmp,
			      S390_IS_Z10 (stfle_bits), __memcmp_z10)
	      IFUNC_IMPL_ADD (array, i, memcmp, 1, __memcmp_default))

#ifdef SHARED

  IFUNC_IMPL (i, name, memcpy,
	      IFUNC_IMPL_ADD (array, i, memcpy,
			      S390_IS_Z196 (stfle_bits), __memcpy_z196)
	      IFUNC_IMPL_ADD (array, i, memcpy,
			      S390_IS_Z10 (stfle_bits), __memcpy_z10)
	      IFUNC_IMPL_ADD (array, i, memcpy, 1, __memcpy_default))

  IFUNC_IMPL (i, name, mempcpy,
	      IFUNC_IMPL_ADD (array, i, mempcpy,
			      S390_IS_Z196 (stfle_bits), ____mempcpy_z196)
	      IFUNC_IMPL_ADD (array, i, mempcpy,
			      S390_IS_Z10 (stfle_bits), ____mempcpy_z10)
	      IFUNC_IMPL_ADD (array, i, mempcpy, 1, ____mempcpy_default))

#endif /* SHARED */

#ifdef HAVE_S390_VX_ASM_SUPPORT

# define IFUNC_VX_IMPL(FUNC)						\
  IFUNC_IMPL (i, name, FUNC,						\
	      IFUNC_IMPL_ADD (array, i, FUNC, dl_hwcap & HWCAP_S390_VX, \
			      __##FUNC##_vx)				\
	      IFUNC_IMPL_ADD (array, i, FUNC, 1, __##FUNC##_c))

  IFUNC_VX_IMPL (strlen);
  IFUNC_VX_IMPL (wcslen);

  IFUNC_VX_IMPL (strnlen);
  IFUNC_VX_IMPL (wcsnlen);

  IFUNC_VX_IMPL (strcpy);
  IFUNC_VX_IMPL (wcscpy);

  IFUNC_VX_IMPL (stpcpy);
  IFUNC_VX_IMPL (wcpcpy);

  IFUNC_VX_IMPL (strncpy);
  IFUNC_VX_IMPL (wcsncpy);

  IFUNC_VX_IMPL (stpncpy);
  IFUNC_VX_IMPL (wcpncpy);

  IFUNC_VX_IMPL (strcat);
  IFUNC_VX_IMPL (wcscat);

  IFUNC_VX_IMPL (strncat);
  IFUNC_VX_IMPL (wcsncat);

  IFUNC_VX_IMPL (strcmp);
  IFUNC_VX_IMPL (wcscmp);

  IFUNC_VX_IMPL (strncmp);
  IFUNC_VX_IMPL (wcsncmp);

  IFUNC_VX_IMPL (strchr);
  IFUNC_VX_IMPL (wcschr);

  IFUNC_VX_IMPL (strchrnul);
  IFUNC_VX_IMPL (wcschrnul);

  IFUNC_VX_IMPL (strrchr);
  IFUNC_VX_IMPL (wcsrchr);

  IFUNC_VX_IMPL (strspn);
  IFUNC_VX_IMPL (wcsspn);

  IFUNC_VX_IMPL (strpbrk);
  IFUNC_VX_IMPL (wcspbrk);

  IFUNC_VX_IMPL (strcspn);
  IFUNC_VX_IMPL (wcscspn);

  IFUNC_VX_IMPL (memchr);
  IFUNC_VX_IMPL (wmemchr);
  IFUNC_VX_IMPL (rawmemchr);

  IFUNC_VX_IMPL (memccpy);

  IFUNC_VX_IMPL (wmemset);

  IFUNC_VX_IMPL (wmemcmp);

  IFUNC_VX_IMPL (memrchr);

#endif /* HAVE_S390_VX_ASM_SUPPORT */

  return i;
}
