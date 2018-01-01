/* Multiple versions of cosf
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

#include <init-arch.h>
#include <libm-alias-float.h>

extern float __cosf_sse2 (float);
extern float __cosf_ia32 (float);
float __cosf (float);

libm_ifunc (__cosf, HAS_CPU_FEATURE (SSE2) ? __cosf_sse2 : __cosf_ia32);
libm_alias_float (__cos, cos);

#define COSF __cosf_ia32
#include <sysdeps/ieee754/flt-32/s_cosf.c>
