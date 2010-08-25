/* Multiple versions of __memmove_chk.
   Copyright (C) 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <string.h>
#include "init-arch.h"

#define MEMMOVE_CHK __memmove_chk_sse2

extern __typeof (__memmove_chk) __memmove_chk_sse2 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_ssse3 attribute_hidden;
extern __typeof (__memmove_chk) __memmove_chk_ssse3_back attribute_hidden;

#include "debug/memmove_chk.c"

libc_ifunc (__memmove_chk,
	    HAS_SSSE3
	    ? (HAS_FAST_COPY_BACKWARD
	       ? __memmove_chk_ssse3_back : __memmove_chk_ssse3)
	    : __memmove_chk_sse2);
