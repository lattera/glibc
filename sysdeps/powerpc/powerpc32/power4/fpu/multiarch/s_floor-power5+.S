/* floor function.  PowerPC32/power5+ version.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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
#define compat_symbol(lib, name, alias, ver)

#define __floor __floor_power5plus

#include <sysdeps/powerpc/powerpc32/power5+/fpu/s_floor.S>
