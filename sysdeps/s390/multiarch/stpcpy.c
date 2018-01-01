/* Multiple versions of stpcpy.
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
# define stpcpy __redirect_stpcpy
# define __stpcpy __redirect___stpcpy
/* Omit the stpcpy inline definitions because it would redefine stpcpy.  */
# define __NO_STRING_INLINES
# define NO_MEMPCPY_STPCPY_REDIRECT
# include <string.h>
# undef stpcpy
# undef __stpcpy
# include <ifunc-resolve.h>

s390_vx_libc_ifunc_redirected (__redirect___stpcpy, __stpcpy);
weak_alias (__stpcpy, stpcpy)

#else
# include <string/stpcpy.c>
#endif /* !(defined HAVE_S390_VX_ASM_SUPPORT && IS_IN (libc)) */
