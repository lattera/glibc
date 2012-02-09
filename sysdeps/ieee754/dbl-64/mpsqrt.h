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
/* MODULE_NAME:mpatan.h                                           */
/*                                                                */
/* common data and variables prototype and definition             */
/******************************************************************/

#ifndef MPSQRT_H
#define MPSQRT_H

extern const number __mpsqrt_one attribute_hidden;
extern const number __mpsqrt_halfrad attribute_hidden;
extern const int __mpsqrt_mp[33] attribute_hidden;


#ifndef AVOID_MPSQRT_H
#ifdef BIG_ENDI
  const number
/**/ __mpsqrt_one            = {{0x3ff00000, 0x00000000} }, /* 1      */
/**/ __mpsqrt_halfrad        = {{0x41600000, 0x00000000} }; /* 2**23  */

#else
#ifdef LITTLE_ENDI
  const number
/**/ __mpsqrt_one            = {{0x00000000, 0x3ff00000} }, /* 1      */
/**/ __mpsqrt_halfrad        = {{0x00000000, 0x41600000} }; /* 2**23  */

#endif
#endif

  const int __mpsqrt_mp[33] = {0,0,0,0,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,
			     4,4,4,4,4,4,4,4,4};
#endif

#define  ONE       __mpsqrt_one.d
#define  HALFRAD   __mpsqrt_halfrad.d

#endif
