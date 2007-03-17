/* Checking macros for stdio functions.
   Copyright (C) 2004, 2005, 2007 Free Software Foundation, Inc.
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

#ifndef _STDIO_H
# error "Never include <bits/stdio2.h> directly; use <stdio.h> instead."
#endif

extern int __sprintf_chk (char *__restrict __s, int __flag, size_t __slen,
			  __const char *__restrict __format, ...) __THROW;
extern int __vsprintf_chk (char *__restrict __s, int __flag, size_t __slen,
			   __const char *__restrict __format,
			   _G_va_list __ap) __THROW;

#define sprintf(str, ...) \
  __builtin___sprintf_chk (str, __USE_FORTIFY_LEVEL - 1, __bos (str), \
			   __VA_ARGS__)
#define vsprintf(str, fmt, ap) \
  __builtin___vsprintf_chk (str, __USE_FORTIFY_LEVEL - 1, __bos (str), fmt, ap)

#if defined __USE_BSD || defined __USE_ISOC99 || defined __USE_UNIX98

extern int __snprintf_chk (char *__restrict __s, size_t __n, int __flag,
			   size_t __slen, __const char *__restrict __format,
			   ...) __THROW;
extern int __vsnprintf_chk (char *__restrict __s, size_t __n, int __flag,
			    size_t __slen, __const char *__restrict __format,
			    _G_va_list __ap) __THROW;

# define snprintf(str, len, ...) \
  __builtin___snprintf_chk (str, len, __USE_FORTIFY_LEVEL - 1, __bos (str), \
			    __VA_ARGS__)
# define vsnprintf(str, len, fmt, ap) \
  __builtin___vsnprintf_chk (str, len, __USE_FORTIFY_LEVEL - 1, __bos (str), \
			     fmt, ap)

#endif

#if __USE_FORTIFY_LEVEL > 1

extern int __fprintf_chk (FILE *__restrict __stream, int __flag,
			  __const char *__restrict __format, ...);
extern int __printf_chk (int __flag, __const char *__restrict __format, ...);
extern int __vfprintf_chk (FILE *__restrict __stream, int __flag,
			   __const char *__restrict __format, _G_va_list __ap);
extern int __vprintf_chk (int __flag, __const char *__restrict __format,
			  _G_va_list __ap);

# define printf(...) \
  __printf_chk (__USE_FORTIFY_LEVEL - 1, __VA_ARGS__)
# define fprintf(stream, ...) \
  __fprintf_chk (stream, __USE_FORTIFY_LEVEL - 1, __VA_ARGS__)
# define vprintf(format, ap) \
  __vprintf_chk (__USE_FORTIFY_LEVEL - 1, format, ap)
# define vfprintf(stream, format, ap) \
  __vfprintf_chk (stream, __USE_FORTIFY_LEVEL - 1, format, ap)

#endif

extern char *__gets_chk (char *__str, size_t) __wur;
extern char *__REDIRECT (__gets_alias, (char *__str), gets) __wur;

__extern_always_inline __wur char *
gets (char *__str)
{
  if (__bos (__str) != (size_t) -1)
    return __gets_chk (__str, __bos (__str));
  return __gets_alias (__str);
}

extern char *__fgets_chk (char *__restrict __s, size_t __size, int __n,
			  FILE *__restrict __stream) __wur;
extern char *__REDIRECT (__fgets_alias,
			 (char *__restrict __s, int __n,
			  FILE *__restrict __stream), fgets) __wur;

__extern_always_inline __wur char *
fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
{
  if (__bos (__s) != (size_t) -1
      && (!__builtin_constant_p (__n) || (size_t) __n > __bos (__s)))
    return __fgets_chk (__s, __bos (__s), __n, __stream);
  return __fgets_alias (__s, __n, __stream);
}

#ifdef __USE_GNU
extern char *__fgets_unlocked_chk (char *__restrict __s, size_t __size,
				   int __n, FILE *__restrict __stream) __wur;
extern char *__REDIRECT (__fgets_unlocked_alias,
			 (char *__restrict __s, int __n,
			  FILE *__restrict __stream), fgets_unlocked) __wur;

__extern_always_inline __wur char *
fgets_unlocked (char *__restrict __s, int __n, FILE *__restrict __stream)
{
  if (__bos (__s) != (size_t) -1
      && (!__builtin_constant_p (__n) || (size_t) __n > __bos (__s)))
    return __fgets_unlocked_chk (__s, __bos (__s), __n, __stream);
  return __fgets_unlocked_alias (__s, __n, __stream);
}
#endif
