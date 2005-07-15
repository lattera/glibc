/* Checking macros for wchar functions.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

#ifndef _WCHAR_H
# error "Never include <bits/wchar.h> directly; use <wchar.h> instead."
#endif


extern wchar_t *__wmemcpy_chk (wchar_t *__restrict __s1,
			       __const wchar_t *__restrict __s2, size_t __n,
			       size_t __ns1) __THROW;
extern wchar_t *__REDIRECT (__wmemcpy_alias, (wchar_t *__restrict __s1,
					      __const wchar_t *__restrict __s2,
					      size_t __n), wmemcpy) __THROW;

extern __always_inline wchar_t *
wmemcpy (wchar_t *__restrict __s1, __const wchar_t *__restrict __s2,
	 size_t __n)
{
  if (__bos0 (__s1) != (size_t) -1)
    return __wmemcpy_chk (__s1, __s2, __n, __bos0 (__s1) / sizeof (wchar_t));
  return __wmemcpy_alias (__s1, __s2, __n);
}


extern wchar_t *__wmemmove_chk (wchar_t *__s1, __const wchar_t *__s2,
				size_t __n, size_t __ns1) __THROW;
extern wchar_t *__REDIRECT (__wmemmove_alias, (wchar_t *__s1,
					       __const wchar_t *__s2,
					       size_t __n), wmemmove) __THROW;

extern __always_inline wchar_t *
wmemmove (wchar_t *__restrict __s1, __const wchar_t *__restrict __s2,
	  size_t __n)
{
  if (__bos0 (__s1) != (size_t) -1)
    return __wmemmove_chk (__s1, __s2, __n, __bos0 (__s1) / sizeof (wchar_t));
  return __wmemmove_alias (__s1, __s2, __n);
}


#ifdef __USE_GNU
extern wchar_t *__wmempcpy_chk (wchar_t *__restrict __s1,
				__const wchar_t *__restrict __s2, size_t __n,
				size_t __ns1) __THROW;
extern wchar_t *__REDIRECT (__wmempcpy_alias,
			    (wchar_t *__restrict __s1,
			     __const wchar_t *__restrict __s2,
			     size_t __n), wmempcpy) __THROW;

extern __always_inline wchar_t *
wmempcpy (wchar_t *__restrict __s1, __const wchar_t *__restrict __s2,
	  size_t __n)
{
  if (__bos0 (__s1) != (size_t) -1)
    return __wmempcpy_chk (__s1, __s2, __n, __bos0 (__s1) / sizeof (wchar_t));
  return __wmempcpy_alias (__s1, __s2, __n);
}
#endif


extern wchar_t *__wmemset_chk (wchar_t *__s, wchar_t __c, size_t __n,
			       size_t __ns) __THROW;
extern wchar_t *__REDIRECT (__wmemset_alias, (wchar_t *__s, wchar_t __c,
					      size_t __n), wmemset) __THROW;

extern __always_inline wchar_t *
wmemset (wchar_t *__restrict __s, wchar_t __c, size_t __n)
{
  if (__bos0 (__s) != (size_t) -1)
    return __wmemset_chk (__s, __c, __n, __bos0 (__s) / sizeof (wchar_t));
  return __wmemset_alias (__s, __c, __n);
}


extern wchar_t *__wcscpy_chk (wchar_t *__restrict __dest,
			      __const wchar_t *__restrict __src,
			      size_t __n) __THROW;
extern wchar_t *__REDIRECT (__wcscpy_alias,
			    (wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src), wcscpy)
  __THROW;

extern __always_inline wchar_t *
wcscpy (wchar_t *__dest, __const wchar_t *__src)
{
  if (__bos (__dest) != (size_t) -1)
    return __wcscpy_chk (__dest, __src, __bos (__dest) / sizeof (wchar_t));
  return __wcscpy_alias (__dest, __src);
}


extern wchar_t *__wcpcpy_chk (wchar_t *__dest, __const wchar_t *__src,
			      size_t __destlen) __THROW;
extern wchar_t *__REDIRECT (__wcpcpy_alias, (wchar_t *__dest,
					     __const wchar_t *__src), wcpcpy)
  __THROW;

extern __always_inline wchar_t *
wcpcpy (wchar_t *__dest, __const wchar_t *__src)
{
  if (__bos (__dest) != (size_t) -1)
    return __wcpcpy_chk (__dest, __src, __bos (__dest) / sizeof (wchar_t));
  return __wcpcpy_alias (__dest, __src);
}


extern wchar_t *__wcsncpy_chk (wchar_t *__restrict __dest,
			       __const wchar_t *__restrict __src, size_t __n,
			       size_t __destlen) __THROW;
extern wchar_t *__REDIRECT (__wcsncpy_alias,
			    (wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src, size_t __n),
			    wcsncpy) __THROW;

extern __always_inline wchar_t *
wcsncpy (wchar_t *__dest, __const wchar_t *__src, size_t __n)
{
  if (__bos (__dest) != (size_t) -1
      && (!__builtin_constant_p (__n) || __bos (__dest) >= __n))
    return __wcsncpy_chk (__dest, __src, __n,
			  __bos (__dest) / sizeof (wchar_t));
  return __wcsncpy_alias (__dest, __src, __n);
}


extern wchar_t *__wcpncpy_chk (wchar_t *__restrict __dest,
			       __const wchar_t *__restrict __src, size_t __n,
			       size_t __destlen) __THROW;
extern wchar_t *__REDIRECT (__wcpncpy_alias,
			    (wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src, size_t __n),
			    wcpncpy) __THROW;

extern __always_inline wchar_t *
wcpncpy (wchar_t *__dest, __const wchar_t *__src, size_t __n)
{
  if (__bos (__dest) != (size_t) -1
      && (!__builtin_constant_p (__n) || __bos (__dest) >= __n))
    return __wcpncpy_chk (__dest, __src, __n,
			  __bos (__dest) / sizeof (wchar_t));
  return __wcpncpy_alias (__dest, __src, __n);
}


extern wchar_t *__wcscat_chk (wchar_t *__restrict __dest,
			      __const wchar_t *__restrict __src,
			      size_t __destlen) __THROW;
extern wchar_t *__REDIRECT (__wcscat_alias,
			    (wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src), wcscat)
  __THROW;

extern __always_inline wchar_t *
wcscat (wchar_t *__dest, __const wchar_t *__src)
{
  if (__bos (__dest) != (size_t) -1)
    return __wcscat_chk (__dest, __src, __bos (__dest) / sizeof (wchar_t));
  return __wcscat_alias (__dest, __src);
}


extern wchar_t *__wcsncat_chk (wchar_t *__restrict __dest,
			       __const wchar_t *__restrict __src,
			       size_t __n, size_t __destlen) __THROW;
extern wchar_t *__REDIRECT (__wcsncat_alias,
			    (wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src, size_t __n),
			    wcsncat) __THROW;

extern __always_inline wchar_t *
wcsncat (wchar_t *__dest, __const wchar_t *__src, size_t __n)
{
  if (__bos (__dest) != (size_t) -1)
    return __wcsncat_chk (__dest, __src, __n,
			  __bos (__dest) / sizeof (wchar_t));
  return __wcsncat_alias (__dest, __src, __n);
}


extern int __swprintf_chk (wchar_t *__restrict __s, size_t __n,
			   int __flag, size_t __s_len,
			   __const wchar_t *__restrict __format, ...)
     __THROW /* __attribute__ ((__format__ (__wprintf__, 5, 6))) */;

/* XXX We might want to have support in gcc for swprintf.  */
#define swprintf(s, n, format, ...) \
  (__bos (s) != (size_t) -1 || __USE_FORTIFY_LEVEL > 1			      \
   ? __swprintf_chk (s, n, __USE_FORTIFY_LEVEL - 1, __bos (s), format,	      \
		     __VA_ARGS__)					      \
   : swprintf (s, n, format, __VA_ARGS__))


extern int __vswprintf_chk (wchar_t *__restrict __s, size_t __n,
			    int __flag, size_t __s_len,
			    __const wchar_t *__restrict __format,
			    __gnuc_va_list __arg)
     __THROW /* __attribute__ ((__format__ (__wprintf__, 5, 0))) */;
extern int __REDIRECT (__vswprintf_alias,
		       (wchar_t *__restrict __s, size_t __n,
			__const wchar_t *__restrict __format,
			__gnuc_va_list __arg), vswprintf)
     __THROW /* __attribute__ ((__format__ (__wprintf__, 3, 0))) */;


extern __always_inline int
vswprintf (wchar_t *__s, size_t __n, __const wchar_t *__format,
	   __gnuc_va_list __arg)
{
  if (__bos (__s) != (size_t) -1 || __USE_FORTIFY_LEVEL > 1)
    return __vswprintf_chk (__s, __n, __USE_FORTIFY_LEVEL - 1, __bos (__s),
			    __format, __arg);
  return vswprintf (__s, __n, __format, __arg);
}


#if __USE_FORTIFY_LEVEL > 1

extern int __fwprintf_chk (FILE *__restrict __stream, int __flag,
			   __const wchar_t *__restrict __format, ...);
extern int __wprintf_chk (int __flag, __const wchar_t *__restrict __format,
			  ...);
extern int __vfwprintf_chk (FILE *__restrict __stream, int __flag,
			    __const wchar_t *__restrict __format,
			    _G_va_list __ap);
extern int __vwprintf_chk (int __flag, __const wchar_t *__restrict __format,
			   _G_va_list __ap);

# define wprintf(...) \
  __wprintf_chk (__USE_FORTIFY_LEVEL - 1, __VA_ARGS__)
# define fwprintf(stream, ...) \
  __fwprintf_chk (stream, __USE_FORTIFY_LEVEL - 1, __VA_ARGS__)
# define vwprintf(format, ap) \
  __vwprintf_chk (__USE_FORTIFY_LEVEL - 1, format, ap)
# define vfwprintf(stream, format, ap) \
  __vfwprintf_chk (stream, __USE_FORTIFY_LEVEL - 1, format, ap)

#endif

extern wchar_t *__fgetws_chk (wchar_t *__restrict __s, size_t __size, int __n,
			      FILE *__restrict __stream) __wur;
extern wchar_t *__REDIRECT (__fgetws_alias,
			    (wchar_t *__restrict __s, int __n,
			     FILE *__restrict __stream), fgetws) __wur;

extern __always_inline __wur wchar_t *
fgetws (wchar_t *__restrict __s, int __n, FILE *__restrict __stream)
{
  if (__bos (__s) != (size_t) -1
      && (!__builtin_constant_p (__n) || (size_t) __n > __bos (__s)))
    return __fgetws_chk (__s, __bos (__s), __n, __stream);
  return __fgetws_alias (__s, __n, __stream);
}

#ifdef __USE_GNU
extern wchar_t *__fgetws_unlocked_chk (wchar_t *__restrict __s, size_t __size,
				       int __n, FILE *__restrict __stream)
  __wur;
extern wchar_t *__REDIRECT (__fgetws_unlocked_alias,
			    (wchar_t *__restrict __s, int __n,
			     FILE *__restrict __stream), fgetws_unlocked)
  __wur;

extern __always_inline __wur wchar_t *
fgetws_unlocked (wchar_t *__restrict __s, int __n, FILE *__restrict __stream)
{
  if (__bos (__s) != (size_t) -1
      && (!__builtin_constant_p (__n) || (size_t) __n > __bos (__s)))
    return __fgetws_unlocked_chk (__s, __bos (__s), __n, __stream);
  return __fgetws_unlocked_alias (__s, __n, __stream);
}
#endif
