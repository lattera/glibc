/* __fxstat64 () implementation.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Hide the prototypes for __fxstat and _fxstat so that GCC will not
   complain about the different function signatures if they are aliased
   to  __fxstat64.  If XSTAT_IS_XSTAT64 is set to non-zero then the stat and
   stat64 structures have an identical layout but different type names.  */

#define __fxstat __fxstat_disable
#define _fxstat _fxstat_disable

#include <sysdeps/unix/sysv/linux/fxstat64.c>

#undef __fxstat
#undef _fxstat
#if XSTAT_IS_XSTAT64
weak_alias (__fxstat64, __fxstat)
weak_alias (__fxstat64, _fxstat)
hidden_ver (__fxstat64, __fxstat)
#endif
