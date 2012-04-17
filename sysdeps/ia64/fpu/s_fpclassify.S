/* Return classification value corresponding to argument.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
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

#include <sysdep.h>
#undef ret

ENTRY (__fpclassify)
{
.mfi
	fclass.m p6, p7 = farg0, @nat | @qnan | @snan
	;;
}
{
.mfb
(p6)	mov ret0 = 0
(p7)	fclass.m p7, p8 = farg0, @inf
(p6)	br.ret.sptk.many rp
	;;
}	
{
.mfb
(p7)	mov ret0 = 1
(p8)	fclass.m p8, p6 = farg0, @zero
(p7)	br.ret.sptk.many rp
	;;
}
{
.mfb
(p8)	mov ret0 = 2
(p6)	fclass.m p6, p7 = farg0, @unorm
(p8)	br.ret.sptk.many rp
	;;
}
{
	.pred.rel.mutex p6, p7
(p6)	mov ret0 = 3
(p7)	mov ret0 = 4
	br.ret.sptk.many rp
}
END (__fpclassify)

strong_alias (__fpclassify, __fpclassifyf)
strong_alias (__fpclassify, __fpclassifyl)
libm_hidden_def (__fpclassify)
libm_hidden_def (__fpclassifyf)
libm_hidden_def (__fpclassifyl)
