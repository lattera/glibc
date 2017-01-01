/* Machine-independant string function optimizations.
   Copyright (C) 1997-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#ifndef _STRING_H
# error "Never use <bits/string2.h> directly; include <string.h> instead."
#endif

#ifndef __NO_STRING_INLINES

/* Unlike the definitions in the header <bits/string.h> the
   definitions contained here are not optimized down to assembler
   level.  Those optimizations are not always a good idea since this
   means the code size increases a lot.  Instead the definitions here
   optimize some functions in a way which do not dramatically
   increase the code size and which do not use assembler.  The main
   trick is to use GCC's `__builtin_constant_p' function.

   Every function XXX which has a defined version in
   <bits/string.h> must be accompanied by a symbol _HAVE_STRING_ARCH_XXX
   to make sure we don't get redefinitions.

   We must use here macros instead of inline functions since the
   trick won't work with the latter.  */

#ifndef __STRING_INLINE
# ifdef __cplusplus
#  define __STRING_INLINE inline
# else
#  define __STRING_INLINE __extern_inline
# endif
#endif

/* Dereferencing a pointer arg to run sizeof on it fails for the void
   pointer case, so we use this instead.
   Note that __x is evaluated twice. */
#define __string2_1bptr_p(__x) \
  ((size_t)(const void *)((__x) + 1) - (size_t)(const void *)(__x) == 1)

/* Set N bytes of S to 0.  */
#if !defined _HAVE_STRING_ARCH_memset
# define __bzero(s, n) __builtin_memset (s, '\0', n)
#endif


#ifndef _HAVE_STRING_ARCH_strchr
extern void *__rawmemchr (const void *__s, int __c);
#  define strchr(s, c) \
  (__extension__ (__builtin_constant_p (c) && !__builtin_constant_p (s)	      \
		  && (c) == '\0'					      \
		  ? (char *) __rawmemchr (s, c)				      \
		  : __builtin_strchr (s, c)))
#endif


/* Copy SRC to DEST, returning pointer to final NUL byte.  */
#ifdef __USE_GNU
# ifndef _HAVE_STRING_ARCH_stpcpy
#  define __stpcpy(dest, src) __builtin_stpcpy (dest, src)
/* In glibc we use this function frequently but for namespace reasons
   we have to use the name `__stpcpy'.  */
#  define stpcpy(dest, src) __stpcpy (dest, src)
# endif
#endif


/* Copy no more than N characters of SRC to DEST.  */
#ifndef _HAVE_STRING_ARCH_strncpy
# define strncpy(dest, src, n) __builtin_strncpy (dest, src, n)
#endif


/* Append no more than N characters from SRC onto DEST.  */
#ifndef _HAVE_STRING_ARCH_strncat
# ifdef _USE_STRING_ARCH_strchr
#  define strncat(dest, src, n) \
  (__extension__ ({ char *__dest = (dest);				      \
		    __builtin_constant_p (src) && __builtin_constant_p (n)    \
		    ? (strlen (src) < ((size_t) (n))			      \
		       ? strcat (__dest, src)				      \
		       : (*((char *) __mempcpy (strchr (__dest, '\0'),	      \
						src, n)) = '\0', __dest))     \
		    : strncat (dest, src, n); }))
# else
#  define strncat(dest, src, n) __builtin_strncat (dest, src, n)
# endif
#endif


/* Compare characters of S1 and S2.  */
#ifndef _HAVE_STRING_ARCH_strcmp
# define strcmp(s1, s2) \
  __extension__								      \
  ({ size_t __s1_len, __s2_len;						      \
     (__builtin_constant_p (s1) && __builtin_constant_p (s2)		      \
      && (__s1_len = strlen (s1), __s2_len = strlen (s2),		      \
	  (!__string2_1bptr_p (s1) || __s1_len >= 4)			      \
	  && (!__string2_1bptr_p (s2) || __s2_len >= 4))		      \
      ? __builtin_strcmp (s1, s2)					      \
      : (__builtin_constant_p (s1) && __string2_1bptr_p (s1)		      \
	 && (__s1_len = strlen (s1), __s1_len < 4)			      \
	 ? (__builtin_constant_p (s2) && __string2_1bptr_p (s2)		      \
	    ? __builtin_strcmp (s1, s2)					      \
	    : __strcmp_cg (s1, s2, __s1_len))				      \
	 : (__builtin_constant_p (s2) && __string2_1bptr_p (s2)		      \
	    && (__s2_len = strlen (s2), __s2_len < 4)			      \
	    ? (__builtin_constant_p (s1) && __string2_1bptr_p (s1)	      \
	       ? __builtin_strcmp (s1, s2)				      \
	       : -__strcmp_cg (s2, s1, __s2_len))			      \
            : __builtin_strcmp (s1, s2)))); })

# define __strcmp_cg(s1, s2, l1) \
  (__extension__ ({ const unsigned char *__s2 =				      \
		      (const unsigned char *) (const char *) (s2);	      \
		    int __result =					      \
		      (((const unsigned char *) (const char *) (s1))[0]	      \
		       - __s2[0]);					      \
		    if (l1 > 0 && __result == 0)			      \
		      {							      \
			__result = (((const unsigned char *)		      \
				     (const char *) (s1))[1] - __s2[1]);      \
			if (l1 > 1 && __result == 0)			      \
			  {						      \
			    __result = (((const unsigned char *)	      \
					 (const char *) (s1))[2] - __s2[2]);  \
			    if (l1 > 2 && __result == 0)		      \
			      __result = (((const unsigned char *)	      \
					  (const char *)  (s1))[3]	      \
					  - __s2[3]);			      \
			  }						      \
		      }							      \
		    __result; }))
#endif


/* Compare N characters of S1 and S2.  */
#ifndef _HAVE_STRING_ARCH_strncmp
# define strncmp(s1, s2, n)						      \
  (__extension__ (__builtin_constant_p (n)				      \
		  && ((__builtin_constant_p (s1)			      \
		       && strlen (s1) < ((size_t) (n)))			      \
		      || (__builtin_constant_p (s2)			      \
			  && strlen (s2) < ((size_t) (n))))		      \
		  ? strcmp (s1, s2) : strncmp (s1, s2, n)))
#endif


/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
#ifndef _HAVE_STRING_ARCH_strcspn
# define strcspn(s, reject) __builtin_strcspn (s, reject)
#endif


/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
#ifndef _HAVE_STRING_ARCH_strspn
# define strspn(s, accept) __builtin_strspn (s, accept)
#endif


/* Find the first occurrence in S of any character in ACCEPT.  */
#ifndef _HAVE_STRING_ARCH_strpbrk
# define strpbrk(s, accept) __builtin_strpbrk (s, accept)
#endif


/* We need the memory allocation functions for inline strdup().
   Referring to stdlib.h (even minimally) is not allowed
   in any of the tight standards compliant modes.  */
#ifdef __USE_MISC

# if !defined _HAVE_STRING_ARCH_strdup || !defined _HAVE_STRING_ARCH_strndup
#  define __need_malloc_and_calloc
#  include <stdlib.h>
# endif

# ifndef _HAVE_STRING_ARCH_strdup

extern char *__strdup (const char *__string) __THROW __attribute_malloc__;
#  define __strdup(s) \
  (__extension__ (__builtin_constant_p (s) && __string2_1bptr_p (s)	      \
		  ? (((const char *) (s))[0] == '\0'			      \
		     ? (char *) calloc ((size_t) 1, (size_t) 1)		      \
		     : ({ size_t __len = strlen (s) + 1;		      \
			  char *__retval = (char *) malloc (__len);	      \
			  if (__retval != NULL)				      \
			    __retval = (char *) memcpy (__retval, s, __len);  \
			  __retval; }))					      \
		  : __strdup (s)))

#  if defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K8
#   define strdup(s) __strdup (s)
#  endif
# endif

# ifndef _HAVE_STRING_ARCH_strndup

extern char *__strndup (const char *__string, size_t __n)
     __THROW __attribute_malloc__;
#  define __strndup(s, n) \
  (__extension__ (__builtin_constant_p (s) && __string2_1bptr_p (s)	      \
		  ? (((const char *) (s))[0] == '\0'			      \
		     ? (char *) calloc ((size_t) 1, (size_t) 1)		      \
		     : ({ size_t __len = strlen (s) + 1;		      \
			  size_t __n = (n);				      \
			  char *__retval;				      \
			  if (__n < __len)				      \
			    __len = __n + 1;				      \
			  __retval = (char *) malloc (__len);		      \
			  if (__retval != NULL)				      \
			    {						      \
			      __retval[__len - 1] = '\0';		      \
			      __retval = (char *) memcpy (__retval, s,	      \
							  __len - 1);	      \
			    }						      \
			  __retval; }))					      \
		  : __strndup (s, n)))

#  ifdef __USE_XOPEN2K8
#   define strndup(s, n) __strndup (s, n)
#  endif
# endif

#endif /* Use misc. or use GNU.  */

#ifndef _FORCE_INLINES
# undef __STRING_INLINE
#endif

#endif /* No string inlines.  */
