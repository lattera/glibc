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

#if SHLIB_COMPAT(libc, GLIBC_2_0, GLIBC_2_2_6)

extern int64_t __ashldi3 (int64_t, int32_t);
int64_t INTUSE (__ashldi3) (int64_t u, int32_t b)
{
  return __ashldi3 (u, b);
}
symbol_version (INTUSE (__ashldi3), __ashldi3, GLIBC_2.0);


extern int64_t __ashrdi3 (int64_t, int32_t);
int64_t INTUSE (__ashrdi3) (int64_t u, int32_t b)
{
  return __ashrdi3 (u, b);
}
symbol_version (INTUSE (__ashrdi3), __ashrdi3, GLIBC_2.0);


extern int64_t __lshrdi3 (int64_t, int32_t);
int64_t INTUSE (__lshrdi3) (int64_t u, int32_t b)
{
  return __lshrdi3 (u, b);
}
symbol_version (INTUSE (__lshrdi3), __lshrdi3, GLIBC_2.0);


extern int32_t __cmpdi2 (int64_t, int64_t);
int32_t INTUSE (__cmpdi2) (int64_t u, int64_t v)
{
  return __cmpdi2 (u, v);
}
symbol_version (INTUSE (__cmpdi2), __cmpdi2, GLIBC_2.0);


extern int32_t __ucmpdi2 (int64_t, int64_t);
int32_t INTUSE (__ucmpdi2) (int64_t u, int64_t v)
{
  return __ucmpdi2 (u, v);
}
symbol_version (INTUSE (__ucmpdi2), __ucmpdi2, GLIBC_2.0);


extern int64_t __fixdfdi (double);
int64_t INTUSE (__fixdfdi) (double d)
{
  return __fixdfdi (d);
}
symbol_version (INTUSE (__fixdfdi), __fixdfdi, GLIBC_2.0);


extern int64_t __fixunsdfdi (double);
int64_t INTUSE (__fixunsdfdi) (double d)
{
  return __fixunsdfdi (d);
}
symbol_version (INTUSE (__fixunsdfdi), __fixunsdfdi, GLIBC_2.0);


extern int64_t __fixsfdi (float);
int64_t INTUSE (__fixsfdi) (float d)
{
  return __fixsfdi (d);
}
symbol_version (INTUSE (__fixsfdi), __fixsfdi, GLIBC_2.0);


extern int64_t __fixunssfdi (float);
int64_t INTUSE (__fixunssfdi) (float d)
{
  return __fixunssfdi (d);
}
symbol_version (INTUSE (__fixunssfdi), __fixunssfdi, GLIBC_2.0);


extern double __floatdidf (int64_t);
double INTUSE (__floatdidf) (int64_t u)
{
  return __floatdidf (u);
}
symbol_version (INTUSE (__floatdidf), __floatdidf, GLIBC_2.0);


extern float __floatdisf (int64_t);
float INTUSE (__floatdisf) (int64_t u)
{
  return __floatdisf (u);
}
symbol_version (INTUSE (__floatdisf), __floatdisf, GLIBC_2.0);

#endif
