
/*
 * IBM Accurate Mathematical Library
 * Copyright (c) International Business Machines Corp., 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU  Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/******************************************************************/
/*                                                                */
/* MODULE_NAME:mpatan.h                                           */
/*                                                                */
/* common data and variables prototype and definition             */
/******************************************************************/

#ifndef MPSQRT_H
#define MPSQRT_H

#ifdef BIG_ENDI
  static const number
/**/ one            = {{0x3ff00000, 0x00000000} }, /* 1      */
/**/ halfrad        = {{0x41600000, 0x00000000} }; /* 2**23  */

#else
#ifdef LITTLE_ENDI
  static const number
/**/ one            = {{0x00000000, 0x3ff00000} }, /* 1      */
/**/ halfrad        = {{0x00000000, 0x41600000} }; /* 2**23  */

#endif
#endif

#define  ONE       one.d
#define  HALFRAD   halfrad.d

  static const int mp[33] = {0,0,0,0,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,
                             4,4,4,4,4,4,4,4,4};

#endif
