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

#ifndef _STRING_H
# error "Never use <bits/string2.h> directly; include <string.h> instead."
#endif

#ifndef __NO_STRING_INLINES

/* Unlike the definitions in the header <bits/string.h> the
   definitions contained here are not optimizing down to assembler
   level.  These optimizations are not always a good idea since this
   means the code size increases a lot.  Instead the definitions here
   optimize some functions in a way which does not dramatically
   increase the code size and which does not use assembler.  The main
   trick is to use GNU CC's `__builtin_constant_p' function.

   Every function XXX which has a defined version in
   <bits/string.h> must be accompanied by a symbol _HAVE_STRING_ARCH_XXX
   to make sure we don't get redefinitions.

   We must use here macros instead of inline functions since the
   trick won't work with the later.  */

#ifdef __cplusplus
# define __STRING_INLINE inline
#else
# define __STRING_INLINE extern __inline
#endif

#if _STRING_ARCH_unaligned
/* If we can do unaligned memory accesses we must know the endianess.  */
# include <endian.h>
# include <bits/types.h>

# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define __STRING2_SMALL_GET16(src, idx) \
     (((src)[idx + 1] << 8) | (src)[idx])
#  define __STRING2_SMALL_GET32(src, idx) \
     ((((src)[idx + 3] << 8 | (src)[idx + 2]) << 8			      \
       | (src)[idx + 1]) << 8 | (src)[idx])
# else
#  define __STRING2_SMALL_GET16(src, idx) \
     (((src)[idx] << 8) | (src)[idx + 1])
#  define __STRING2_SMALL_GET32(src, idx) \
     ((((src)[idx] << 8 | (src)[idx + 1]) << 8				      \
       | (src)[idx + 2]) << 8 | (src)[idx + 3])
# endif
#else
/* These are a few types we need for the optimizations if we cannot
   use unaligned memory accesses.  */
# define __STRING2_COPY_TYPE(N) \
  typedef struct { char __arr[N]; }					      \
    __STRING2_COPY_ARR##N __attribute__ ((packed))
__STRING2_COPY_TYPE (2);
__STRING2_COPY_TYPE (3);
__STRING2_COPY_TYPE (4);
__STRING2_COPY_TYPE (5);
__STRING2_COPY_TYPE (6);
__STRING2_COPY_TYPE (7);
__STRING2_COPY_TYPE (8);
# undef __STRING2_COPY_TYPE
#endif


/* Set N bytes of S to C.  */
#ifndef _HAVE_STRING_ARCH_memset
# define memset(s, c, n) \
  (__extension__ (__builtin_constant_p (c) && (c) == '\0'		      \
		  ? ({ void *__s = (s); __bzero (__s, n); __s; })	      \
		  : memset (s, c, n)))
#endif


/* Copy SRC to DEST.  */
#ifndef _HAVE_STRING_ARCH_strcpy
# define strcpy(dest, src) \
  (__extension__ (__builtin_constant_p (src)				      \
		  ? (sizeof ((src)[0]) == 1 && strlen (src) + 1 <= 8	      \
		     ? __strcpy_small (dest, src, strlen (src) + 1)	      \
		     : (char *) memcpy (dest, src, strlen (src) + 1))	      \
		  : strcpy (dest, src)))

# if _STRING_ARCH_unaligned
#  define __strcpy_small(dest, src, srclen) \
  (__extension__ ({ unsigned char *__dest = (unsigned char *) (dest);	      \
		    switch (srclen)					      \
		      {							      \
		      case 1:						      \
			*__dest = '\0';					      \
			break;						      \
		      case 2:						      \
			*((__uint16_t *) __dest) =			      \
			  __STRING2_SMALL_GET16 (src, 0);		      \
			break;						      \
		      case 3:						      \
			*((__uint16_t *) __dest) =			      \
			  __STRING2_SMALL_GET16 (src, 0);		      \
			*(__dest + 2) = '\0';				      \
			break;						      \
		      case 4:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			break;						      \
		      case 5:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*(__dest + 4) = '\0';				      \
			break;						      \
		      case 6:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*((__uint16_t *) (__dest + 4)) =		      \
			  __STRING2_SMALL_GET16 (src, 4);		      \
			break;						      \
		      case 7:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*((__uint16_t *) (__dest + 4)) =		      \
			  __STRING2_SMALL_GET16 (src, 4);		      \
			*(__dest + 6) = '\0';				      \
			break;						      \
		      case 8:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*((__uint32_t *) (__dest + 4)) =		      \
			  __STRING2_SMALL_GET32 (src, 4);		      \
			break;						      \
		      }							      \
		    (char *) __dest; }))
# else
#  define __strcpy_small(dest, src, srclen) \
  (__extension__ ({ unsigned char *__dest = (unsigned char *) (dest);	      \
		    switch (srclen)					      \
		      {							      \
		      case 1:						      \
			*__dest = '\0';					      \
			break;						      \
		      case 2:						      \
			*((__STRING2_COPY_ARR2 *) __dest) =		      \
			  ((__STRING2_COPY_ARR2) { { (src)[0], '\0' } });     \
			break;						      \
		      case 3:						      \
			*((__STRING2_COPY_ARR3 *) __dest) =		      \
			  ((__STRING2_COPY_ARR3) { { (src)[0], (src)[1],      \
						     '\0' } });		      \
			break;						      \
		      case 4:						      \
			*((__STRING2_COPY_ARR4 *) __dest) =		      \
			  ((__STRING2_COPY_ARR4) { { (src)[0], (src)[1],      \
						     (src)[2], '\0' } });     \
			break;						      \
		      case 5:						      \
			*((__STRING2_COPY_ARR5 *) __dest) =		      \
			  ((__STRING2_COPY_ARR5) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     '\0' } });		      \
			break;						      \
		      case 6:						      \
			*((__STRING2_COPY_ARR6 *) __dest) =		      \
			  ((__STRING2_COPY_ARR6) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     (src)[4], '\0' } });     \
			break;						      \
		      case 7:						      \
			*((__STRING2_COPY_ARR7 *) __dest) =		      \
			  ((__STRING2_COPY_ARR7) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     (src)[4], (src)[5],      \
						     '\0' } });		      \
			break;						      \
		      case 8:						      \
			*((__STRING2_COPY_ARR8 *) __dest) =		      \
			  ((__STRING2_COPY_ARR8) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     (src)[4], (src)[5],      \
						     (src)[6], '\0' } });     \
			break;						      \
		    }							      \
		  (char *) __dest; }))
# endif
#endif


/* Copy SRC to DEST, returning pointer to final NUL byte.  */
#ifdef __USE_GNU
# ifndef _HAVE_STRING_ARCH_stpcpy
#  define __stpcpy(dest, src) \
  (__extension__ (__builtin_constant_p (src)				      \
		  ? (sizeof ((src)[0]) == 1 && strlen (src) + 1 <= 8	      \
		     ? __stpcpy_small (dest, src, strlen (src) + 1)	      \
		     : ((char *) __mempcpy (dest, src, strlen (src) + 1) - 1))\
		  : __stpcpy (dest, src)))
/* In glibc we use this function frequently but for namespace reasons
   we have to use the name `__stpcpy'.  */
#  define stpcpy(dest, src) __stpcpy (dest, src)

#  if _STRING_ARCH_unaligned
#   define __stpcpy_small(dest, src, srclen) \
  (__extension__ ({ unsigned char *__dest = (unsigned char *) (dest);	      \
		    switch (srclen)					      \
		      {							      \
		      case 1:						      \
			*__dest = '\0';					      \
			break;						      \
		      case 2:						      \
			*((__uint16_t *) __dest) =			      \
			  __STRING2_SMALL_GET16 (src, 0);		      \
			++__dest;					      \
			break;						      \
		      case 3:						      \
			*((__uint16_t *) __dest)++ =			      \
			  __STRING2_SMALL_GET16 (src, 0);		      \
			*__dest = '\0';					      \
			break;						      \
		      case 4:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			__dest += 3;					      \
			break;						      \
		      case 5:						      \
			*((__uint32_t *) __dest)++ =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*__dest = '\0';					      \
			break;						      \
		      case 6:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*((__uint16_t *) (__dest + 4)) =		      \
			  __STRING2_SMALL_GET16 (src, 4);		      \
			__dest += 5;					      \
			break;						      \
		      case 7:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*((__uint16_t *) (__dest + 4)) =		      \
			  __STRING2_SMALL_GET16 (src, 4);		      \
			__dest += 6;					      \
			*__dest = '\0';					      \
			break;						      \
		      case 8:						      \
			*((__uint32_t *) __dest) =			      \
			  __STRING2_SMALL_GET32 (src, 0);		      \
			*((__uint32_t *) (__dest + 4)) =		      \
			  __STRING2_SMALL_GET32 (src, 4);		      \
			__dest += 7;					      \
			break;						      \
		      }							      \
		    (char *) __dest; }))
#  else
#   define __stpcpy_small(dest, src, srclen) \
  (__extension__ ({ unsigned char *__dest = (unsigned char *) (dest);	      \
		    switch (srclen)					      \
		      {							      \
		      case 1:						      \
			*__dest = '\0';					      \
			break;						      \
		      case 2:						      \
			*((__STRING2_COPY_ARR2 *) __dest) =		      \
			  ((__STRING2_COPY_ARR2) { { (src)[0], '\0' } });     \
			break;						      \
		      case 3:						      \
			*((__STRING2_COPY_ARR3 *) __dest) =		      \
			  ((__STRING2_COPY_ARR3) { { (src)[0], (src)[1],      \
						     '\0' } });		      \
			break;						      \
		      case 4:						      \
			*((__STRING2_COPY_ARR4 *) __dest) =		      \
			  ((__STRING2_COPY_ARR4) { { (src)[0], (src)[1],      \
						     (src)[2], '\0' } });     \
			break;						      \
		      case 5:						      \
			*((__STRING2_COPY_ARR5 *) __dest) =		      \
			  ((__STRING2_COPY_ARR5) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     '\0' } });		      \
			break;						      \
		      case 6:						      \
			*((__STRING2_COPY_ARR6 *) __dest) =		      \
			  ((__STRING2_COPY_ARR6) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     (src)[4], '\0' } });     \
			break;						      \
		      case 7:						      \
			*((__STRING2_COPY_ARR7 *) __dest) =		      \
			  ((__STRING2_COPY_ARR7) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     (src)[4], (src)[5],      \
						     '\0' } });		      \
			break;						      \
		      case 8:						      \
			*((__STRING2_COPY_ARR8 *) __dest) =		      \
			  ((__STRING2_COPY_ARR8) { { (src)[0], (src)[1],      \
						     (src)[2], (src)[3],      \
						     (src)[4], (src)[5],      \
						     (src)[6], '\0' } });     \
			break;						      \
		    }							      \
		  (char *) (__dest + ((srclen) - 1)); }))
#  endif
# endif
#endif


/* Copy no more than N characters of SRC to DEST.  */
#ifndef _HAVE_STRING_ARCH_strncpy
# if defined _HAVE_STRING_ARCH_memset && defined _HAVE_STRING_ARCH_mempcpy
#  define strncpy(dest, src, n) \
  (__extension__ ({ char *__dest = (dest);				      \
		    __builtin_constant_p (src) && __builtin_constant_p (n)    \
		    ? (strlen (src) + 1 >= ((size_t) (n))		      \
		       ? (char *) memcpy (__dest, src, n)		      \
		       : (memset (__mempcpy (__dest, src, strlen (src)),      \
				  '\0', n - strlen (src)),		      \
			  __dest))					      \
		    : strncpy (__dest, src, n); }))
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
  (__extension__ ({ char *__dest = (dest);				      \
		    __builtin_constant_p (src) && __builtin_constant_p (n)    \
		    ? (strlen (src) < ((size_t) (n))			      \
		       ? strcat (__dest, src)				      \
		       : (memcpy (strchr (__dest, '\0'), src, n), __dest))    \
		    : strncat (dest, src, n); }))
# else
#  define strncat(dest, src, n) \
  (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n)      \
		  ? (strlen (src) < ((size_t) (n))			      \
		     ? strcat (dest, src)				      \
		     : strncat (dest, src, n))				      \
		  : strncat (dest, src, n)))
# endif
#endif


/* Compare characters of S1 and S2.  */
#ifndef _HAVE_STRING_ARCH_strcmp
# define strcmp(s1, s2) \
  (__extension__ (__builtin_constant_p (s1) && __builtin_constant_p (s2)      \
		  && (sizeof (s1)[0] != 1 || strlen (s1) >= 4)		      \
		  && (sizeof (s2)[0] != 1 || strlen (s2) >= 4)		      \
		  ? memcmp (s1, s2, (strlen (s1) < strlen (s2)		      \
				     ? strlen (s1) : strlen (s2)) + 1)	      \
		  : (__builtin_constant_p (s1) && sizeof (s1)[0] == 1	      \
		     && sizeof (s2)[0] == 1 && strlen (s1) < 4		      \
		     ? __strcmp_cg (s1, s2, strlen (s1))		      \
		     : (__builtin_constant_p (s2) && sizeof (s1)[0] == 1      \
			&& sizeof (s2)[0] == 1 && strlen (s2) < 4	      \
			? __strcmp_gc (s1, s2, strlen (s2))		      \
			: strcmp (s1, s2)))))

# define __strcmp_cg(s1, s2, l1) \
  (__extension__ ({ __const unsigned char *__s2 = (unsigned char *) (s2);     \
		    register int __result = (unsigned char) (s1)[0] - __s2[0];\
		    if (l1 > 0 && __result == 0)			      \
		      {							      \
			__result = (unsigned char) (s1)[1] - __s2[1];	      \
			if (l1 > 1 && __result == 0)			      \
			  {						      \
			    __result = (unsigned char) (s1)[2] - __s2[2];     \
			    if (l1 > 2 && __result == 0)		      \
			      __result = (unsigned char) (s1)[3] - __s2[3];   \
			  }						      \
		      }							      \
		    __result; }))

# define __strcmp_gc(s1, s2, l2) \
  (__extension__ ({ __const unsigned char *__s1 = (unsigned char *) (s1);     \
		    register int __result = __s1[0] - (unsigned char) (s2)[0];\
		    if (l2 > 0 && __result == 0)			      \
		      {							      \
			__result = __s1[1] - (unsigned char) (s2)[1];	      \
			if (l2 > 1 && __result == 0)			      \
			  {						      \
			    __result = __s1[2] - (unsigned char) (s2)[2];     \
			    if (l2 > 2 && __result == 0)		      \
			      __result = __s1[3] - (unsigned char) (s2)[3];   \
			  }						      \
		      }							      \
		    __result; }))
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
  (__extension__ (__builtin_constant_p (reject) && sizeof ((reject)[0]) == 1  \
		  ? ((reject)[0] == '\0'				      \
		     ? strlen (s)					      \
		     : ((reject)[1] == '\0'				      \
			? __strcspn_c1 (s, (reject)[0])			      \
			: strcspn (s, reject)))				      \
		  : strcspn (s, reject)))

__STRING_INLINE size_t __strcspn_c1 (__const char *__s, char __reject);
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
  (__extension__ (__builtin_constant_p (accept) && sizeof ((accept)[0]) == 1  \
		  ? ((accept)[0] == '\0'				      \
		     ? 0						      \
		     : ((accept)[1] == '\0'				      \
			? __strspn_c1 (s, (accept)[0])			      \
			: strspn (s, accept)))				      \
		  : strspn (s, accept)))

__STRING_INLINE size_t __strspn_c1 (__const char *__s, char __accept);
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
  (__extension__ (__builtin_constant_p (accept) && sizeof ((accept)[0]) == 1  \
		  ? ((accept)[0] == '\0'				      \
		     ? NULL						      \
		     : ((accept)[1] == '\0'				      \
			? strchr (s, (accept)[0])			      \
			: strpbrk (s, accept)))				      \
		  : strpbrk (s, accept)))
#endif


/* Find the first occurrence of NEEDLE in HAYSTACK.  */
#ifndef _HAVE_STRING_ARCH_strstr
# define strstr(haystack, needle) \
  (__extension__ (__builtin_constant_p (needle) && sizeof ((needle)[0]) == 1  \
		  ? ((needle)[0] == '\0'				      \
		     ? haystack						      \
		     : ((needle)[1] == '\0'				      \
			? strchr (haystack, (needle)[0])		      \
			: strstr (haystack, needle)))			      \
		  : strstr (haystack, needle)))
#endif


#ifdef __USE_GNU
# ifndef _HAVE_STRING_ARCH_strnlen
__STRING_INLINE size_t strnlen (__const char *__string, size_t __maxlen);
__STRING_INLINE size_t
strnlen (__const char *__string, size_t __maxlen)
{
  __const char *__end = (__const char *) memchr (__string, '\0', __maxlen);
  return __end ? __end - __string : __maxlen;
}
# endif
#endif


#ifdef __USE_BSD
# ifndef _HAVE_STRING_ARCH_strsep

#  define strsep(s, reject) \
  (__extension__ (__builtin_constant_p (reject) && sizeof ((reject)[0]) == 1  \
		  ? ((reject)[0] != '\0' && (reject)[1] == '\0'		      \
		     ? __strsep_1c (s, (reject)[0])			      \
		     : __strsep_g (s, reject))				      \
		  : __strsep_g (s, reject)))

__STRING_INLINE char *__strsep_1c (char **__s, char __reject);
__STRING_INLINE char *
__strsep_1c (char **__s, char __reject)
{
  register char *__retval = *__s;
  if (__retval == NULL || *__retval == '\0')
    return NULL;
  while (*__retval == __reject)
    ++__retval;
  if ((*__s = strchr (__retval, __reject)) != NULL)
    *(*__s)++ = '\0';
  return __retval;
}

__STRING_INLINE char *__strsep_g (char **__s, __const char *__reject);
__STRING_INLINE char *
__strsep_g (char **__s, __const char *__reject)
{
  register char *__retval = *__s;
  if (__retval == NULL || *__retval == '\0')
    return NULL;
  if ((*__s = strpbrk (__retval, __reject)) != NULL)
    *(*__s)++ = '\0';
  return __retval;
}
# endif
#endif


#undef __STRING_INLINE

#endif /* No string inlines.  */
