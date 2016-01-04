/* Wrapper part of tests for AVX ISA versions of vector math functions.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#include "test-float-vlen8.h"
#include <immintrin.h>

#define VEC_TYPE __m256

VECTOR_WRAPPER (WRAPPER_NAME (cosf), _ZGVcN8v_cosf)
VECTOR_WRAPPER (WRAPPER_NAME (sinf), _ZGVcN8v_sinf)
VECTOR_WRAPPER_fFF (WRAPPER_NAME (sincosf), _ZGVcN8vvv_sincosf)
VECTOR_WRAPPER (WRAPPER_NAME (logf), _ZGVcN8v_logf)
VECTOR_WRAPPER (WRAPPER_NAME (expf), _ZGVcN8v_expf)
VECTOR_WRAPPER_ff (WRAPPER_NAME (powf), _ZGVcN8vv_powf)
