/* AltiVec (new) version of setjmp for PowerPC.
   Copyright (C) 1995-97, 1999-2002, 2004 Free Software Foundation, Inc.
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

#include <libc-symbols.h>
#include <rtld-global-offsets.h>
#include <shlib-compat.h>

#if defined NOT_IN_libc
/* Build a non-versioned object for rtld-*.  */
# include "setjmp-common.S"

#else /* !NOT_IN_libc */
/* Build a versioned object for libc.  */
default_symbol_version (__vmxsetjmp, setjmp, GLIBC_2.3.4)
default_symbol_version (__vmx_setjmp,_setjmp,GLIBC_2.3.4)
default_symbol_version (__vmx__sigsetjmp,__sigsetjmp,GLIBC_2.3.4)
# define setjmp __vmxsetjmp
# define _setjmp __vmx_setjmp
# define __sigsetjmp __vmx__sigsetjmp
# define __sigjmp_save __vmx__sigjmp_save
# include "setjmp-common.S"
strong_alias (__vmxsetjmp, __vmx__setjmp)
strong_alias (__vmx__sigsetjmp, __setjmp)

# if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_3, GLIBC_2_3_4)
#  undef setjmp
#  undef _setjmp
#  undef __sigsetjmp
#  undef __sigjmp_save
#  undef JB_SIZE
#  define __NO_VMX__
symbol_version (__novmxsetjmp, setjmp, GLIBC_2.3)
symbol_version (__novmx_setjmp,_setjmp,GLIBC_2.3);
symbol_version (__novmx__sigsetjmp,__sigsetjmp,GLIBC_2.3)
#  define setjmp __novmxsetjmp
#  define _setjmp __novmx_setjmp
#  define __sigsetjmp __novmx__sigsetjmp
#  define __sigjmp_save __novmx__sigjmp_save
#  include "setjmp-common.S"
strong_alias (__novmxsetjmp, __novmx__setjmp)
# endif
#endif /* !NOT_IN_libc */
