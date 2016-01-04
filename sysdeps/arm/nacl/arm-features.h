/* Macros to test for CPU features on ARM.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _NACL_ARM_FEATURES_H
#define _NACL_ARM_FEATURES_H 1

#ifdef __SOFTFP__
# error NaCl should always have VFP enabled
#endif

/* NaCl does not support iWMMXt at all.  */
#define ARM_ASSUME_NO_IWMMXT    1

/* NaCl does not allow instructions to target the pc register.  */
#define ARM_ALWAYS_BX           1

/* Computed branch targets must be bundle-aligned, which is to 16 bytes.  */
#define ARM_BX_ALIGN_LOG2       4

/* Two-register addressing modes are never allowed.  */
#define ARM_NO_INDEX_REGISTER   1

/* Only ARM mode code is allowed, never Thumb mode.  */
#define NO_THUMB

#include_next <arm-features.h>

#endif  /* arm-features.h */
