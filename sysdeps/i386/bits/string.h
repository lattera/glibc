/* Optimized, inlined string functions.  i386 version.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#error "Never use <bits/string.h> directly; include <string.h> instead."
#endif

/* We only provide optimizations for the GNU CC.  */
#if defined __GNUC__ && __GNUC__ >= 2

#ifdef __cplusplus
# define __STRING_INLINE inline
#else
# define __STRING_INLINE extern __inline
#endif


/* Copy N bytes of SRC to DEST.  */
#define memcpy(dest, src, n) \
  (__extension__ (__builtin_constant_p (n)				      \
		  ? __memcpy_c (dest, src, n)				      \
		  : memcpy (dest, src, n)))
/* This looks horribly ugly, but the compiler can optimize it totally,
   as the count is constant.  */
__STRING_INLINE void *
__memcpy_c (void *__dest, __const void *__src, size_t __n)
{
  switch (__n)
    {
    case 0:
      return __dest;
    case 1:
      *(unsigned char *) __dest = *(const unsigned char *) __src;
      return __dest;
    case 2:
      *(unsigned short int *) __dest = *(const unsigned short int *) __src;
      return __dest;
    case 3:
      *(unsigned short int *) __dest = *(const unsigned short int *) __src;
      *(2 + (unsigned char *) __dest) = *(2 + (const unsigned char *) __src);
      return __dest;
    case 4:
      *(unsigned long int *) __dest = *(const unsigned long int *) __src;
      return __dest;
    case 6:	/* for ethernet addresses */
      *(unsigned long int *) __dest = *(const unsigned long int *) __src;
      *(2 + (unsigned short int *) __dest) =
	*(2 + (const unsigned short int *) __src);
      return __dest;
    case 8:
      *(unsigned long int *) __dest = *(const unsigned long int *) __to;
      *(1 + (unsigned long int *) __dest) =
	*(1 + (const unsigned long int *) __src);
      return __dest;
    case 12:
      *(unsigned long int *) __dest = *(const unsigned long int *) __src;
      *(1 + (unsigned long int *) __dest) =
	*(1 + (const unsigned long int *) __src);
      *(2 + (unsigned long int *) __dest) =
	*(2 + (const unsigned long int *) __src);
      return __dest;
    case 16:
      *(unsigned long int *) __dest = *(const unsigned long int *) __src;
      *(1 + (unsigned long int *) __dest) =
	*(1 + (const unsigned long int *) __src);
      *(2 + (unsigned long int *) __dest) =
	*(2 + (const unsigned long int *) __src);
      *(3 + (unsigned long int *) __dest) =
	*(3 + (const unsigned long int *) __src);
      return __dest;
    case 20:
      *(unsigned long int *) __dest = *(const unsigned long int *) __src;
      *(1 + (unsigned long int *) __dest) =
	*(1 + (const unsigned long int *) __src);
      *(2 + (unsigned long int *) __dest) =
	*(2 + (const unsigned long int *) __src);
      *(3 + (unsigned long int *) __dest) =
	*(3 + (const unsigned long int *) __src);
      *(4 + (unsigned long int *) __dest) =
	*(4 + (const unsigned long int *) __src);
      return __dest;
    }
#define __COMMON_CODE(x) \
  __asm__ __volatile__							      \
    ("cld\n\t"								      \
     "rep; movsl"							      \
     x									      \
     : /* no outputs */							      \
     : "c" (__n / 4), "D" (__dest), "S" (__src)				      \
     : "cx", "di", "si", "memory");

  switch (__n % 4)
    {
    case 0:
      __COMMON_CODE ("");
      return __dest;
    case 1:
      __COMMON_CODE ("\n\tmovsb");
      return __dest;
    case 2:
      __COMMON_CODE ("\n\tmovsw");
      return __dest;
    case 3:
      __COMMON_CODE ("\n\tmovsw\n\tmovsb");
      return __dest;
    }
#undef __COMMON_CODE
}


/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
__STRING_INLINE void *
memmove (void *__dest, __const void *__src, size_t __n)
{
  if (__dest < __src)
    __asm__ __volatile__
      ("cld\n\t"
       "rep\n\t"
       "movsb"
       : /* no output */
       : "c" (__n), "S" (__src),"D" (__dest)
       : "cx", "si", "di");
  else
    __asm__ __volatile__
      ("std\n\t"
       "rep\n\t"
       "movsb\n\t"
       "cld"
       : /* no output */
       : "c" (__n), "S" (__n - 1 + (const char *) __src),
	 "D" (__n - 1 + (char *) __dest)
       : "cx", "si", "di", "memory");
  return __dest;
}


/* Set N bytes of S to C.  */
#define memset(s, c, n) \
  (__extension__ (__builtin_constant_p (c)				      \
		  ? (__builtin_constant_p (n)				      \
		     ? __memset_cc (s, 0x01010101UL * (unsigned char) (c), n) \
		     : __memset_cg (s, 0x01010101UL * (unsigned char) (c), n))\
		  : __memset_gg (s, c, n)))

__STRING_INLINE void *
__memset_cc (void *__s, unsigned long int __pattern, size_t __n)
{
  switch (__n)
    {
    case 0:
      return s;
    case 1:
      *(unsigned char *) __s = __pattern;
      return __s;
    case 2:
      *(unsigned short int *) __s = __pattern;
      return s;
    case 3:
      *(unsigned short int *) __s = __pattern;
      *(2 + (unsigned char *) __s) = __pattern;
      return __s;
    case 4:
      *(unsigned long *) __s = __pattern;
      return __s;
	}
#define __COMMON_CODE(x) \
  __asm__ __volatile__							      \
    ("cld\n\t"								      \
     "rep; stosl"							      \
     x									      \
     : /* no outputs */							      \
     : "a" (__pattern),"c" (__n / 4), "D" (__s)				      \
     : "cx", "di", "memory")

  switch (__n % 4)
    {
    case 0:
      __COMMON_CODE ("");
      return __s;
    case 1:
      __COMMON_CODE ("\n\tstosb");
      return __s;
    case 2:
      __COMMON__CODE ("\n\tstosw");
      return s;
    case 3:
      __COMMON_CODE ("\n\tstosw\n\tstosb");
      return __s;
    }
#undef __COMMON_CODE
}

__STRING_INLINE void *
__memset_cg (void *__s, unsigned long __c, size_t __n)
{
  __asm__ __volatile__
    ("cld\n\t"
     "rep; stosl\n\t"
     "testb	$2,%b1\n\t"
     "je	1f\n\t"
     "stosw\n"
     "1:\n\t"
     "testb	$1,%b1\n\t"
     "je	2f\n\t"
     "stosb\n"
     "2:"
     : /* no output */
     : "a" (__c), "q" (__n), "c" (__n / 4), "D" (__s)
     : "cx", "di", "memory");
  return __s;
}

__STRING_INLINE void *
__memset_gg (void *__s, char __c, size_t __n)
{
  __asm__ __volatile__
    ("cld\n\t"
     "rep; stosb"
     : /* no output */
     : "a" (__c),"D" (__s), "c" (__n)
     : "cx", "di", "memory");
  return __s;
}




/* Search N bytes of S for C.  */
__STRING_INLINE void *
memchr (__const void *__s, int __c, size_t __n)
{
  register void *__res;
  if (count == 0)
    return NULL;
  __asm__ __volatile__
    ("cld\n\t"
     "repne; scasb\n\t"
     "je	1f\n\t"
     "movl	$1,%0\n"
     "1:\n\t"
     "decl	%0"
     : "=D" (__res)
     : "a" (__c), "D" (__s), "c" (__n)
     : "cx", "cc");
  return __res;
}


/* Search N bytes of S for C.  */
__STRING_INLINE void *
memchr (__const void *__s, int __c, size_t __n)
{
  register void *__res;
  if (count == 0)
    return NULL;
  __asm__ __volatile__
    ("cld\n\t"
     "repne\n\t"
     "scasb\n\t"
     "je	1f\n\t"
     "movl	$1,%0\n"
     "1:"
     : "=D" (__res)
     : "a" (__c), "0" (__s), "c" (__n)
     : "cx");
  return __res - 1;
}


/* Return the length of S.  */
__STRING_INLINE size_t
strlen (__const char *__str)
{
  register size_t __res;
  __asm__ __volatile__
    ("cld\n\t"
     "repne; scasb\n\t"
     "notl %0"
     : "=c" (__res)
     : "D" (__str), "a" (0), "0" (0xffffffff)
     : "di", "cc");
  return __res - 1;
}


/* Copy SRC to DEST.  */
__STRING_INLINE char *
strcpy (char *__dest, __const char *__src)
{
  __asm__ __volatile__
    ("cld\n"
     "1:\n\t"
     "lodsb\n\t"
     "stosb\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b"
     : /* no output */
     : "S" (__src), "D" (__dest)
     : "si", "di", "ax", "memory", "cc");
  return __dest;
}


/* Copy no more than N characters of SRC to DEST.  */
__STRING_INLINE char *
strncpy (char *__dest, __const char *__src, size_t __n)
{
  __asm__ __volatile__
    ("cld\n"
     "1:\n\t"
     "decl	%2\n\t"
     "js	2f\n\t"
     "lodsb\n\t"
     "stosb\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n\t"
     "rep; stosb\n"
     "2:"
     : /* no output */
     : "S" (__src), "D" (__dest), "c" (__n)
     : "si", "di", "ax", "cx", "memory", "cc");
  return __dest;
}


/* Append SRC onto DEST.  */
__STRING_INLINE char *
strcat (char *__dest, __const char *__src)
{
  __asm__ __volatile__
    ("cld\n\t"
     "repne; scasb\n\t"
     "decl	%1\n"
     "1:\n\t"
     "lodsb\n\t"
     "stosb\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b"
     : /* no output */
     : "S" (__src), "D" (__dest), "a" (0), "c" (0xffffffff)
     : "si", "di", "ax", "cx", "memory", "cc");
  return __dest;
}


/* Append no more than N characters from SRC onto DEST.  */
__STRING_INLINE char *
strncat (char *__dest, __const char *__src, size_t __n)
{
  __asm__ __volatile__
    ("cld\n\t"
     "repne; scasb\n\t"
     "decl	%1\n\t"
     "movl	%4,%3\n"
     "1:\n\t"
     "decl	%3\n\t"
     "js	2f\n\t"
     "lodsb\n\t"
     "stosb\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n"
     "2:\n\t"
     "xorl	%2,%2\n\t"
     "stosb"
     : /* no output */
     : "S" (__src), "D" (__dest), "a" (0), "c" (0xffffffff), "g" (__n)
     : "si", "di", "ax", "cx", "memory", "cc");
  return __dest;
}


/* Compare S1 and S2.  */
__STRING_INLINE int
strcmp (__const char *__s1, __const char *__s2)
{
  register int __res;
  __asm__ __volatile__
    ("cld\n"
     "1:\n\t"
     "lodsb\n\t"
     "scasb\n\t"
     "jne	2f\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n\t"
     "xorl	%%eax,%%eax\n\t"
     "jmp	3f\n"
     "2:\n\t"
     "sbbl	%%eax,%%eax\n\t"
     "orb	$1,%%eax\n"
     "3:"
     : "=a" (__res)
     : "S" (__s1), "D" (__s2)
     : "si", "di", "cc");
  return __res;
}


/* Compare N characters of S1 and S2.  */
__STRING_INLINE int
strncmp (__const char *__s1, __const char *__s2, size_t __n)
{
  register int __res;
  __asm__ __volatile__
    ("cld\n"
     "1:\n\t"
     "decl	%3\n\t"
     "js	2f\n\t"
     "lodsb\n\t"
     "scasb\n\t"
     "jne	3f\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n"
     "2:\n\t"
     "xorl	%%eax,%%eax\n\t"
     "jmp	4f\n"
     "3:\n\t"
     "sbbl	%%eax,%%eax\n\t"
     "orb	$1,%%al\n"
     "4:"
     : "=a" (__res)
     : "S" (__s1), "D" (__s2), "c" (__n)
     : "si", "di", "cx", "cc");
  return __res;
}


/* Find the first occurrence of C in S.  */
#define strchr(s, c) \
  (__extension__ (__builtin_constant_p (c)				      \
		  ? __strchr_c (s, ((c) & 0xff) << 8)			      \
		  : __strchr_g (s, c)))

__STRING_INLINE char *
__strchr_g (__const char *__s, int __c)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "movb	%%al,%%ah\n"
     "1:\n\t"
     "lodsb\n\t"
     "cmpb	%%ah,%%al\n\t"
     "je	2f\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n\t"
     "movl	$1,%1\n"
     "2:\n\t"
     "movl	%1,%0"
     : "=a" (__res)
     : "S" (__s), "0" (__c)
     : "si", "cc");
  return __res - 1;
}

__STRING_INLINE char *
__strchr_c (__const char *__s, int __c)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "1:\n\t"
     "lodsb\n\t"
     "cmpb	%%ah,%%al\n\t"
     "je	2f\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n\t"
     "movl	$1,%1\n"
     "2:\n\t"
     "movl	%1,%0"
     : "=a" (__res)
     : "S" (__s), "0" (__c)
     : "si", "cc");
  return __res - 1;
}


/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
#ifdef __PIC__
__STRING_INLINE size_t
strcspn (__const char *__s, __const char *__reject)
{
  register char *__res;
  __asm__ __volatile__
    ("pushl	%%ebx\n\t"
     "cld\n\t"
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"
     "movl	%%ecx,%%ebx\n"
     "1:\n\t"
     "lodsb\n\t"
     "testb	%%al,%%al\n\t"
     "je	2f\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%ebx,%%ecx\n\t"
     "repne; scasb\n\t"
     "jne	1b\n"
     "2:\n\t"
     "popl	%%ebx"
     : "=S" (__res)
     : "a" (0), "c" (0xffffffff), "0" (__s), "r" (__reject)
     : "ax", "cx", "di", "cc");
  return (__res - 1) - __s;
}
#else
__STRING_INLINE size_t
strcspn (__const char *__s, __const char *__reject)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"
     "movl	%%ecx,%%edx\n"
     "1:\n\t"
     "lodsb\n\t"
     "testb	%%al,%%al\n\t"
     "je	2f\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%edx,%%ecx\n\t"
     "repne; scasb\n\t"
     "jne	1b\n"
     "2:"
     : "=S" (__res)
     : "a" (0), "c" (0xffffffff),"0" (__s), "g" (__reject)
     : "ax", "cx", "dx", "di", "cc");
  return (__res - 1) - __s;
}
#endif


/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
#ifdef __PIC__
__STRING_INLINE size_t
strspn (__const char *__s, __const char *__accept)
{
  register char *__res;
  __asm__ __volatile__
    ("pushl	%%ebx\n\t"
     "cld\n\t"
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"
     "movl	%%ecx,%%ebx\n"
     "1:\n\t"
     "lodsb\n\t"
     "testb	%%al,%%al\n\t"
     "je	2f\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%ebx,%%ecx\n\t"
     "repne; scasb\n\t"
     "je	1b\n"
     "2:\n\t"
     "popl	%%ebx"
     : "=S" (__res)
     : "a" (0), "c" (0xffffffff), "0" (__s), "r" (__accept)
     : "ax", "cx", "di", "cc");
  return (__res - 1) - __s;
}
#else
__STRING_INLINE size_t
strspn (__const char *__s, __const char *__accept)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"
     "movl	%%ecx,%%edx\n"
     "1:\n\t"
     "lodsb\n\t"
     "testb	%%al,%%al\n\t"
     "je	2f\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%edx,%%ecx\n\t"
     "repne; scasb\n\t"
     "je	1b\n"
     "2:"
     : "=S" (__res)
     : "a" (0), "c" (0xffffffff), "0" (__s), "g" (__accept)
     : "ax", "cx", "dx", "di", "cc");
  return (__res - 1) - __s;
}
#endif


/* Find the first occurrence in S of any character in ACCEPT.  */
#ifdef __PIC__
__STRING_INLINE char *
strpbrk (__const char *__s, __const char *__accept)
{
  register char *__res;
  __asm__ __volatile__
    ("pushl	%%ebx\n\t"
     "cld\n\t"
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"
     "movl	%%ecx,%%ebx\n"
     "1:\n\t"
     "lodsb\n\t"
     "testb	%%al,%%al\n\t"
     "je	2f\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%ebx,%%ecx\n\t"
     "repne; scasb\n\t"
     "jne	1b\n\t"
     "decl	%0\n\t"
     "jmp	3f\n"
     "2:\n\t"
     "xorl	%0,%0\n"
     "3:\n\t"
     "popl	%%ebx"
     : "=S" (__res)
     : "a" (0), "c" (0xffffffff), "0" (__s), "r" (__accept)
     : "ax", "cx", "di", "cc");
  return __res;
}
#else
__STRING_INLINE char *
strpbrk (__const char *__s, __const char *__accept)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"
     "movl	%%ecx,%%edx\n"
     "1:\n\t"
     "lodsb\n\t"
     "testb	%%al,%%al\n\t"
     "je	2f\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%edx,%%ecx\n\t"
     "repne; scasb\n\t"
     "jne	1b\n\t"
     "decl	%0\n\t"
     "jmp	3f\n"
     "2:\n\t"
     "xorl	%0,%0\n"
     "3:"
     : "=S" (__res)
     : "a" (0), "c" (0xffffffff), "0" (__s), "g" (__accept)
     : "ax", "cx", "dx", "di", "cc");
  return __res;
}
#endif


/* Find the first occurrence of NEEDLE in HAYSTACK.  */
#ifdef __PIC__
__STRING_INLINE char *
strstr (__const char *__haystack, __const char *__needle)
{
  register char *__res;
  __asm__ __volatile__
    ("pushl	%%ebx\n\t"
     "cld\n\t" \
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"	/* NOTE! This also sets Z if searchstring='' */
     "movl	%%ecx,%%ebx\n"
     "1:\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%esi,%%eax\n\t"
     "movl	%%ebx,%%ecx\n\t"
     "repe; cmpsb\n\t"
     "je	2f\n\t"		/* also works for empty string, see above */
     "xchgl	%%eax,%%esi\n\t"
     "incl	%%esi\n\t"
     "cmpb	$0,-1(%%eax)\n\t"
     "jne	1b\n\t"
     "xorl	%%eax,%%eax\n\t"
     "2:\n\t"
     "popl	%%ebx"
     : "=a" (__res)
     : "0" (0), "c" (0xffffffff), "S" (__haystack), "r" (__needle)
     : "cx", "di", "si", "cc");
  return __res;
}
#else
__STRING_INLINE char *
strstr (__const char *__haystack, __const char *__needle)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t" \
     "movl	%4,%%edi\n\t"
     "repne; scasb\n\t"
     "notl	%%ecx\n\t"
     "decl	%%ecx\n\t"	/* NOTE! This also sets Z if searchstring='' */
     "movl	%%ecx,%%edx\n"
     "1:\n\t"
     "movl	%4,%%edi\n\t"
     "movl	%%esi,%%eax\n\t"
     "movl	%%edx,%%ecx\n\t"
     "repe; cmpsb\n\t"
     "je	2f\n\t"		/* also works for empty string, see above */
     "xchgl	%%eax,%%esi\n\t"
     "incl	%%esi\n\t"
     "cmpb	$0,-1(%%eax)\n\t"
     "jne	1b\n\t"
     "xorl	%%eax,%%eax\n\t"
     "2:"
     : "=a" (__res)
     : "0" (0), "c" (0xffffffff), "S" (__haystack), "g" (__needle)
     : "cx", "dx", "di", "si", "cc");
  return __res;
}
#endif


#undef __STRING_INLINE

#endif	/* GNU CC */
