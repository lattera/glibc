/* Offsets for data table for vectorized sin.
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

#ifndef D_SIN_DATA_H
#define D_SIN_DATA_H

/* Offsets for data table */
#define __dAbsMask                    	0
#define __dRangeVal                   	64
#define __dInvPI                      	128
#define __dRShifter                   	192
#define __dZero                       	256
#define __lNZero                      	320
#define __dPI1                        	384
#define __dPI2                        	448
#define __dPI3                        	512
#define __dPI4                        	576
#define __dPI1_FMA                    	640
#define __dPI2_FMA                    	704
#define __dPI3_FMA                    	768
#define __dC1                         	832
#define __dC2                         	896
#define __dC3                         	960
#define __dC4                         	1024
#define __dC5                         	1088
#define __dC6                         	1152
#define __dC7                         	1216

.macro double_vector offset value
.if .-__svml_dsin_data != \offset
.err
.endif
.rept 8
.quad \value
.endr
.endm

#endif
