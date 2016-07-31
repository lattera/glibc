/* floor function, sparc64 version.
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

#ifdef HAVE_AS_VIS3_SUPPORT
# include <sparc-ifunc.h>
# include <math.h>

extern double __floor_vis3 (double);
extern double __floor_generic (double);

sparc_libm_ifunc(__floor, hwcap & HWCAP_SPARC_VIS3 ? __floor_vis3 : __floor_generic);
weak_alias (__floor, floor)

# define __floor __floor_generic
#endif

#include <sysdeps/ieee754/dbl-64/wordsize-64/s_floor.c>
