/* Multiple versions of mempcpy.
   Copyright (C) 2016 Free Software Foundation, Inc.
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


#if defined SHARED && IS_IN (libc)
# define mempcpy __redirect_mempcpy
# define __mempcpy __redirect___mempcpy
/* Omit the mempcpy inline definitions because it would redefine mempcpy.  */
# define _HAVE_STRING_ARCH_mempcpy 1
# include <string.h>
# undef mempcpy
# undef __mempcpy
# include <ifunc-resolve.h>

s390_libc_ifunc (__redirect___mempcpy, ____mempcpy, __mempcpy)
weak_alias (__mempcpy, mempcpy);
#endif
