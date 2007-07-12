/* pre-.hidden libgcc compatibility
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


#include <stdint.h>
#include <shlib-compat.h>

#if SHLIB_COMPAT(libc, GLIBC_2_2, GLIBC_2_2_6)

typedef int int128_t __attribute__((__mode__(TI)));

extern long double __divtf3 (long double, long double) attribute_hidden;
long double INTUSE (__divtf3) (long double x, long double y)
{
  return __divtf3 (x, y);
}
symbol_version (INTUSE (__divtf3), __divtf3, GLIBC_2.2);

extern double __divdf3 (double, double) attribute_hidden;
double INTUSE (__divdf3) (double x, double y)
{
  return __divdf3 (x, y);
}
symbol_version (INTUSE (__divdf3), __divdf3, GLIBC_2.2);

extern float __divsf3 (float, float) attribute_hidden;
float INTUSE (__divsf3) (float x, float y)
{
  return __divsf3 (x, y);
}
symbol_version (INTUSE (__divsf3), __divsf3, GLIBC_2.2);

extern int64_t __divdi3 (int64_t, int64_t) attribute_hidden;
int64_t INTUSE (__divdi3) (int64_t x, int64_t y)
{
  return __divdi3 (x, y);
}
symbol_version (INTUSE (__divdi3), __divdi3, GLIBC_2.2);

extern int64_t __moddi3 (int64_t, int64_t) attribute_hidden;
int64_t INTUSE (__moddi3) (int64_t x, int64_t y)
{
  return __moddi3 (x, y);
}
symbol_version (INTUSE (__moddi3), __moddi3, GLIBC_2.2);

extern uint64_t __udivdi3 (uint64_t, uint64_t) attribute_hidden;
uint64_t INTUSE (__udivdi3) (uint64_t x, uint64_t y)
{
  return __udivdi3 (x, y);
}
symbol_version (INTUSE (__udivdi3), __udivdi3, GLIBC_2.2);

extern uint64_t __umoddi3 (uint64_t, uint64_t) attribute_hidden;
uint64_t INTUSE (__umoddi3) (uint64_t x, uint64_t y)
{
  return __umoddi3 (x, y);
}
symbol_version (INTUSE (__umoddi3), __umoddi3, GLIBC_2.2);

extern int128_t __multi3 (int128_t, int128_t) attribute_hidden;
int128_t INTUSE (__multi3) (int128_t x, int128_t y)
{
  return __multi3 (x, y);
}
symbol_version (INTUSE (__multi3), __multi3, GLIBC_2.2);

#endif
