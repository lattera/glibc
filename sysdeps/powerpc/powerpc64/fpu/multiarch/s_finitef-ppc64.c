/* finitef().  PowerPC64 default version.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
   Contributed by Luis Machado <luisgpm@br.ibm.com>.
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

#include <math.h>

#undef weak_alias
#define weak_alias(a, b)

#define FINITEF __finitef_ppc64
#ifdef SHARED
# undef hidden_def
# define hidden_def(a) \
   __hidden_ver1 (__finitef_ppc64, __GI___finitef, __finitef_ppc64);
#endif

#include <sysdeps/ieee754/flt-32/s_finitef.c>
