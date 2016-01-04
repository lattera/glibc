/* isnan().  PowerPC64/POWER7 version.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <math_ldbl_opt.h>

#undef hidden_def
#define hidden_def(name)
#undef weak_alias
#define weak_alias(name, alias)
#undef strong_alias
#define strong_alias(name, alias)
#undef compat_symbol
#define compat_symbol(lib, name, symbol, ver)

#define __isnan __isnan_power7

#include <sysdeps/powerpc/powerpc64/power7/fpu/s_isnan.S>
