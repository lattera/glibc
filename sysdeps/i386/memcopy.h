/* memcopy.h -- definitions for memory copy functions.  i386 version.
   Copyright (C) 1991 Free Software Foundation, Inc.
   Contributed by Torbjorn Granlund (tege@sics.se).

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

#include <sysdeps/generic/memcopy.h>

#undef	OP_T_THRES
#define	OP_T_THRES	8

#undef	BYTE_COPY_FWD
#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes)				      \
  asm volatile(/* Clear the direction flag, so copying goes forward.  */      \
	       "cld\n"							      \
	       /* Copy bytes.  */					      \
	       "rep\n"							      \
	       "movsb" :						      \
	       "=D" (dst_bp), "=S" (src_bp) :				      \
	       "0" (dst_bp), "1" (src_bp), "c" (nbytes) :		      \
	       "cx")

#undef	BYTE_COPY_BWD
#define BYTE_COPY_BWD(dst_ep, src_ep, nbytes)				      \
  do									      \
    {									      \
      asm volatile(/* Set the direction flag, so copying goes backwards.  */  \
		   "std\n"						      \
		   /* Copy bytes.  */					      \
		   "rep\n"						      \
		   "movsb\n"						      \
		   /* Clear the dir flag.  Convention says it should be 0. */ \
		   "cld" :						      \
		   "=D" (dst_ep), "=S" (src_ep) :			      \
		   "0" (dst_ep - 1), "1" (src_ep - 1), "c" (nbytes) :	      \
		   "cx");						      \
      dst_ep += 1;							      \
      src_ep += 1;							      \
    } while (0)

#undef	WORD_COPY_FWD
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		      \
  do									      \
    {									      \
      asm volatile(/* Clear the direction flag, so copying goes forward.  */  \
		   "cld\n"						      \
		   /* Copy longwords.  */				      \
		   "rep\n"						      \
		   "movsl" :						      \
		   "=D" (dst_bp), "=S" (src_bp) :			      \
		   "0" (dst_bp), "1" (src_bp), "c" ((nbytes) / 4) :	      \
		   "cx");						      \
      (nbytes_left) = (nbytes) % 4;					      \
    } while (0)

#undef	WORD_COPY_BWD
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)		      \
  do									      \
    {									      \
      asm volatile(/* Set the direction flag, so copying goes backwards.  */  \
		   "std\n"						      \
		   /* Copy longwords.  */				      \
		   "rep\n"						      \
		   "movsl\n"						      \
		   /* Clear the dir flag.  Convention says it should be 0. */ \
		   "cld" :						      \
		   "=D" (dst_ep), "=S" (src_ep) :			      \
		   "0" (dst_ep - 4), "1" (src_ep - 4), "c" ((nbytes) / 4) :   \
		   "cx");						      \
      dst_ep += 4;							      \
      src_ep += 4;							      \
      (nbytes_left) = (nbytes) % 4;					      \
    } while (0)
