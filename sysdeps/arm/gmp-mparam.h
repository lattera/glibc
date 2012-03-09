/* gmp-mparam.h -- Compiler/machine parameter header file.

Copyright (C) 1991, 1993, 1994, 1995, 2005 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library.  If not, see <http://www.gnu.org/licenses/>.  */

#define BITS_PER_MP_LIMB 32
#define BYTES_PER_MP_LIMB 4
#define BITS_PER_LONGINT 32
#define BITS_PER_INT 32
#define BITS_PER_SHORTINT 16
#define BITS_PER_CHAR 8

#if defined(__ARMEB__)
# define IEEE_DOUBLE_MIXED_ENDIAN 0
# define IEEE_DOUBLE_BIG_ENDIAN 1
#elif defined(__VFP_FP__)
# define IEEE_DOUBLE_MIXED_ENDIAN 0
# define IEEE_DOUBLE_BIG_ENDIAN 0
#else
# define IEEE_DOUBLE_BIG_ENDIAN 0
# define IEEE_DOUBLE_MIXED_ENDIAN 1
#endif
