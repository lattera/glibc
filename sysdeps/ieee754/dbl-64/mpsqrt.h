/*
 * IBM Accurate Mathematical Library
 * Written by International Business Machines Corp.
 * Copyright (C) 2001-2016 Free Software Foundation, Inc.
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

extern const int __mpsqrt_mp[33] attribute_hidden;


#ifndef AVOID_MPSQRT_H
  const int __mpsqrt_mp[33] = {0,0,0,0,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,
			     4,4,4,4,4,4,4,4,4};
#endif

#endif
