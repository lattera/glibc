/*
 * IBM Accurate Mathematical Library
 * Written by International Business Machines Corp.
 * Copyright (C) 2001, 2011 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/******************************************************************/
/*                                                                */
/* MODULE_NAME:mpexp.h                                            */
/*                                                                */
/* common data and variables prototype and definition             */
/******************************************************************/

#ifndef MPEXP_H
#define MPEXP_H

extern const number __mpexp_twomm1[33] attribute_hidden;
extern const number __mpexp_nn[9] attribute_hidden;
extern const number __mpexp_radix attribute_hidden;
extern const number __mpexp_radixi attribute_hidden;
extern const number __mpexp_zero attribute_hidden;
extern const number __mpexp_one attribute_hidden;
extern const number __mpexp_two attribute_hidden;
extern const number __mpexp_half attribute_hidden;


#ifndef AVOID_MPEXP_H
#ifdef BIG_ENDI
  const number
	__mpexp_twomm1[33] = {                     /* 2**-m1 */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x3ee00000, 0x00000000} }, /* 2**-17 */
/**/                  {{0x3e800000, 0x00000000} }, /* 2**-23 */
/**/                  {{0x3e800000, 0x00000000} }, /* 2**-23 */
/**/                  {{0x3e300000, 0x00000000} }, /* 2**-28 */
/**/                  {{0x3e400000, 0x00000000} }, /* 2**-27 */
/**/                  {{0x3d900000, 0x00000000} }, /* 2**-38 */
/**/                  {{0x3d500000, 0x00000000} }, /* 2**-42 */
/**/                  {{0x3d800000, 0x00000000} }, /* 2**-39 */
/**/                  {{0x3d400000, 0x00000000} }, /* 2**-43 */
/**/                  {{0x3d000000, 0x00000000} }, /* 2**-47 */
/**/                  {{0x3d400000, 0x00000000} }, /* 2**-43 */
/**/                  {{0x3d000000, 0x00000000} }, /* 2**-47 */
/**/                  {{0x3cd00000, 0x00000000} }, /* 2**-50 */
/**/                  {{0x3c900000, 0x00000000} }, /* 2**-54 */
/**/                  {{0x3c600000, 0x00000000} }, /* 2**-57 */
/**/                  {{0x3c300000, 0x00000000} }, /* 2**-60 */
/**/                  {{0x3bf00000, 0x00000000} }, /* 2**-64 */
/**/                  {{0x3bc00000, 0x00000000} }, /* 2**-67 */
/**/                  {{0x3b800000, 0x00000000} }, /* 2**-71 */
/**/                  {{0x3b500000, 0x00000000} }, /* 2**-74 */
/**/                  {{0x3bb00000, 0x00000000} }, /* 2**-68 */
/**/                  {{0x3b800000, 0x00000000} }, /* 2**-71 */
/**/                  {{0x3b500000, 0x00000000} }, /* 2**-74 */
/**/                  {{0x3b200000, 0x00000000} }, /* 2**-77 */
/**/                  {{0x3b900000, 0x00000000} }, /* 2**-70 */
/**/                  {{0x3b600000, 0x00000000} }, /* 2**-73 */
/**/                  {{0x3b300000, 0x00000000} }, /* 2**-76 */
/**/                  {{0x3b100000, 0x00000000} }, /* 2**-78 */
/**/                  {{0x3ae00000, 0x00000000} }, /* 2**-81 */
  };
  const number
	       __mpexp_nn[9]={                     /* n      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x3ff00000, 0x00000000} }, /* 1      */
/**/                  {{0x40000000, 0x00000000} }, /* 2      */
/**/                  {{0x40080000, 0x00000000} }, /* 3      */
/**/                  {{0x40100000, 0x00000000} }, /* 4      */
/**/                  {{0x40140000, 0x00000000} }, /* 5      */
/**/                  {{0x40180000, 0x00000000} }, /* 6      */
/**/                  {{0x401c0000, 0x00000000} }, /* 7      */
/**/                  {{0x40200000, 0x00000000} }, /* 8      */
  };

  const number
/**/ __mpexp_radix    = {{0x41700000, 0x00000000} }, /* 2**24  */
/**/ __mpexp_radixi   = {{0x3e700000, 0x00000000} }, /* 2**-24 */
/**/ __mpexp_zero     = {{0x00000000, 0x00000000} }, /* 0      */
/**/ __mpexp_one      = {{0x3ff00000, 0x00000000} }, /* 1      */
/**/ __mpexp_two      = {{0x40000000, 0x00000000} }, /* 2      */
/**/ __mpexp_half     = {{0x3fe00000, 0x00000000} }; /* 1/2    */

#else
#ifdef LITTLE_ENDI
  const number
	__mpexp_twomm1[33] = {                     /* 2**-m1 */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x3ee00000} }, /* 2**-17 */
/**/                  {{0x00000000, 0x3e800000} }, /* 2**-23 */
/**/                  {{0x00000000, 0x3e800000} }, /* 2**-23 */
/**/                  {{0x00000000, 0x3e300000} }, /* 2**-28 */
/**/                  {{0x00000000, 0x3e400000} }, /* 2**-27 */
/**/                  {{0x00000000, 0x3d900000} }, /* 2**-38 */
/**/                  {{0x00000000, 0x3d500000} }, /* 2**-42 */
/**/                  {{0x00000000, 0x3d800000} }, /* 2**-39 */
/**/                  {{0x00000000, 0x3d400000} }, /* 2**-43 */
/**/                  {{0x00000000, 0x3d000000} }, /* 2**-47 */
/**/                  {{0x00000000, 0x3d400000} }, /* 2**-43 */
/**/                  {{0x00000000, 0x3d000000} }, /* 2**-47 */
/**/                  {{0x00000000, 0x3cd00000} }, /* 2**-50 */
/**/                  {{0x00000000, 0x3c900000} }, /* 2**-54 */
/**/                  {{0x00000000, 0x3c600000} }, /* 2**-57 */
/**/                  {{0x00000000, 0x3c300000} }, /* 2**-60 */
/**/                  {{0x00000000, 0x3bf00000} }, /* 2**-64 */
/**/                  {{0x00000000, 0x3bc00000} }, /* 2**-67 */
/**/                  {{0x00000000, 0x3b800000} }, /* 2**-71 */
/**/                  {{0x00000000, 0x3b500000} }, /* 2**-74 */
/**/                  {{0x00000000, 0x3bb00000} }, /* 2**-68 */
/**/                  {{0x00000000, 0x3b800000} }, /* 2**-71 */
/**/                  {{0x00000000, 0x3b500000} }, /* 2**-74 */
/**/                  {{0x00000000, 0x3b200000} }, /* 2**-77 */
/**/                  {{0x00000000, 0x3b900000} }, /* 2**-70 */
/**/                  {{0x00000000, 0x3b600000} }, /* 2**-73 */
/**/                  {{0x00000000, 0x3b300000} }, /* 2**-76 */
/**/                  {{0x00000000, 0x3b100000} }, /* 2**-78 */
/**/                  {{0x00000000, 0x3ae00000} }, /* 2**-81 */
  };
  const number
	       __mpexp_nn[9]={                     /* n      */
/**/                  {{0x00000000, 0x00000000} }, /* 0      */
/**/                  {{0x00000000, 0x3ff00000} }, /* 1      */
/**/                  {{0x00000000, 0x40000000} }, /* 2      */
/**/                  {{0x00000000, 0x40080000} }, /* 3      */
/**/                  {{0x00000000, 0x40100000} }, /* 4      */
/**/                  {{0x00000000, 0x40140000} }, /* 5      */
/**/                  {{0x00000000, 0x40180000} }, /* 6      */
/**/                  {{0x00000000, 0x401c0000} }, /* 7      */
/**/                  {{0x00000000, 0x40200000} }, /* 8      */
  };

  const number
/**/ __mpexp_radix    = {{0x00000000, 0x41700000} }, /* 2**24  */
/**/ __mpexp_radixi   = {{0x00000000, 0x3e700000} }, /* 2**-24 */
/**/ __mpexp_zero     = {{0x00000000, 0x00000000} }, /* 0      */
/**/ __mpexp_one      = {{0x00000000, 0x3ff00000} }, /* 1      */
/**/ __mpexp_two      = {{0x00000000, 0x40000000} }, /* 2      */
/**/ __mpexp_half     = {{0x00000000, 0x3fe00000} }; /* 1/2    */

#endif
#endif
#endif

#define  RADIX     __mpexp_radix.d
#define  RADIXI    __mpexp_radixi.d
#define  ZERO      __mpexp_zero.d
#define  ONE       __mpexp_one.d
#define  TWO       __mpexp_two.d
#define  HALF      __mpexp_half.d

#endif
