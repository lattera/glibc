/* memcopy.h -- definitions for memory copy functions.  Motorola 68020 version.
   Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Torbjorn Granlund (tege@sics.se).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdeps/generic/memcopy.h>

#if	defined(__mc68020__) || defined(mc68020)

#undef	OP_T_THRES
#define	OP_T_THRES	16

/* WORD_COPY_FWD and WORD_COPY_BWD are not symmetric on the 68020,
   because of its weird instruction overlap characteristics.  */

#undef	WORD_COPY_FWD
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		      \
  do									      \
    {									      \
      size_t __nwords = (nbytes) / sizeof (op_t);			      \
      size_t __nblocks = __nwords / 8 + 1;				      \
      dst_bp -= (8 - __nwords % 8) * sizeof (op_t);			      \
      src_bp -= (8 - __nwords % 8) * sizeof (op_t);			      \
      switch (__nwords % 8)						      \
	do								      \
	  {								      \
	    ((op_t *) dst_bp)[0] = ((op_t *) src_bp)[0];		      \
	  case 7:							      \
	    ((op_t *) dst_bp)[1] = ((op_t *) src_bp)[1];		      \
	  case 6:							      \
	    ((op_t *) dst_bp)[2] = ((op_t *) src_bp)[2];		      \
	  case 5:							      \
	    ((op_t *) dst_bp)[3] = ((op_t *) src_bp)[3];		      \
	  case 4:							      \
	    ((op_t *) dst_bp)[4] = ((op_t *) src_bp)[4];		      \
	  case 3:							      \
	    ((op_t *) dst_bp)[5] = ((op_t *) src_bp)[5];		      \
	  case 2:							      \
	    ((op_t *) dst_bp)[6] = ((op_t *) src_bp)[6];		      \
	  case 1:							      \
	    ((op_t *) dst_bp)[7] = ((op_t *) src_bp)[7];		      \
	  case 0:							      \
	    src_bp += 32;						      \
	    dst_bp += 32;						      \
	    __nblocks--;						      \
	  }								      \
      while (__nblocks != 0);						      \
      (nbytes_left) = (nbytes) % sizeof (op_t);				      \
    } while (0)

#undef	WORD_COPY_BWD
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)		      \
  do									      \
    {									      \
      size_t __nblocks = (nbytes) / 32 + 1;				      \
      op_t *__dst_ep = (op_t *) (dst_ep);				      \
      op_t *__src_ep = (op_t *) (src_ep);				      \
      switch ((nbytes) / sizeof (op_t) % 8)				      \
	do								      \
	  {								      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 7:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 6:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 5:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 4:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 3:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 2:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 1:							      \
	    *--__dst_ep = *--__src_ep;					      \
	  case 0:							      \
	    __nblocks--;						      \
	  }								      \
      while (__nblocks != 0);						      \
      (nbytes_left) = (nbytes) % sizeof (op_t);				      \
      (dst_ep) = (unsigned long) __dst_ep;				      \
      (src_ep) = (unsigned long) __src_ep;				      \
    } while (0)

#endif
