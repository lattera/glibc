/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#define scalbln __no_scalbln_decl
#define scalblnf __no_scalblnf_decl
#define scalblnl __no_scalblnl_decl
#define __scalbln __no__scalbln_decl
#define __scalblnf __no__scalblnf_decl
#define __scalblnl __no__scalblnl_decl
#include <math.h>
#undef scalbln
#undef scalblnf
#undef scalblnl
#undef __scalbln
#undef __scalblnf
#undef __scalblnl

#ifndef suffix
#define suffix /*empty*/
#endif
#ifndef float_type
#define float_type double
#endif

#define __CONCATX(a,b) __CONCAT(a,b)

float_type
__CONCATX(__scalbn,suffix) (x, exp)
     float_type x;
     int exp;
{
  return __m81_u(__CONCATX(__scalbn,suffix))(x, exp);
}

#define weak_aliasx(a,b) weak_alias(a,b)
#define strong_aliasx(a,b) strong_alias(a,b)
weak_aliasx (__CONCATX(__scalbn,suffix), __CONCATX(scalbn,suffix))
strong_aliasx (__CONCATX(__scalbn,suffix), __CONCATX(__scalbln,suffix))
weak_aliasx (__CONCATX(__scalbn,suffix), __CONCATX(scalbln,suffix))
