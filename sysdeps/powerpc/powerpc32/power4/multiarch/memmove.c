/* Multiple versions of memmove. PowerPC32 version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#if defined SHARED && IS_IN (libc)
/* Redefine memmove so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
# include <string.h>
# include "init-arch.h"

extern __typeof (memmove) __memmove_ppc attribute_hidden;
extern __typeof (memmove) __memmove_power7 attribute_hidden;

libc_ifunc (memmove,
            (hwcap & PPC_FEATURE_HAS_VSX)
            ? __memmove_power7
            : __memmove_ppc);
#else
# include <string/memmove.c>
#endif
