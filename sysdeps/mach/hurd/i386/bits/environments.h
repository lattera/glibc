/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _UNISTD_H
# error "Never include this file directly.  Use <unistd.h> instead"
#endif

/* By default we have 32-bit wide `int', `long int', pointers and `off_t'.  */
#define _XBS5_ILP32_OFF32	1

/* The Hurd does not yet implement an environment with the above size
   but an 64-bit size `off_t'.  */
#define _XBS5_ILP32_OFFBIG	-1

/* We can never provide environments with 64-bit wide pointers.  */
#define _XBS5_LP64_OFF64	-1
#define _XBS5_LPBIG_OFFBIG	-1
