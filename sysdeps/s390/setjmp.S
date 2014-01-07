/* Extendible version of setjmp for System z
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <libc-symbols.h>
#include <shlib-compat.h>

versioned_symbol (libc, __v2setjmp, setjmp, GLIBC_2_19)
versioned_symbol (libc, __v2_setjmp, _setjmp, GLIBC_2_19)
versioned_symbol (libc, __v2__setjmp, __setjmp, GLIBC_2_19)
versioned_symbol (libc, __v2__sigsetjmp, __sigsetjmp, GLIBC_2_19)
#define setjmp __v2setjmp
#define _setjmp __v2_setjmp
#define __setjmp __v2__setjmp
#define __sigsetjmp __v2__sigsetjmp
#define __sigjmp_save __v2__sigjmp_save

#include "setjmp-common.S"

#undef setjmp
#undef _setjmp
#undef __setjmp
#undef __sigsetjmp
#undef __sigjmp_save
libc_hidden_ver (__v2setjmp, setjmp)
libc_hidden_ver (__v2_setjmp, _setjmp)
libc_hidden_ver (__v2__setjmp, __setjmp)
libc_hidden_ver (__v2__sigsetjmp, __sigsetjmp)

#if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_19)
compat_symbol (libc, __v1setjmp, setjmp, GLIBC_2_0)
compat_symbol (libc, __v1_setjmp, _setjmp, GLIBC_2_0)
compat_symbol (libc, __v1__setjmp, __setjmp, GLIBC_2_0)
compat_symbol (libc, __v1__sigsetjmp, __sigsetjmp, GLIBC_2_0)
# define setjmp __v1setjmp
# define _setjmp __v1_setjmp
# define __setjmp __v1__setjmp
# define __sigsetjmp __v1__sigsetjmp
# define __sigjmp_save __v1__sigjmp_save
# define __V1_JMPBUF

# include "setjmp-common.S"

# undef setjmp
# undef _setjmp
# undef __setjmp
# undef __sigsetjmp
# undef __sigjmp_save

#endif /* if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_19) */
