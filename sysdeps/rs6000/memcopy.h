/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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

#include <sysdeps/generic/memcopy.h>

#undef	OP_T_THRES
#define OP_T_THRES 32

#undef	BYTE_COPY_FWD
#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes)				      \
  do									      \
    {									      \
      size_t __nbytes = nbytes;						      \
      asm volatile("mtspr	1,%2\n"					      \
		   "lsx		6,0,%1\n"				      \
		   "stsx	6,0,%0" : /* No outputs.  */ :		      \
		   "b" (dst_bp), "b" (src_bp), "r" (__nbytes) :		      \
		   "6", "7", "8", "9", "10", "11", "12", "13");		      \
      dst_bp += __nbytes;						      \
      src_bp += __nbytes;						      \
    } while (0)

#undef	BYTE_COPY_BWD
#define BYTE_COPY_BWD(dst_ep, src_ep, nbytes)				      \
  do									      \
    {									      \
      size_t __nbytes = (nbytes);					      \
      dst_ep -= __nbytes;						      \
      src_ep -= __nbytes;						      \
      asm volatile("mtspr	1,%2\n"					      \
		   "lsx		6,0,%1\n"				      \
		   "stsx	6,0,%0" : /* No outputs.  */ :		      \
		   "b" (dst_ep), "b" (src_ep), "r" (__nbytes) :		      \
		   "6", "7", "8", "9", "10", "11", "12", "13");		      \
    } while (0)

#undef	WORD_COPY_FWD
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		      \
  do									      \
    {									      \
      size_t __nblocks = (nbytes) / 32;					      \
      if (__nblocks != 0)						      \
	asm volatile("mtctr	%4\n"					      \
		     "lsi	6,%1,32\n"				      \
		     "ai	%1,%1,32\n"				      \
		     "stsi	6,%0,32\n"				      \
		     "ai	%0,%0,32\n"				      \
		     "bdn	$-16" :					      \
		     "=b" (dst_bp), "=b" (src_bp) :			      \
		     "0" (dst_bp), "1" (src_bp), "r" (__nblocks) :	      \
		     "6", "7", "8", "9", "10", "11", "12", "13");	      \
      (nbytes_left) = (nbytes) % 32;					      \
    } while (0)

#undef	WORD_COPY_BWD
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)		      \
  do									      \
    {									      \
      size_t __nblocks = (nbytes) / 32;					      \
      if (__nblocks != 0)						      \
	asm volatile("mtctr	%4\n"					      \
		     "ai	%1,%1,-32\n"				      \
		     "lsi	6,%1,32\n"				      \
		     "ai	%0,%0,-32\n"				      \
		     "stsi	6,%0,32\n"				      \
		     "bdn	$-16" :					      \
		     "=b" (dst_ep), "=b" (src_ep) :			      \
		     "0" (dst_ep), "1" (src_ep), "r" (__nblocks) :	      \
		     "6", "7", "8", "9", "10", "11", "12", "13");	      \
      (nbytes_left) = (nbytes) % 32;					      \
    } while (0)
