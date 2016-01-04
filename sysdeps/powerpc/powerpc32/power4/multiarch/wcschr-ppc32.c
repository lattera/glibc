/* Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#include <wchar.h>

#if IS_IN (libc)
# undef libc_hidden_weak
# define libc_hidden_weak(name)

# undef weak_alias
# undef libc_hidden_def

# ifdef SHARED
#  define libc_hidden_def(name)  \
    __hidden_ver1 (__wcschr_ppc, __GI_wcschr, __wcschr_ppc); \
    strong_alias (__wcschr_ppc, __wcschr_ppc_1); \
    __hidden_ver1 (__wcschr_ppc_1, __GI___wcschr, __wcschr_ppc_1);
#  define weak_alias(name,alias)
# else
#  define weak_alias(name, alias) \
    _weak_alias(__wcschr_ppc, __wcschr)
#  define libc_hidden_def(name)
# endif /* SHARED  */
#endif

extern __typeof (wcschr) __wcschr_ppc;

#define WCSCHR  __wcschr_ppc
#include <wcsmbs/wcschr.c>
