/* Macros to test for CPU features on ARM.  Generic ARM version.
   Copyright (C) 2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _ARM_ARM_FEATURES_H
#define _ARM_ARM_FEATURES_H 1

/* An OS-specific arm-features.h file should define ARM_HAVE_VFP to
   an appropriate expression for testing at runtime whether the VFP
   hardware is present.  We'll then redefine it to a constant if we
   know at compile time that we can assume VFP.  */

#ifndef __SOFTFP__
/* The compiler is generating VFP instructions, so we're already
   assuming the hardware exists.  */
# undef ARM_HAVE_VFP
# define ARM_HAVE_VFP	1
#endif

/* An OS-specific arm-features.h file may define ARM_ASSUME_NO_IWMMXT
   to indicate at compile time that iWMMXt hardware is never present
   at runtime (or that we never care about its state) and so need not
   be checked for.  */

#endif  /* arm-features.h */
