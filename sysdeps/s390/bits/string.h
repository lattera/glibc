/* Optimized, inlined string functions.  S/390 version.
   Copyright (C) 2000, 2001, 2007 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

#ifndef _STRING_H
# error "Never use <bits/string.h> directly; include <string.h> instead."
#endif

/* The s390 processors can access unaligned multi-byte variables.  */
#define _STRING_ARCH_unaligned	1

/* We only provide optimizations if the user selects them and if
   GNU CC is used.  */
#if !defined __NO_STRING_INLINES && defined __USE_STRING_INLINES \
    && defined __GNUC__ && __GNUC__ >= 2

#ifndef __STRING_INLINE
# ifndef __extern_inline
#  define __STRING_INLINE inline
# else
#  define __STRING_INLINE __extern_inline
# endif
#endif

#define _HAVE_STRING_ARCH_strlen 1
#ifndef _FORCE_INLINES
#define strlen(str) __strlen_g ((str))

__STRING_INLINE size_t __strlen_g (__const char *) __asm__ ("strlen");

__STRING_INLINE size_t
__strlen_g (__const char *__str)
{
    char *__ptr, *__tmp;

    __ptr = (char *) 0;
    __tmp = (char *) __str;
    __asm__ __volatile__ ("   la    0,0\n"
			  "0: srst  %0,%1\n"
			  "   jo    0b\n"
			  : "+&a" (__ptr), "+&a" (__tmp) : 
			  : "cc", "memory", "0" );
    return (size_t) (__ptr - __str);
}
#endif

/* Copy SRC to DEST.  */
#define _HAVE_STRING_ARCH_strcpy 1
#ifndef _FORCE_INLINES
#define strcpy(dest, src) __strcpy_g ((dest), (src))

__STRING_INLINE char *__strcpy_g (char *, __const char *) __asm ("strcpy");

__STRING_INLINE char *
__strcpy_g (char *__dest, __const char *__src)
{
    char *tmp = __dest;

    __asm__ __volatile__ ("   la    0,0\n"
			  "0: mvst  %0,%1\n"
			  "   jo    0b"
			  : "+&a" (__dest), "+&a" (__src) :
			  : "cc", "memory", "0" );
    return tmp;
}
#endif

#define _HAVE_STRING_ARCH_strncpy 1
#ifndef _FORCE_INLINES
#define strncpy(dest, src, n) __strncpy_g ((dest), (src), (n))

__STRING_INLINE char *__strncpy_g (char *, __const char *, size_t)
     __asm__ ("strncpy");

__STRING_INLINE char *
__strncpy_g (char *__dest, __const char *__src, size_t __n)
{
    char *__ret = __dest;
    char *__ptr;
    size_t __diff;

    if (__n > 0) {
      __diff = (size_t) (__dest - __src);
      __ptr = (char *) __src;
      __asm__ __volatile__ ("   j     1f\n"
                            "0: la    %0,1(%0)\n"
                            "1: icm   0,1,0(%0)\n"
                            "   stc   0,0(%2,%0)\n"
                            "   jz    3f\n"
#if defined(__s390x__)
                            "   brctg %1,0b\n"
#else
                            "   brct  %1,0b\n"
#endif
                            "   j     4f\n"
                            "2: la    %0,1(%0)\n"
                            "   stc   0,0(%2,%0)\n"
#if defined(__s390x__)
                            "3: brctg %1,2b\n"
#else
                            "3: brct  %1,2b\n"
#endif
                            "4:"
                            : "+&a" (__ptr), "+&a" (__n) : "a" (__diff)
                            : "cc", "memory", "0" );
    }
    return __ret;
}
#endif

/* Append SRC onto DEST.  */
#define _HAVE_STRING_ARCH_strcat 1
#ifndef _FORCE_INLINES
#define strcat(dest, src) __strcat_g ((dest), (src))

__STRING_INLINE char *__strcat_g (char *, __const char *) __asm__ ("strcat");

__STRING_INLINE char *
__strcat_g (char *__dest, __const char *__src)
{
    char *__ret = __dest;
    char *__ptr, *__tmp;

    /* Move __ptr to the end of __dest.  */
    __ptr = (char *) 0;
    __tmp = __dest;
    __asm__ __volatile__ ("   la    0,0\n"
			  "0: srst  %0,%1\n"
			  "   jo    0b\n"
			  : "+&a" (__ptr), "+&a" (__tmp) :
			  : "cc", "0" );

    /* Now do the copy.  */
    __asm__ __volatile__ ("   la    0,0\n"
			  "0: mvst  %0,%1\n"
			  "   jo    0b"
			  : "+&a" (__ptr), "+&a" (__src) :
			  : "cc", "memory", "0" );
    return __ret;
}
#endif

/* Append no more than N characters from SRC onto DEST.  */
#define _HAVE_STRING_ARCH_strncat 1
#ifndef _FORCE_INLINES
#define strncat(dest, src, n) __strncat_g ((dest), (src), (n))

__STRING_INLINE char *__strncat_g (char *, __const char *, size_t)
     __asm__ ("strncat");

__STRING_INLINE char *
__strncat_g (char *__dest, __const char *__src, size_t __n)
{
    char *__ret = __dest;
    char *__ptr, *__tmp;
    size_t __diff;

    if (__n > 0) {
      /* Move __ptr to the end of __dest.  */
      __ptr = (char *) 0;
      __tmp = __dest;
      __asm__ __volatile__ ("   la    0,0\n"
			    "0: srst  %0,%1\n"
			  "   jo    0b\n"
			    : "+&a" (__ptr), "+&a" (__tmp) :
			    : "cc", "memory", "0" );

      __diff = (size_t) (__ptr - __src);
      __tmp = (char *) __src;
      __asm__ __volatile__ ("   j     1f\n"
                            "0: la    %0,1(%0)\n"
                            "1: icm   0,1,0(%0)\n"
                            "   stc   0,0(%2,%0)\n"
                            "   jz    2f\n"
#if defined(__s390x__)
                            "   brctg %1,0b\n"
#else
                            "   brct  %1,0b\n"
#endif
			    "   slr   0,0\n"
                            "   stc   0,1(%2,%0)\n"
			    "2:"
                            : "+&a" (__tmp), "+&a" (__n) : "a" (__diff)
                            : "cc", "memory", "0" );

    }
    return __ret;
}
#endif

/* Search N bytes of S for C.  */
#define _HAVE_STRING_ARCH_memchr 1
#ifndef _FORCE_INLINES
__STRING_INLINE void *
memchr (__const void *__str, int __c, size_t __n)
{
    char *__ptr, *__tmp;

    __tmp = (char *) __str;
    __ptr = (char *) __tmp + __n;
    __asm__ __volatile__ ("   lhi   0,0xff\n"
			  "   nr    0,%2\n"
			  "0: srst  %0,%1\n"
			  "   jo    0b\n"
                          "   brc   13,1f\n"
                          "   la    %0,0\n"
                          "1:"
			  : "+&a" (__ptr), "+&a" (__tmp) : "d" (__c)
			  : "cc", "memory", "0" );
    return __ptr;
}
#endif

/* Search N bytes of S for C.  */
#define _HAVE_STRING_ARCH_memchr 1
#ifndef _FORCE_INLINES
__STRING_INLINE int
strcmp (__const char *__s1, __const char *__s2)
{
    char *__p1, *__p2;
    int __ret;

    __p1 = (char *) __s1;
    __p2 = (char *) __s2;
    __asm__ __volatile__ ("   slr   0,0\n"
                          "0: clst  %1,%2\n"
			  "   jo    0b\n"
			  "   ipm   %0\n"
			  "   srl   %0,28"
			  : "=d" (__ret), "+&a" (__p1), "+&a" (__p2) : 
			  : "cc", "memory", "0" );
    __ret = (__ret == 0) ? 0 : (__ret == 1) ? -1 : 1;
    return __ret;
}
#endif

#endif	/* Use string inlines && GNU CC.  */
