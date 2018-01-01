/* Signal number definitions.  BSD version.
   Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#ifndef _BITS_SIGNUM_H
#define _BITS_SIGNUM_H 1

#ifndef _SIGNAL_H
#error "Never include <bits/signum.h> directly; use <signal.h> instead."
#endif

#include <bits/signum-generic.h>

/* Adjustments and additions to the signal number constants for
   4.2 or 4.3 BSD-derived Unix systems.  */

#define SIGEMT		 7	/* Emulator trap (4.2 BSD).  */
#define SIGINFO		29	/* Information request (4.4 BSD).  */
#define SIGLOST		32	/* Resource lost (Sun); server died (GNU).  */

#endif /* bits/signum.h.  */
