/* Compatibility functions for floating point formatting, long double version.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <float.h>

#define FLOAT_TYPE long double
#define FUNC_PREFIX q
#define FLOAT_FMT_FLAG "L"
/* Actually we have to write (LDBL_DIG + log10 (LDBL_MAX_10_EXP)) but
   we don't have log10 available in the preprocessor.  Since we cannot
   assume anything on the used `long double' format be generous.  */
#define MAXDIG (NDIGIT_MAX + 12)
#define FCVT_MAXDIG (LDBL_MAX_10_EXP + MAXDIG)
#if LDBL_MANT_DIG == 64
# define NDIGIT_MAX 21
#elif LDBL_MANT_DIG == 53
# define NDIGIT_MAX 17
#elif LDBL_MANT_DIG == 113
# define NDIGIT_MAX 36
#elif LDBL_MANT_DIG == 106
# define NDIGIT_MAX 34
#elif LDBL_MANT_DIG == 56
# define NDIGIT_MAX 18
#else
/* See IEEE 854 5.6, table 2 for this formula.  Unfortunately we need a
   compile time constant here, so we cannot use it.  */
# error "NDIGIT_MAX must be precomputed"
# define NDIGIT_MAX (lrint (ceil (M_LN2 / M_LN10 * LDBL_MANT_DIG + 1.0)))
#endif

#include "efgcvt.c"
