/* 64-bit multiplication and division libgcc routines for 32-bit PowerPC
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#define _DIVDI3_C
#include <sysdeps/wordsize-32/divdi3.c>

INTDEF (__udivdi3);
INTDEF (__moddi3);
INTDEF (__umoddi3);

#ifdef HAVE_DOT_HIDDEN
asm (".hidden __divdi3");
asm (".hidden __udivdi3");
asm (".hidden __moddi3");
asm (".hidden __umoddi3");
#endif

#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_2_6)

symbol_version (INTUSE (__divdi3), __divdi3, GLIBC_2.0);
symbol_version (INTUSE (__udivdi3), __udivdi3, GLIBC_2.0);
symbol_version (INTUSE (__moddi3), __moddi3, GLIBC_2.0);
symbol_version (INTUSE (__umoddi3), __umoddi3, GLIBC_2.0);

#endif
