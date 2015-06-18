/* Offsets for data table for function sincosf.
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

#ifndef S_SINCOSF_DATA_H
#define S_SINCOSF_DATA_H

#define __dT                          	0
#define __sAbsMask                    	4096
#define __sRangeReductionVal          	4160
#define __sRangeVal                   	4224
#define __sS1                         	4288
#define __sS2                         	4352
#define __sC1                         	4416
#define __sC2                         	4480
#define __iIndexMask                  	4544
#define __i2pK_1                      	4608
#define __sSignMask                   	4672
#define __sPI1                        	4736
#define __sPI2                        	4800
#define __sPI3                        	4864
#define __sPI4                        	4928
#define __sPI1_FMA                    	4992
#define __sPI2_FMA                    	5056
#define __sPI3_FMA                    	5120
#define __sA3                         	5184
#define __sA5                         	5248
#define __sA7                         	5312
#define __sA9                         	5376
#define __sA5_FMA                     	5440
#define __sA7_FMA                     	5504
#define __sA9_FMA                     	5568
#define __sInvPI                      	5632
#define __sRShifter                   	5696
#define __sHalfPI                     	5760
#define __sOneHalf                    	5824

.macro float_vector offset value
.if .-__svml_ssincos_data != \offset
.err
.endif
.rept 16
.long \value
.endr
.endm

#endif
