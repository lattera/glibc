/* Optimized, inlined string functions.  ARM version.
   Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
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

#ifndef _STRING_H
# error "Never use <bits/string.h> directly; include <string.h> instead."
#endif

/* We must defeat the generic optimized versions of these functions in
   <bits/string2.h> since they don't work on the ARM.  This is because
   the games they play with the __STRING2_COPY_ARR# structures fail
   when structs are always 32-bit aligned.
   XXX Should provide suitably optimal replacements.  */
#define _HAVE_STRING_ARCH_strcpy 1
#define _HAVE_STRING_ARCH_stpcpy 1
#define _HAVE_STRING_ARCH_mempcpy 1
