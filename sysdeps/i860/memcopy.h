/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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

#include <sysdeps/generic/memcopy.h>

#if 0
#undef	MERGE
/* In order to make this work properly, an 's' constraint need to be added
   to tm-i860.h, to mean the SC register.  */
#define MERGE(w0, sh_1, w1, sh_2)					      \
  ({									      \
    unsigned int __merge;						      \
    asm("shrd %2,%1,%0" :						      \
	"=r" (__merge) :						      \
	"r" (w0), "r" (w1), "s" (sh_1));				      \
    __merge;								      \
  })
#endif
