/* Machine-independant string function optimizations.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _BITS_STRING2_H
#define _BITS_STRING2_H	1

/* Unlike the definitions in the header <bits/string.h> the
   definitions contained here are not optimizing down to assembler
   level.  These optimizations are not always a good idea since this
   means the code size increases a lot.  Instead the definitions here
   optimize some functions in a way which does not dramatically
   increase the code size and which does not use assembler.  The main
   trick is to use GNU CC's `__builtin_constant_p' function.

   Every function XXX which has a defined version in
   <bits/string.h> must be accompanied by a have _HAVE_STRING_ARCH_XXX
   to make sure we don't get redefinitions.

   We must use here macros instead of inline functions since the
   trick won't work with the later.  */

#ifdef __cplusplus
# define __STRING_INLINE inline
#else
# define __STRING_INLINE extern __inline
#endif

/* We need some more types.  */
#include <bits/types.h>


/* Copy SRC to DEST.  */
#ifndef _HAVE_STRING_ARCH_strcpy
# define strcpy(dest, src) \
  (__extension__ (__builtin_constant_p (src)				      \
		  ? (strlen (src) + 1 <= 8				      \
		     ? __strcpy_small (dest, src, strlen (src) + 1)	      \
		     : (char *) memcpy (dest, src, strlen (src) + 1))	      \
		  : strcpy (dest, src)))

__STRING_INLINE char *
__strcpy_small (char *__dest, __const char *__src, size_t __srclen)
{
  register char *__tmp = __dest;
  switch (__srclen)
    {
    case 7:
      *((__uint16_t *) __tmp)++ = *((__uint16_t *) __src)++;
    case 5:
      *((__uint32_t *) __tmp)++ = *((__uint32_t *) __src)++;
      *((unsigned char *) __tmp) = '\0';
      break;

    case 8:
      *((__uint32_t *) __tmp)++ = *((__uint32_t *) __src)++;
    case 4:
      *((__uint32_t *) __tmp) = *((__uint32_t *) __src);
      break;

    case 6:
      *((__uint32_t *) __tmp)++ = *((__uint32_t *) __src)++;
    case 2:
      *((__uint16_t *) __tmp) = *((__uint16_t *) __src);
      break;

    case 3:
      *((__uint16_t *) __tmp)++ = *((__uint16_t *) __src)++;
    case 1:
      *((unsigned char *) __tmp) = '\0';
      break;

    default:
      break;
    }
  return __dest;
}
#endif


/* Copy SRC to DEST, returning pointer to final NUL byte.  */
#ifdef __USE_GNU
# ifndef _HAVE_STRING_ARCH_stpcpy
#  define __stpcpy(dest, src) \
  (__extension__ (__builtin_constant_p (src)				      \
		  ? (strlen (src) + 1 <= 8				      \
		     ? __stpcpy_small (dest, src, strlen (src) + 1)	      \
		     : ((char *) __mempcpy (dest, src, strlen (src) + 1) - 1))\
		  : __stpcpy (dest, src)))
/* In glibc we use this function frequently but for namespace reasons
   we have to use the name `__stpcpy'.  */
#  define stpcpy(dest, src) __stpcpy (dest, src)

__STRING_INLINE char *
__stpcpy_small (char *__dest, __const char *__src, size_t __srclen)
{
  register char *__tmp = __dest;
  switch (__srclen)
    {
    case 7:
      *((__uint16_t *) __tmp)++ = *((__uint16_t *) __src)++;
    case 5:
      *((__uint32_t *) __tmp)++ = *((__uint32_t *) __src)++;
      *((unsigned char *) __tmp) = '\0';
      return __tmp;

    case 8:
      *((__uint32_t *) __tmp)++ = *((__uint32_t *) __src)++;
    case 4:
      *((__uint32_t *) __tmp) = *((__uint32_t *) __src);
      return __tmp + 3;

    case 6:
      *((__uint32_t *) __tmp)++ = *((__uint32_t *) __src)++;
    case 2:
      *((__uint16_t *) __tmp) = *((__uint16_t *) __src);
      return __tmp + 1;

    case 3:
      *((__uint16_t *) __tmp)++ = *((__uint16_t *) __src)++;
    case 1:
      *((unsigned char *) __tmp) = '\0';
      return __tmp;

    default:
      break;
    }
  /* This should never happen.  */
  return NULL;
}
# endif
#endif


/* Copy no more than N characters of SRC to DEST.  */
#ifndef _HAVE_STRING_ARCH_strncpy
# if defined _HAVE_STRING_ARCH_memset && defined _HAVE_STRING_ARCH_mempcpy
#  define strncpy(dest, src, n) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  ? (strlen (src) + 1 >= ((size_t) (n))			      \
		     ? (char *) memcpy (dest, src, n)			      \
		     : (memset (__mempcpy (dest, src, strlen (src)), '\0',    \
				n - strlen (src)),			      \
			dest))						      \
		  : strncpy (dest, src, n)))
# else
#  define strncpy(dest, src, n) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  ? (strlen (src) + 1 >= ((size_t) (n))			      \
		     ? (char *) memcpy (dest, src, n)			      \
		     : strncpy (dest, src, n))				      \
		  : strncpy (dest, src, n)))
# endif
#endif


/* Append no more than N characters from SRC onto DEST.  */
#ifndef _HAVE_STRING_ARCH_strncat
# ifdef _HAVE_STRING_ARCH_strchr
#  define strncat(dest, src, n) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  ? (strlen (src) < ((size_t) (n))			      \
		     ? strcat (dest, src)				      \
		     : (memcpy (strchr (dest, '\0'), src, n), dest))	      \
		  : strncat (dest, src, n)))
# else
#  define strncat(dest, src, n) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  ? (strlen (src) < ((size_t) (n))			      \
		     ? strcat (dest, src)				      \
		     : strncat (dest, src, n))				      \
		  : strncat (dest, src, n)))
# endif
#endif


/* Compare N characters of S1 and S2.  */
#ifndef _HAVE_STRING_ARCH_strncmp
# define strncmp(s1, s2, n) \
  (__extension__ (__builtin_constant_p (s1) && strlen (s1) < ((size_t) (n))   \
		  ? strcmp (s1, s2)					      \
		  : (__builtin_constant_p (s2) && strlen (s2) < ((size_t) (n))\
		     ? strcmp (s1, s2)					      \
		     : strncmp (s1, s2, n))))
#endif


/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
#ifndef _HAVE_STRING_ARCH_strcspn
# define strcspn(s, reject) \
  (__extension__ (__builtin_constant_p (reject)				      \
		  ? (((const char *) (reject))[0] == '\0'		      \
		     ? strlen (s)					      \
		     : (((const char *) (reject))[1] == '\0'		      \
			? __strcspn_c1 (s, ((((const char *) (reject))[0]     \
					     & 0xff) << 8))		      \
			: strcspn (s, reject)))	      \
		  : strcspn (s, reject)))

__STRING_INLINE size_t
__strcspn_c1 (__const char *__s, char __reject)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject)
    ++__result;
  return __result;
}
#endif


/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
#ifndef _HAVE_STRING_ARCH_strspn
# define strspn(s, accept) \
  (__extension__ (__builtin_constant_p (accept)				      \
		  ? (((const char *) (accept))[0] == '\0'		      \
		     ? 0						      \
		     : (((const char *) (accept))[1] == '\0'		      \
			? __strspn_c1 (s, ((const char *) (accept))[0])	      \
			: strspn (s, accept)))				      \
		  : strspn (s, accept)))

__STRING_INLINE size_t
__strspn_c1 (__const char *__s, char __accept)
{
  register size_t __result = 0;
  /* Please note that __accept never can be '\0'.  */
  while (__s[__result] == __accept)
    ++__result;
  return __result;
}
#endif


/* Find the first occurrence in S of any character in ACCEPT.  */
#ifndef _HAVE_STRING_ARCH_strpbrk
# define strpbrk(s, accept) \
  (__extension__ (__builtin_constant_p (accept)				      \
		  ? (((const char *) (accept))[0] == '\0'		      \
		     ? NULL						      \
		     : (((const char *) (accept))[1] == '\0'		      \
			? strchr (s, ((const char *) (accept))[0])	      \
			: strpbrk (s, accept)))				      \
		  : strpbrk (s, accept)))
#endif


/* Find the first occurrence of NEEDLE in HAYSTACK.  */
#ifndef _HAVE_STRING_ARCH_strstr
# define strstr(haystack, needle) \
  (__extension__ (__builtin_constant_p (needle)				      \
		  ? (((const char *) (needle))[0] == '\0'		      \
		     ? haystack						      \
		     : (((const char *) (needle))[1] == '\0'		      \
			? strchr (haystack, ((const char *) (needle))[0])     \
			: strstr (haystack, needle)))			      \
		  : strstr (haystack, needle)))
#endif


#ifdef __USE_GNU
# ifndef _HAVE_STRING_ARCH_strnlen
extern __inline size_t
strnlen (__const char *__string, size_t __maxlen)
{
  __const char *__end = (__const char *) memchr (__string, '\0', __maxlen);
  return __end ? __end - __string : __maxlen;
}
# endif
#endif


#undef __STRING_INLINE

#endif /* bits/string2.h */
