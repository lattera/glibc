/* Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdint.h>

#define TEST_STACK_ALIGN() \
  ({									     \
    /* Altivec __vector int etc. needs 16byte aligned stack.		     \
       Instead of using altivec.h here, use aligned attribute instead.  */   \
    struct _S								     \
      {									     \
        int _i __attribute__((aligned (16)));				     \
	int _j[3];							     \
      } _s = { ._i = 18, ._j[0] = 19, ._j[1] = 20, ._j[2] = 21 };	     \
    double _d = 12.0;							     \
    long double _ld = 15.0;						     \
    int _ret = 0;							     \
    printf ("__vector int:  { %d, %d, %d, %d } %p %zu\n", _s._i, _s._j[0],   \
            _s._j[1], _s._j[2], &_s, __alignof (_s));			     \
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
