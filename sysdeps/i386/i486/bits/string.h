/* Optimized, inlined string functions.  i486 version.
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
		  : __memcpy_g (dest, src, n)))
#define __memcpy_c(dest, src, n) \
  (((n) % 4 == 0)							      \
   ? __memcpy_by4 (dest, src, n)					      \
   : (((n) % 2 == 0)							      \
      ? __memcpy_by2 (dest, src, n)					      \
      : __memcpy_g (dest, src, n)))

__STRING_INLINE void *
__memcpy_by4 (void *__dest, __const void *__src, size_t __n)
{
  register void *__tmp = __dest;
  register int __dummy1, __dummy2;
  __asm__ __volatile__
    ("1:\n\t"
     "movl	(%2),%0\n\t"
     "leal	4(%2),%2\n\t"
     "movl	%0,(%1)\n\t"
     "leal	4(%1),%1\n\t"
     "decl	%3\n\t"
     "jnz	1b"
     : "=r" (__dummy1), "=r" (__tmp), "=r" (__src), "=r" (__dummy2)
     : "1" (__tmp), "2" (__src), "3" (__n / 4)
     : "memory", "cc");
  return __dest;
}

__STRING_INLINE void *
__memcpy_by2 (void *__dest, __const void *__src, size_t __n)
{
  register void *__tmp = __dest;
  register int __dummy1, __dummy2;
  __asm__ __volatile__
    ("shrl	$1,%3\n\t"
     "jz	2f\n"                 /* only a word */
     "1:\n\t"
     "movl	(%2),%0\n\t"
     "leal	4(%2),%2\n\t"
     "movl	%0,(%1)\n\t"
     "leal	4(%1),%1\n\t"
     "decl	%3\n\t"
     "jnz	1b\n"
     "2:\n\t"
     "movw	(%2),%w0\n\t"
     "movw	%w0,(%1)"
     : "=q" (__dummy1), "=r" (__tmp), "=r" (__src), "=r" (__dummy2)
     : "1" (__tmp), "2" (__src), "3" (__n / 2)
     : "memory", "cc");
  return __dest;
}

__STRING_INLINE void *
__memcpy_g (void *__dest, __const void *__src, size_t __n)
{
  register void *__tmp = __dest;
  __asm__ __volatile__
    ("cld\n\t"
     "shrl	$1,%%ecx\n\t"
     "jnc	1f\n\t"
     "movsb\n"
     "1:\n\t"
     "shrl	$1,%%ecx\n\t"
     "jnc	2f\n\t"
     "movsw\n"
     "2:\n\t"
     "rep; movsl"
     : /* no output */
     : "c" (__n), "D" (__tmp),"S" (__src)
     : "cx", "di", "si", "memory", "cc");
  return __dest;
}


/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
__STRING_INLINE void *
memmove (void *__dest, __const void *__src, size_t __n)
{
  register void *__tmp = __dest;
  if (__dest < __src)
    __asm__ __volatile__
      ("cld\n\t"
       "rep; movsb"
       : /* no output */
       : "c" (__n), "S" (__src), "D" (__tmp)
       : "cx", "si", "di");
  else
    __asm__ __volatile__
      ("std\n\t"
       "rep; movsb\n\t"
       "cld"
       : /* no output */
       : "c" (__n), "S" (__n - 1 + (__const char *) __src),
	 "D" (__n - 1 + (char *) __tmp)
       : "cx", "si", "di", "memory");
  return __dest;
}


/* Compare N bytes of S1 and S2.  */
#ifndef __PIC__
/* gcc has problems to spill registers when using PIC.  */
__STRING_INLINE int
memcmp (__const void *__s1, __const void *__s2, size_t __n)
{
  register int __res;
  __asm__ __volatile__
    ("cld\n\t"
     "repe; cmpsb\n\t"
     "je	1f\n\t"
     "sbbl	%0,%0\n\t"
     "orb	$1,%b0\n"
     "1:"
     : "=a" (__res)
     : "0" (0), "S" (__s1), "D" (__s2), "c" (__n)
     : "si", "di", "cx", "cc");
  return __res;
}
#endif


/* Set N bytes of S to C.  */
#define memset(s, c, n) \
  (__extension__ (__builtin_constant_p (c)				      \
		  ? (__builtin_constant_p (n)				      \
		     ? __memset_cc (s, c, n)				      \
		     : __memset_cg (s, c, n))				      \
		  : (__builtin_constant_p (n)				      \
		     ? __memset_gc (s, c, n)				      \
		     : __memset_gg (s, c, n))))
#define __memset_cc(s, c, n) \
  (((n) % 4 == 0)							      \
   ? __memset_cc_by4 (s, c, n)						      \
   : (((n) % 2== 0)							      \
      ? __memset_cc_by2 (s, c, n)					      \
      : __memset_cg (s, c, n)))
#define __memset_gc(s, c, n) \
  (((n) % 4== 0)							      \
   ? __memset_gc_by4 (s, c, n)						      \
   : (((n) % 2 == 0)							      \
      ? __memset_gc_by2 (s, c, n)					      \
      : __memset_gg (s, c, n)))

__STRING_INLINE void *
__memset_cc_by4 (void *__s, int __c, size_t __n)
{
  register char *__tmp = __s;
  register int __dummy;
  __asm__ __volatile__
    ("1:\n\t"
     "movl	%2,(%0)\n\t"
     "leal	4(%0),%0\n\t"
     "decl	%1\n\t"
     "jnz	1b"
     : "=r" (__tmp), "=r" (__dummy)
     : "q" (0x01010101UL * (unsigned char) __c), "0" (__tmp), "1" (__n / 4)
     : "memory", "cc");
  return __s;
}

__STRING_INLINE void *
__memset_cc_by2 (void *__s, char __c, size_t __n)
{
  register void *__tmp = __s;
  register int __dummy;
  __asm__ __volatile__
    ("shrl	$1,%1\n\t"	/* may be divisible also by 4 */
     "jz	2f\n"
     "1:\n\t"
     "movl	%2,(%0)\n\t"
     "leal	4(%0),%0\n\t"
     "decl	%1\n\t"
     "jnz	1b\n"
     "2:\n\t"
     "movw	%w2,(%0)"
     : "=r" (__tmp), "=r" (__dummy)
     : "q" (0x01010101UL * (unsigned char) __c), "0" (__tmp), "1" (__n / 2)
     : "memory", "cc");
  return __s;
}

__STRING_INLINE void *
__memset_gc_by4 (void *__s, char __c, size_t __n)
{
  register void *__tmp = __s;
  register int __dummy;
  __asm__ __volatile__
    ("movb	%b0,%h0\n"
     "pushw	%w0\n\t"
     "shll	$16,%0\n\t"
     "popw	%w0\n"
     "1:\n\t"
     "movl	%0,(%1)\n\t"
     "addl	$4,%1\n\t"
     "decl	%2\n\t"
     "jnz	1b\n"
     : "=q" (__c), "=r" (__tmp), "=r" (__dummy)
     : "0" ((unsigned int) __c), "1" (__tmp), "2" (__n / 4)
     : "memory", "cc");
  return __s;
}

__STRING_INLINE void *
__memset_gc_by2 (void *__s, char __c, size_t __n)
{
  register void *__tmp = __s;
  register int __dummy1, __dummy2;
  __asm__ __volatile__
    ("movb	%b0,%h0\n\t"
     "shrl	$1,%2\n\t"	/* may be divisible also by 4 */
     "jz	2f\n\t"
     "pushw	%w0\n\t"
     "shll	$16,%0\n\t"
     "popw	%w0\n"
     "1:\n\t"
     "movl	%0,(%1)\n\t"
     "leal	4(%1),%1\n\t"
     "decl	%2\n\t"
     "jnz	1b\n"
     "2:\n\t"
     "movw	%w0,(%1)"
     : "=q" (__dummy1), "=r" (__tmp), "=r" (__dummy2)
     : "0" ((unsigned int) __c), "1" (__tmp), "2" (__n / 2)
     : "memory", "cc");
  return __s;
}

__STRING_INLINE void *
__memset_cg (void *__s, char __c, size_t __n)
{
  register void *__tmp = __s;
  __asm__ __volatile__
    ("shrl	$1,%%ecx\n\t"
     "rep; stosw\n\t"
     "jnc	1f\n\t"
     "movb	%%al,(%%edi)\n"
     "1:"
     : /* no output */
     : "c" (__n),"D" (__tmp), "a" (0x0101U * (unsigned char) __c)
     : "cx", "di", "memory", "cc");
  return __s;
}

__STRING_INLINE void *
__memset_gg (void *__s, char __c, size_t __n)
{
  register void *__tmp = __s;
  __asm__ __volatile__
    ("movb	%%al,%%ah\n\t"
     "shrl	$1,%%ecx\n\t"
     "rep; stosw\n\t"
     "jnc	1f\n\t"
     "movb	%%al,(%%edi)\n"
     "1:"
     : /* no output */
     : "c" (__n), "D" (__tmp), "a" (__c)
     : "cx", "di", "memory", "cc");
  return __s;
}


/* Search N bytes of S for C.  */
__STRING_INLINE void *
memchr (__const void *__s, int __c, size_t __n)
{
  register void *__res;
  if (__n == 0)
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


/* Return the length of S.  */
__STRING_INLINE size_t
strlen (__const char *__str)
{
  register char __dummy;
  register __const char *__tmp = __str;
  __asm__ __volatile__
    ("1:\n\t"
     "movb	(%0),%1\n\t"
     "leal	1(%0),%0\n\t"
     "testb	%1,%1\n\t"
     "jne	1b"
     : "=r" (__tmp), "=q" (__dummy)
     : "0" (__str)
     : "memory", "cc" );
  return __tmp - __str - 1;
}


/* Copy SRC to DEST.  */
__STRING_INLINE char *
strcpy (char *__dest, __const char *__src)
{
  register char *__tmp = __dest;
  register char __dummy;
  __asm__ __volatile__
    (
     "1:\n\t"
     "movb	(%0),%2\n\t"
     "incl	%0\n\t"
     "movb	%2,(%1)\n\t"
     "incl	%1\n\t"
     "testb	%2,%2\n\t"
     "jne	1b"
     : "=r" (__src), "=r" (__tmp), "=q" (__dummy)
     : "0" (__src), "1" (__tmp)
     : "memory", "cc");
  return __dest;
}


/* Copy no more than N characters of SRC to DEST.  */
__STRING_INLINE char *
strncpy (char *__dest, __const char *__src, size_t __n)
{
  register char *__tmp = __dest;
  register char __dummy;
  if (__n > 0)
    __asm__ __volatile__
      ("1:\n\t"
       "movb	(%0),%2\n\t"
       "incl	%0\n\t"
       "movb	%2,(%1)\n\t"
       "incl	%1\n\t"
       "decl	%3\n\t"
       "je	3f\n\t"
       "testb	%2,%2\n\t"
       "jne	1b\n\t"
       "2:\n\t"
       "movb	%2,(%1)\n\t"
       "incl	%1\n\t"
       "decl	%3\n\t"
       "jne	2b\n\t"
       "3:"
       : "=r" (__src), "=r" (__tmp), "=q" (__dummy), "=r" (__n)
       : "0" (__src), "1" (__tmp), "3" (__n)
       : "memory", "cc");

  return __dest;
}


/* Append SRC onto DEST.  */
__STRING_INLINE char *
strcat (char *__dest, __const char *__src)
{
  register char *__tmp = __dest - 1;
  register char __dummy;
  __asm__ __volatile__
    (
     "1:\n\t"
     "incl	%1\n\t"
     "cmpb	$0,(%1)\n\t"
     "jne	1b\n"
     "2:\n\t"
     "movb	(%2),%b0\n\t"
     "incl	%2\n\t"
     "movb	%b0,(%1)\n\t"
     "incl	%1\n\t"
     "testb	%b0,%b0\n\t"
     "jne	2b\n"
     : "=q" (__dummy), "=r" (__tmp), "=r" (__src)
     : "1"  (__tmp), "2"  (__src)
     : "memory", "cc");
  return __dest;
}


/* Append no more than N characters from SRC onto DEST.  */
__STRING_INLINE char *
strncat (char *__dest, __const char *__src, size_t __n)
{
  register char *__tmp = __dest - 1;
  register char __dummy;
  __asm__ __volatile__
    (
     "1:\n\t"
     "incl	%1\n\t"
     "cmpb	$0,(%1)\n\t"
     "jne	1b\n"
     "2:\n\t"
     "decl	%3\n\t"
     "js	3f\n\t"
     "movb	(%2),%b0\n\t"
     "leal	1(%2),%2\n\t"
     "movb	%b0,(%1)\n\t"
     "leal	1(%1),%1\n\t"
     "testb	%b0,%b0\n\t"
     "jne	2b\n"
     "3:\n\t"
     "movb	$0,(%1)\n\t"
     : "=q" (__dummy), "=r" (__tmp), "=r" (__src), "=r" (__n)
     : "1" (__tmp), "2" (__src), "3" (__n)
     : "memory", "cc");
  return __dest;
}


/* Compare S1 and S2.  */
__STRING_INLINE int
strcmp (__const char *__s1, __const char *__s2)
{
  register int __res;
  __asm__ __volatile__
    ("1:\n\t"
     "movb	(%1),%b0\n\t"
     "leal	1(%1),%1\n\t"
     "cmpb	%b0,(%2)\n\t"
     "jne	2f\n\t"
     "leal	1(%2),%2\n\t"
     "testb	%b0,%b0\n\t"
     "jne	1b\n\t"
     "xorl	%0,%0\n\t"
     "jmp	3f\n"
     "2:\n\t"
     "movl	$1,%0\n\t"
     "jb	3f\n\t"
     "negl	%0\n"
     "3:"
     : "=q" (__res), "=r" (__s1), "=r" (__s2)
     : "1" (__s1), "2" (__s2)
     : "cc");
  return __res;
}


/* Compare N characters of S1 and S2.  */
__STRING_INLINE int
strncmp (__const char *__s1, __const char *__s2, size_t __n)
{
  register int __res;
  __asm__ __volatile__
    ("1:\n\t"
     "decl	%3\n\t"
     "js	2f\n\t"
     "movb	(%1),%b0\n\t"
     "incl	%1\n\t"
     "cmpb	%b0,(%2)\n\t"
     "jne	3f\n\t"
     "incl	%2\n\t"
     "testb	%b0,%b0\n\t"
     "jne	1b\n"
     "2:\n\t"
     "xorl	%0,%0\n\t"
     "jmp	4f\n"
     "3:\n\t"
     "movl	$1,%0\n\t"
     "jb	4f\n\t"
     "negl	%0\n"
     "4:"
     : "=q" (__res), "=r" (__s1), "=r" (__s2), "=r" (__n)
     : "1"  (__s1), "2"  (__s2),  "3" (__n)
     : "cc");
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
    ("movb	%%al,%%ah\n"
     "1:\n\t"
     "movb	(%0),%%al\n\t"
     "cmpb	%%ah,%%al\n\t"
     "je	2f\n\t"
     "leal	1(%0),%0\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n\t"
     "xorl	%0,%0\n"
     "2:"
     : "=r" (__res)
     : "a" (__c), "0" (__s)
     : "ax", "cc");
  return __res;
}

__STRING_INLINE char *
__strchr_c (__const char *__s, int __c)
{
  register char *__res;
  __asm__ __volatile__
    ("1:\n\t"
     "movb	(%0),%%al\n\t"
     "cmpb	%%ah,%%al\n\t"
     "je	2f\n\t"
     "leal	1(%0),%0\n\t"
     "testb	%%al,%%al\n\t"
     "jne	1b\n\t"
     "xorl	%0,%0\n"
     "2:"
     : "=r" (__res)
     : "a" (__c), "0" (__s)
     : "ax", "cc");
  return __res;
}


/* Find the last occurrence of C in S.  */
#define strrchr(s, c) \
  (__extension__ (__builtin_constant_p (c)				      \
		  ? __strrchr_c (s, ((c) & 0xff) << 8)			      \
		  : __strrchr_g (s, c)))

__STRING_INLINE char *
__strrchr_g (__const char *__s, int __c)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "movb	%%al,%%ah\n"
     "1:\n\t"
     "lodsb\n\t"
     "cmpb	%%ah,%%al\n\t"
     "jne	2f\n\t"
     "leal	-1(%%esi),%0\n"
     "2:\n\t"
     "testb	%%al,%%al\n\t"
     "jne 1b"
     : "=d" (__res)
     : "0" (0), "S" (__s),"a" (__c)
     : "ax", "si", "cc");
  return __res;
}

__STRING_INLINE char *
__strrchr_c (__const char *__s, int __c)
{
  register char *__res;
  __asm__ __volatile__
    ("cld\n\t"
     "1:\n\t"
     "lodsb\n\t"
     "cmpb	%%ah,%%al\n\t"
     "jne	2f\n\t"
     "leal	-1(%%esi),%0\n"
     "2:\n\t"
     "testb	%%al,%%al\n\t"
     "jne 1b"
     : "=d" (__res)
     : "0" (0), "S" (__s),"a" (__c)
     : "ax", "si", "cc");
  return __res;
}


/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
#ifdef __PIC__
__STRING_INLINE size_t
strcspn (__const char *__s, __const char *__reject)
{
  register char *__res;
  __asm__ __volatile__
    ("push	%%ebx\n\t"
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
     : "a" (0), "c" (0xffffffff), "0" (__s), "g" (__reject)
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
     : "a" (0), "c" (0xffffffff), "0" (__s), "g" (__reject)
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
     : "a" (0), "c" (0xffffffff), "0" (__s), "g" (__accept)
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
     : "a" (0), "c" (0xffffffff), "0" (__s), "g" (__accept)
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
     : "0" (0), "c" (0xffffffff), "S" (__haystack), "g" (__needle)
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
