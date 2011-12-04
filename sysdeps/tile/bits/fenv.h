/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#ifndef _FENV_H
# error "Never use <bits/fenv.h> directly; include <fenv.h> instead."
#endif

/* The TILE-Gx hardware does not provide floating-point exception
   handling, and TILEPro does not support any floating-point operations.  */
#define FE_ALL_EXCEPT 0

/* TILE-Gx supports only round-to-nearest.  The software
   floating-point support also acts this way.  */
enum
  {
    FE_TONEAREST = 1,
#define FE_TONEAREST	FE_TONEAREST
  };

/* Type representing exception flags (if there were any).  */
typedef unsigned int fexcept_t;

/* Type representing floating-point environment.  */
typedef unsigned int fenv_t;

/* If the default argument is used we use this value.  */
#define FE_DFL_ENV	((__const fenv_t *) -1l)
