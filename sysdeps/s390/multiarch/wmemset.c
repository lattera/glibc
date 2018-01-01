/* Multiple versions of wmemset.
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

#if defined HAVE_S390_VX_ASM_SUPPORT && IS_IN (libc)
# define wmemset __redirect_wmemset
# define __wmemset __redirect___wmemset
# include <wchar.h>
# undef wmemset
# undef __wmemset
# include <ifunc-resolve.h>

s390_vx_libc_ifunc_redirected (__redirect___wmemset, __wmemset)
weak_alias (__wmemset, wmemset)

#else
# include <wcsmbs/wmemset.c>
#endif /* !(defined HAVE_S390_VX_ASM_SUPPORT && IS_IN (libc)) */
