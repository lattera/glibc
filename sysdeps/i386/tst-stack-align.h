/* Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

typedef struct { int i[4]; } int_al16 __attribute__((aligned (16)));

#define TEST_STACK_ALIGN() \
  ({									     \
    int_al16 _m;							     \
    double _d = 12.0;							     \
    long double _ld = 15.0;						     \
    int _ret = 0;							     \
    printf ("int_al16:  %p %zu\n", &_m, __alignof (int_al16));		     \
    if ((((uintptr_t) &_m) & (__alignof (int_al16) - 1)) != 0)		     \
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
