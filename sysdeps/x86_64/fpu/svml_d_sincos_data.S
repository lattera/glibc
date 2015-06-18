/* Data for function sincos.
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

#include "svml_d_sincos_data.h"

	.section .rodata, "a"
	.align 64

/* Data table for vector implementations of function sincos.
   The table may contain polynomial, reduction, lookup coefficients and
   other coefficients obtained through different methods of research and
   experimental work.  */

	.globl __svml_dsincos_data
__svml_dsincos_data:

/* General purpose constants:
   dSignMask */
double_vector __dSignMask 0x8000000000000000

/* dAbsMask */
double_vector __dAbsMask 0x7fffffffffffffff

/* lRangeVal */
double_vector __dRangeVal 0x4160000000000000

/* HalfPI */
double_vector __dHalfPI 0x3ff921fb54442d18

/* InvPI */
double_vector __dInvPI 0x3fd45f306dc9c883

/* dRShifter */
double_vector __dRShifter 0x4338000000000000

/* dOneHalf */
double_vector __dOneHalf 0x3fe0000000000000

/* Range reduction PI-based constants:
   PI1 */
double_vector __dPI1 0x400921fb40000000

/* PI2 */
double_vector __dPI2 0x3e84442d00000000

/* PI3 */
double_vector __dPI3 0x3d08469880000000

/* PI4 */
double_vector __dPI4 0x3b88cc51701b839a

/* Range reduction PI-based constants if FMA available:
   PI1_FMA */
double_vector __dPI1_FMA 0x400921fb54442d18

/* PI2_FMA */
double_vector __dPI2_FMA 0x3ca1a62633145c06

/* PI3_FMA */
double_vector __dPI3_FMA 0x395c1cd129024e09

/* HalfPI1 */
double_vector __dHalfPI1 0x3ff921fc00000000

/* HalfPI2 */
double_vector __dHalfPI2 0xbea5777a00000000

/* HalfPI3 */
double_vector __dHalfPI3 0xbd473dcc00000000

/* HalfPI4 */
double_vector __dHalfPI4 0x3bf898cc51701b84

/* Polynomial coefficients (relative error 2^(-52.115)):
   C1 */
double_vector __dC1 0xbfc55555555554a7

/* C2 */
double_vector __dC2 0x3f8111111110a4a8

/* C3 */
double_vector __dC3 0xbf2a01a019a5b86d

/* C4 */
double_vector __dC4 0x3ec71de38030fea0

/* C5 */
double_vector __dC5 0xbe5ae63546002231

/* C6 */
double_vector __dC6 0x3de60e6857a2f220

/* C7 */
double_vector __dC7 0xbd69f0d60811aac8
	.type	__svml_dsincos_data,@object
	.size __svml_dsincos_data,.-__svml_dsincos_data
