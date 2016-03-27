/* Copyright (C) 1999-2016 Free Software Foundation, Inc.
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

/*  <bits/string.h> and <bits/string2.h> declare some extern inline
    functions.  These functions are declared additionally here if
    inlining is not possible.  */

#undef __USE_STRING_INLINES
#define __USE_STRING_INLINES
#define _FORCE_INLINES
#define __STRING_INLINE /* empty */
#define __NO_INLINE__

#include <string.h>
#undef index
#undef rindex

#undef __NO_INLINE__
#include <bits/string.h>
#include <bits/string2.h>

#include "shlib-compat.h"

#if SHLIB_COMPAT (libc, GLIBC_2_1_1, GLIBC_2_24)
/* The inline functions are not used from GLIBC 2.24 and forward, however
   they are required to provide the symbols through string-inlines.c
   (if inlining is not possible for compatibility reasons).  */
size_t
__old_strcspn_c1 (const char *__s, int __reject)
{
  size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject)
    ++__result;
  return __result;
}
compat_symbol (libc, __old_strcspn_c1, __strcspn_c1, GLIBC_2_1_1);

size_t
__old_strcspn_c2 (const char *__s, int __reject1, int __reject2)
{
  size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject1
	 && __s[__result] != __reject2)
    ++__result;
  return __result;
}
compat_symbol (libc, __old_strcspn_c2, __strcspn_c2, GLIBC_2_1_1);

size_t
__old_strcspn_c3 (const char *__s, int __reject1, int __reject2,
	      int __reject3)
{
  size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject1
	 && __s[__result] != __reject2 && __s[__result] != __reject3)
    ++__result;
  return __result;
}
compat_symbol (libc, __old_strcspn_c3, __strcspn_c3, GLIBC_2_1_1);

size_t
__old_strspn_c1 (const char *__s, int __accept)
{
  size_t __result = 0;
  /* Please note that __accept never can be '\0'.  */
  while (__s[__result] == __accept)
    ++__result;
  return __result;
}
compat_symbol (libc, __old_strspn_c1, __strspn_c1, GLIBC_2_1_1);

size_t
__old_strspn_c2 (const char *__s, int __accept1, int __accept2)
{
  size_t __result = 0;
  /* Please note that __accept1 and __accept2 never can be '\0'.  */
  while (__s[__result] == __accept1 || __s[__result] == __accept2)
    ++__result;
  return __result;
}
compat_symbol (libc, __old_strspn_c2, __strspn_c2, GLIBC_2_1_1);

size_t
__old_strspn_c3 (const char *__s, int __accept1, int __accept2,
		 int __accept3)
{
  size_t __result = 0;
  /* Please note that __accept1 to __accept3 never can be '\0'.  */
  while (__s[__result] == __accept1 || __s[__result] == __accept2
	 || __s[__result] == __accept3)
    ++__result;
  return __result;
}
compat_symbol (libc, __old_strspn_c3, __strspn_c3, GLIBC_2_1_1);

char *
__strpbrk_c2 (const char *__s, int __accept1, int __accept2)
{
  /* Please note that __accept1 and __accept2 never can be '\0'.  */
  while (*__s != '\0' && *__s != __accept1 && *__s != __accept2)
    ++__s;
  return *__s == '\0' ? NULL : (char *) (size_t) __s;
}
compat_symbol (libc, __old_strpbrk_c2, __strpbrk_c2, GLIBC_2_1_1);

char *
__strpbrk_c3 (const char *__s, int __accept1, int __accept2, int __accept3)
{
  /* Please note that __accept1 to __accept3 never can be '\0'.  */
  while (*__s != '\0' && *__s != __accept1 && *__s != __accept2
	 && *__s != __accept3)
    ++__s;
  return *__s == '\0' ? NULL : (char *) (size_t) __s;
}
compat_symbol (libc, __old_strpbrk_c3, __strpbrk_c3, GLIBC_2_1_1);

#endif
