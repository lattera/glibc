/* Checking macros for wchar functions.
   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
extern wchar_t *__REDIRECT_NTH (__wmemcpy_alias,
				(wchar_t *__restrict __s1,
				 __const wchar_t *__restrict __s2, size_t __n),
				wmemcpy);

__extern_always_inline wchar_t *
__NTH (wmemcpy (wchar_t *__restrict __s1, __const wchar_t *__restrict __s2,
		size_t __n))
{
  if (__bos0 (__s1) != (size_t) -1)
    return __wmemcpy_chk (__s1, __s2, __n, __bos0 (__s1) / sizeof (wchar_t));
  return __wmemcpy_alias (__s1, __s2, __n);
}


extern wchar_t *__wmemmove_chk (wchar_t *__s1, __const wchar_t *__s2,
				size_t __n, size_t __ns1) __THROW;
extern wchar_t *__REDIRECT_NTH (__wmemmove_alias, (wchar_t *__s1,
						   __const wchar_t *__s2,
						   size_t __n), wmemmove);

__extern_always_inline wchar_t *
__NTH (wmemmove (wchar_t *__restrict __s1, __const wchar_t *__restrict __s2,
		 size_t __n))
{
  if (__bos0 (__s1) != (size_t) -1)
    return __wmemmove_chk (__s1, __s2, __n, __bos0 (__s1) / sizeof (wchar_t));
  return __wmemmove_alias (__s1, __s2, __n);
}


#ifdef __USE_GNU
extern wchar_t *__wmempcpy_chk (wchar_t *__restrict __s1,
				__const wchar_t *__restrict __s2, size_t __n,
				size_t __ns1) __THROW;
extern wchar_t *__REDIRECT_NTH (__wmempcpy_alias,
				(wchar_t *__restrict __s1,
				 __const wchar_t *__restrict __s2,
				 size_t __n), wmempcpy);

__extern_always_inline wchar_t *
__NTH (wmempcpy (wchar_t *__restrict __s1, __const wchar_t *__restrict __s2,
		 size_t __n))
{
  if (__bos0 (__s1) != (size_t) -1)
    return __wmempcpy_chk (__s1, __s2, __n, __bos0 (__s1) / sizeof (wchar_t));
  return __wmempcpy_alias (__s1, __s2, __n);
}
#endif


extern wchar_t *__wmemset_chk (wchar_t *__s, wchar_t __c, size_t __n,
			       size_t __ns) __THROW;
extern wchar_t *__REDIRECT_NTH (__wmemset_alias, (wchar_t *__s, wchar_t __c,
						  size_t __n), wmemset);

__extern_always_inline wchar_t *
__NTH (wmemset (wchar_t *__restrict __s, wchar_t __c, size_t __n))
{
  if (__bos0 (__s) != (size_t) -1)
    return __wmemset_chk (__s, __c, __n, __bos0 (__s) / sizeof (wchar_t));
  return __wmemset_alias (__s, __c, __n);
}


extern wchar_t *__wcscpy_chk (wchar_t *__restrict __dest,
			      __const wchar_t *__restrict __src,
			      size_t __n) __THROW;
extern wchar_t *__REDIRECT_NTH (__wcscpy_alias,
				(wchar_t *__restrict __dest,
				 __const wchar_t *__restrict __src), wcscpy);

__extern_always_inline wchar_t *
__NTH (wcscpy (wchar_t *__dest, __const wchar_t *__src))
{
  if (__bos (__dest) != (size_t) -1)
    return __wcscpy_chk (__dest, __src, __bos (__dest) / sizeof (wchar_t));
  return __wcscpy_alias (__dest, __src);
}


extern wchar_t *__wcpcpy_chk (wchar_t *__dest, __const wchar_t *__src,
			      size_t __destlen) __THROW;
extern wchar_t *__REDIRECT_NTH (__wcpcpy_alias, (wchar_t *__dest,
						 __const wchar_t *__src),
				wcpcpy);

__extern_always_inline wchar_t *
__NTH (wcpcpy (wchar_t *__dest, __const wchar_t *__src))
{
  if (__bos (__dest) != (size_t) -1)
    return __wcpcpy_chk (__dest, __src, __bos (__dest) / sizeof (wchar_t));
  return __wcpcpy_alias (__dest, __src);
}


extern wchar_t *__wcsncpy_chk (wchar_t *__restrict __dest,
			       __const wchar_t *__restrict __src, size_t __n,
			       size_t __destlen) __THROW;
extern wchar_t *__REDIRECT_NTH (__wcsncpy_alias,
				(wchar_t *__restrict __dest,
				 __const wchar_t *__restrict __src,
				 size_t __n), wcsncpy);

__extern_always_inline wchar_t *
__NTH (wcsncpy (wchar_t *__dest, __const wchar_t *__src, size_t __n))
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
extern wchar_t *__REDIRECT_NTH (__wcpncpy_alias,
				(wchar_t *__restrict __dest,
				 __const wchar_t *__restrict __src,
				 size_t __n), wcpncpy);

__extern_always_inline wchar_t *
__NTH (wcpncpy (wchar_t *__dest, __const wchar_t *__src, size_t __n))
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
extern wchar_t *__REDIRECT_NTH (__wcscat_alias,
				(wchar_t *__restrict __dest,
				 __const wchar_t *__restrict __src), wcscat);

__extern_always_inline wchar_t *
__NTH (wcscat (wchar_t *__dest, __const wchar_t *__src))
{
  if (__bos (__dest) != (size_t) -1)
    return __wcscat_chk (__dest, __src, __bos (__dest) / sizeof (wchar_t));
  return __wcscat_alias (__dest, __src);
}


extern wchar_t *__wcsncat_chk (wchar_t *__restrict __dest,
			       __const wchar_t *__restrict __src,
			       size_t __n, size_t __destlen) __THROW;
extern wchar_t *__REDIRECT_NTH (__wcsncat_alias,
				(wchar_t *__restrict __dest,
				 __const wchar_t *__restrict __src,
				 size_t __n), wcsncat);

__extern_always_inline wchar_t *
__NTH (wcsncat (wchar_t *__dest, __const wchar_t *__src, size_t __n))
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
#define swprintf(s, n, ...) \
  (__bos (s) != (size_t) -1 || __USE_FORTIFY_LEVEL > 1			      \
   ? __swprintf_chk (s, n, __USE_FORTIFY_LEVEL - 1, __bos (s), __VA_ARGS__)   \
   : swprintf (s, n, __VA_ARGS__))


extern int __vswprintf_chk (wchar_t *__restrict __s, size_t __n,
			    int __flag, size_t __s_len,
			    __const wchar_t *__restrict __format,
			    __gnuc_va_list __arg)
     __THROW /* __attribute__ ((__format__ (__wprintf__, 5, 0))) */;

#define vswprintf(s, n, fmt, ap) \
  (__bos (s) != (size_t) -1 || __USE_FORTIFY_LEVEL > 1			      \
   ? __vswprintf_chk (s, n, __USE_FORTIFY_LEVEL - 1, __bos (s), fmt, ap)      \
   : vswprintf (s, n, fmt, ap))


#if __USE_FORTIFY_LEVEL > 1

extern int __fwprintf_chk (__FILE *__restrict __stream, int __flag,
			   __const wchar_t *__restrict __format, ...);
extern int __wprintf_chk (int __flag, __const wchar_t *__restrict __format,
			  ...);
extern int __vfwprintf_chk (__FILE *__restrict __stream, int __flag,
			    __const wchar_t *__restrict __format,
			    __gnuc_va_list __ap);
extern int __vwprintf_chk (int __flag, __const wchar_t *__restrict __format,
			   __gnuc_va_list __ap);

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
			      __FILE *__restrict __stream) __wur;
extern wchar_t *__REDIRECT (__fgetws_alias,
			    (wchar_t *__restrict __s, int __n,
			     __FILE *__restrict __stream), fgetws) __wur;

__extern_always_inline __wur wchar_t *
fgetws (wchar_t *__restrict __s, int __n, __FILE *__restrict __stream)
{
  if (__bos (__s) != (size_t) -1
      && (!__builtin_constant_p (__n) || (size_t) __n > __bos (__s)))
    return __fgetws_chk (__s, __bos (__s), __n, __stream);
  return __fgetws_alias (__s, __n, __stream);
}

#ifdef __USE_GNU
extern wchar_t *__fgetws_unlocked_chk (wchar_t *__restrict __s, size_t __size,
				       int __n, __FILE *__restrict __stream)
  __wur;
extern wchar_t *__REDIRECT (__fgetws_unlocked_alias,
			    (wchar_t *__restrict __s, int __n,
			     __FILE *__restrict __stream), fgetws_unlocked)
  __wur;

__extern_always_inline __wur wchar_t *
fgetws_unlocked (wchar_t *__restrict __s, int __n, __FILE *__restrict __stream)
{
  if (__bos (__s) != (size_t) -1
      && (!__builtin_constant_p (__n) || (size_t) __n > __bos (__s)))
    return __fgetws_unlocked_chk (__s, __bos (__s), __n, __stream);
  return __fgetws_unlocked_alias (__s, __n, __stream);
}
#endif


extern size_t __wcrtomb_chk (char *__s, wchar_t __wchar, mbstate_t *__p,
			  size_t __buflen) __THROW __wur;
extern size_t __REDIRECT_NTH (__wcrtomb_alias,
			      (char *__restrict __s, wchar_t __wchar,
			       mbstate_t *__restrict __ps), wcrtomb) __wur;

__extern_always_inline __wur size_t
__NTH (wcrtomb (char *__s, wchar_t __wchar, mbstate_t *__ps))
{
  /* We would have to include <limits.h> to get a definition of MB_LEN_MAX.
     But this would only disturb the namespace.  So we define our own
     version here.  */
#define __WCHAR_MB_LEN_MAX	16
#if defined MB_LEN_MAX && MB_LEN_MAX != __WCHAR_MB_LEN_MAX
# error "Assumed value of MB_LEN_MAX wrong"
#endif
  if (__bos (__s) != (size_t) -1 && __WCHAR_MB_LEN_MAX > __bos (__s))
    return __wcrtomb_chk (__s, __wchar, __ps, __bos (__s));
  return __wcrtomb_alias (__s, __wchar, __ps);
}


extern size_t __mbsrtowcs_chk (wchar_t *__restrict __dst,
			       __const char **__restrict __src,
			       size_t __len, mbstate_t *__restrict __ps,
			       size_t __dstlen) __THROW;
extern size_t __REDIRECT_NTH (__mbsrtowcs_alias,
			      (wchar_t *__restrict __dst,
			       __const char **__restrict __src,
			       size_t __len, mbstate_t *__restrict __ps),
			      mbsrtowcs);

__extern_always_inline size_t
__NTH (mbsrtowcs (wchar_t *__restrict __dst, __const char **__restrict __src,
		  size_t __len, mbstate_t *__restrict __ps))
{
  if (__bos (__dst) != (size_t) -1
      && (!__builtin_constant_p (__len)
	  || __len > __bos (__dst) / sizeof (wchar_t)))
    return __mbsrtowcs_chk (__dst, __src, __len, __ps,
			    __bos (__dst) / sizeof (wchar_t));
  return __mbsrtowcs_alias (__dst, __src, __len, __ps);
}


extern size_t __wcsrtombs_chk (char *__restrict __dst,
			       __const wchar_t **__restrict __src,
			       size_t __len, mbstate_t *__restrict __ps,
			       size_t __dstlen) __THROW;
extern size_t __REDIRECT_NTH (__wcsrtombs_alias,
			      (char *__restrict __dst,
			       __const wchar_t **__restrict __src,
			       size_t __len, mbstate_t *__restrict __ps),
			      wcsrtombs);

__extern_always_inline size_t
__NTH (wcsrtombs (char *__restrict __dst, __const wchar_t **__restrict __src,
		  size_t __len, mbstate_t *__restrict __ps))
{
  if (__bos (__dst) != (size_t) -1
      && (!__builtin_constant_p (__len) || __len > __bos (__dst)))
    return __wcsrtombs_chk (__dst, __src, __len, __ps, __bos (__dst));
  return __wcsrtombs_alias (__dst, __src, __len, __ps);
}


#ifdef __USE_GNU
extern size_t __mbsnrtowcs_chk (wchar_t *__restrict __dst,
				__const char **__restrict __src, size_t __nmc,
				size_t __len, mbstate_t *__restrict __ps,
				size_t __dstlen) __THROW;
extern size_t __REDIRECT_NTH (__mbsnrtowcs_alias,
			      (wchar_t *__restrict __dst,
			       __const char **__restrict __src, size_t __nmc,
			       size_t __len, mbstate_t *__restrict __ps),
			      mbsnrtowcs);

__extern_always_inline size_t
__NTH (mbsnrtowcs (wchar_t *__restrict __dst, __const char **__restrict __src,
		   size_t __nmc, size_t __len, mbstate_t *__restrict __ps))
{
  if (__bos (__dst) != (size_t) -1
      && (!__builtin_constant_p (__len)
	  || __len > __bos (__dst) / sizeof (wchar_t)))
    return __mbsnrtowcs_chk (__dst, __src, __nmc, __len, __ps,
			     __bos (__dst) / sizeof (wchar_t));
  return __mbsnrtowcs_alias (__dst, __src, __nmc, __len, __ps);
}


extern size_t __wcsnrtombs_chk (char *__restrict __dst,
				__const wchar_t **__restrict __src,
				size_t __nwc, size_t __len,
				mbstate_t *__restrict __ps, size_t __dstlen)
     __THROW;
extern size_t __REDIRECT_NTH (__wcsnrtombs_alias,
			      (char *__restrict __dst,
			       __const wchar_t **__restrict __src,
			       size_t __nwc, size_t __len,
			       mbstate_t *__restrict __ps), wcsnrtombs);

__extern_always_inline size_t
__NTH (wcsnrtombs (char *__restrict __dst, __const wchar_t **__restrict __src,
		   size_t __nwc, size_t __len, mbstate_t *__restrict __ps))
{
  if (__bos (__dst) != (size_t) -1
      && (!__builtin_constant_p (__len) || __len > __bos (__dst)))
    return __wcsnrtombs_chk (__dst, __src, __nwc, __len, __ps, __bos (__dst));
  return __wcsnrtombs_alias (__dst, __src, __nwc, __len, __ps);
}
#endif
