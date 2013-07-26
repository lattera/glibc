/* AArch64 definitions for profiling support.
   Copyright (C) 1996-2013 Free Software Foundation, Inc.
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

/* GCC version 2 gives us a perfect magical function to get
   just the information we need:
     void *__builtin_return_address (unsigned int N)
   returns the return address of the frame N frames up.  */

#include <sysdep.h>

static void mcount_internal (u_long frompc, u_long selfpc);

#define _MCOUNT_DECL(frompc, selfpc) \
static inline void mcount_internal (u_long frompc, u_long selfpc)

#define MCOUNT \
void __mcount (void)							      \
{									      \
  mcount_internal ((u_long) RETURN_ADDRESS (1), (u_long) RETURN_ADDRESS (0)); \
}
