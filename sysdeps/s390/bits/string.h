/* Optimized, inlined string functions.	 s390 version.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.	*/

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
# ifdef __cplusplus
#  define __STRING_INLINE inline
# else
#  define __STRING_INLINE extern __inline
# endif
#endif

#define _HAVE_STRING_ARCH_strlen 1
__STRING_INLINE size_t
strlen (__const char *__str)
{
    size_t __len;

    __asm__ __volatile__ ("   sr    0,0\n"
			  "   lr    %0,%1\n"
			  "0: srst  0,%0\n"
			  "   jo    0b\n"
			  "   lr    %0,0\n"
			  "   sr    %0,%1"
			  : "=&a" (__len) : "a" (__str)
			  : "cc", "0" );
    return __len;
}

/* Copy SRC to DEST.  */
#define _HAVE_STRING_ARCH_strcpy 1
__STRING_INLINE char *
strcpy (char *__dest, __const char *__src)
{
    char *tmp = __dest;

    __asm__ __volatile__ ("   sr    0,0\n"
			  "0: mvst  %0,%1\n"
			  "   jo    0b"
			  : "+&a" (__dest), "+&a" (__src) :
			  : "cc", "memory", "0" );
    return tmp;
}

#define _HAVE_STRING_ARCH_strncpy 1
__STRING_INLINE char *
strncpy (char *__dest, __const char *__src, size_t __n)
{
    char *tmp = __dest;

    if (__n <= 0)
	return tmp;
    __asm__ __volatile ("   slr	 %0,%1\n"
			"0: icm	 0,1,0(%1)\n"
			"   stc	 0,0(%0,%1)\n"
			"   jz	 2f\n"
			"   la	 %1,1(%1)\n"
			"   brct %2,0b\n"
			"   j	 3f\n"
			"1: la	 %1,1(%1)\n"
			"   stc	 0,0(%0,%1)\n"
			"2: brct %2,1b\n"
			"3:"
			: "+&a" (__dest), "+&a" (__src), "+&d" (__n) :
			: "cc", "memory", "0" );
    return tmp;
}

/* Append SRC onto DEST.  */
#define _HAVE_STRING_ARCH_strcat 1
__STRING_INLINE char *
strcat(char *__dest, const char *__src)
{
    char *tmp = __dest;

    __asm__ __volatile__ ("   sr   0,0\n"
			  "0: srst 0,%0\n"
			  "   jo   0b\n"
			  "   lr   %0,0\n"
			  "   sr   0,0\n"
			  "1: mvst %0,%1\n"
			  "   jo   1b"
			  : "+&a" (__dest), "+&a" (__src) :
			  : "cc", "memory", "0" );
    return tmp;
}

/* Append no more than N characters from SRC onto DEST.	 */
#define _HAVE_STRING_ARCH_strncat 1
__STRING_INLINE char *
strncat (char *__dest, __const char *__src, size_t __n)
{
    char *tmp = __dest;

    if (__n <= 0)
	return tmp;
    __asm__ __volatile__ ("   sr   0,0\n"
			  "0: srst 0,%0\n"
			  "   jo   0b\n"
			  "   lr   %0,0\n"
			  "   slr  %0,%1\n"
			  "1: icm  0,1,0(%1)\n"
			  "   stc  0,0(%0,%1)\n"
			  "   jz   2f\n"
			  "   la   %1,1(%1)\n"
			  "   brct %2,1b\n"
			  "   la   %0,0(%0,%1)\n"
			  "   xc   0(1,%0),0(%0)\n"
			  "2:"
			  : "+&a" (__dest), "+&a" (__src), "+&d" (__n) :
			  : "cc", "memory", "0" );
    return tmp;
}

#endif	/* Use string inlines && GNU CC.  */
