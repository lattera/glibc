/* memcopy.h -- definitions for memory copy functions.  Pentium version.
   Copyright (C) 1994 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* Get the i386 definitions.  We will override some of them below.  */
#include_next <memcopy.h>


/* Written like this, the Pentium pipeline can execute the loop at a
   sustained rate of 2 instructions/clock, or asymptotically 480
   Mbytes/second at 60Mhz.  */

#undef	WORD_COPY_FWD
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		\
  do									\
    {									\
      asm volatile ("subl	$32,%2\n"				\
		    "js		2f\n"					\
		    "1:\n"						\
		    "movl	0(%1),%%eax\n"				\
		    "movl	4(%1),%%edx\n"				\
		    "movl	%%eax,0(%0)\n"				\
		    "movl	%%edx,4(%0)\n"				\
		    "movl	8(%1),%%eax\n"				\
		    "movl	12(%1),%%edx\n"				\
		    "movl	%%eax,8(%0)\n"				\
		    "movl	%%edx,12(%0)\n"				\
		    "movl	16(%1),%%eax\n"				\
		    "movl	20(%1),%%edx\n"				\
		    "movl	%%eax,16(%0)\n"				\
		    "movl	%%edx,20(%0)\n"				\
		    "movl	24(%1),%%eax\n"				\
		    "movl	28(%1),%%edx\n"				\
		    "movl	%%eax,24(%0)\n"				\
		    "movl	%%edx,28(%0)\n"				\
		    "addl	$32,%1\n"				\
		    "addl	$32,%0\n"				\
		    "subl	$32,%2\n"				\
		    "jns	1b\n"					\
		    "2: addl	$32,%2" :				\
		    "=r" (dst_bp), "=r" (src_bp), "=r" (nbytes_left) :	\
		    "0" (dst_bp), "1" (src_bp), "2" (nbytes) :		\
		    "ax", "dx");					\
    } while (0)

#undef	WORD_COPY_BWD
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)		\
  do									\
    {									\
      asm volatile ("subl	$32,%2\n"				\
		    "js		2f\n"					\
		    "1:\n"						\
		    "movl	-4(%1),%%eax\n"				\
		    "movl	-8(%1),%%edx\n"				\
		    "movl	%%eax,-4(%0)\n"				\
		    "movl	%%edx,-8(%0)\n"				\
		    "movl	-12(%1),%%eax\n"			\
		    "movl	-16(%1),%%edx\n"			\
		    "movl	%%eax,-12(%0)\n"			\
		    "movl	%%edx,-16(%0)\n"			\
		    "movl	-20(%1),%%eax\n"			\
		    "movl	-24(%1),%%edx\n"			\
		    "movl	%%eax,-20(%0)\n"			\
		    "movl	%%edx,-24(%0)\n"			\
		    "movl	-28(%1),%%eax\n"			\
		    "movl	-32(%1),%%edx\n"			\
		    "movl	%%eax,-28(%0)\n"			\
		    "movl	%%edx,-32(%0)\n"			\
		    "subl	$32,%1\n"				\
		    "subl	$32,%0\n"				\
		    "subl	$32,%2\n"				\
		    "jns	1b\n"					\
		    "2: addl	$32,%2" :				\
		    "=r" (dst_bp), "=r" (src_bp), "=r" (nbytes_left) :	\
		    "0" (dst_bp), "1" (src_bp), "2" (nbytes) :		\
		    "ax", "dx");					\
    } while (0)
