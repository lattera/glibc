/* Return cosine of complex float value.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

#define __ccosf __ccosf_not_defined
#define ccosf ccosf_not_defined

#include <complex.h>
#include <math.h>

#undef __ccosf
#undef ccosf
#define __ccosf internal_ccosf

static _Complex float internal_ccosf (_Complex float x);

#include <math/s_ccosf.c>
#include "cfloat-compat.h"

#undef __ccosf

c1_cfloat_rettype
__c1_ccosf (c1_cfloat_decl (x))
{
  _Complex float r = internal_ccosf (c1_cfloat_value (x));
  return c1_cfloat_return (r);
}

c2_cfloat_rettype
__c2_ccosf (c2_cfloat_decl (x))
{
  _Complex float r = internal_ccosf (c2_cfloat_value (x));
  return c2_cfloat_return (r);
}

cfloat_versions (ccosf);
