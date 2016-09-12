/* Helper macros for double variants of type generic functions of libm.
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

#ifndef _MATH_TYPE_MACROS_DOUBLE
#define _MATH_TYPE_MACROS_DOUBLE

#define M_LIT(c) c
#define M_MLIT(c) c
#define M_PFX DBL
#define M_SUF(c) c
#define FLOAT double
#define CFLOAT _Complex double
#define M_STRTO_NAN __strtod_nan

/* Machines without a distinct long double type
   alias long double functions to their double
   equivalent.  */
#if defined NO_LONG_DOUBLE && !defined declare_mgen_alias
# define declare_mgen_alias(from, to)	    \
   weak_alias (from, to)		    \
   strong_alias (from, from ## l)	    \
   weak_alias (from, to ## l)
#endif

#if defined NO_LONG_DOUBLE && !defined declare_mgen_alias_2
# define declare_mgen_alias_2(from, to, to2) \
   declare_mgen_alias (from, to)	     \
   weak_alias (from, to2)		     \
   weak_alias (from, to2 ## l)
#endif

/* Supply the generic macros.  */
#include <math-type-macros.h>

#endif
