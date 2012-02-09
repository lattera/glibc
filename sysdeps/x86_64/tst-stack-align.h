/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdint.h>

#define TEST_STACK_ALIGN() \
  ({									     \
    /* AMD64 ABI mandates 16byte aligned stack.				     \
       Unfortunately, current GCC doesn't support __int128 or __float128     \
       types, so use aligned attribute instead.  */			     \
    struct _S								     \
      {									     \
        int _i __attribute__((aligned (16)));				     \
	int _pad[3];							     \
      } _s = { ._i = 18 };						     \
    double _d = 12.0;							     \
    long double _ld = 15.0;						     \
    int _ret = 0;							     \
    printf ("__int128:  %d %p %zu\n", _s._i, &_s, __alignof (_s));	     \
    if ((((uintptr_t) &_s) & (__alignof (_s) - 1)) != 0)		     \
      _ret = 1;								     \
									     \
    printf ("double:  %g %p %zu\n", _d, &_d, __alignof (double));	     \
    if ((((uintptr_t) &_d) & (__alignof (double) - 1)) != 0)		     \
      _ret = 1;								     \
									     \
    printf ("ldouble: %Lg %p %zu\n", _ld, &_ld, __alignof (long double));    \
    if ((((uintptr_t) &_ld) & (__alignof (long double) - 1)) != 0)	     \
      _ret = 1;								     \
    _ret;								     \
    })
