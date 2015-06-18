/* Offsets for data table for function sincos.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#ifndef D_SINCOS_DATA_H
#define D_SINCOS_DATA_H

#define __dSignMask                   	0
#define __dAbsMask                    	64
#define __dRangeVal                   	128
#define __dHalfPI                     	192
#define __dInvPI                      	256
#define __dRShifter                   	320
#define __dOneHalf                    	384
#define __dPI1                        	448
#define __dPI2                        	512
#define __dPI3                        	576
#define __dPI4                        	640
#define __dPI1_FMA                    	704
#define __dPI2_FMA                    	768
#define __dPI3_FMA                    	832
#define __dHalfPI1                    	896
#define __dHalfPI2                    	960
#define __dHalfPI3                    	1024
#define __dHalfPI4                    	1088
#define __dC1                         	1152
#define __dC2                         	1216
#define __dC3                         	1280
#define __dC4                         	1344
#define __dC5                         	1408
#define __dC6                         	1472
#define __dC7                         	1536

.macro double_vector offset value
.if .-__svml_dsincos_data != \offset
.err
.endif
.rept 8
.quad \value
.endr
.endm

#endif
