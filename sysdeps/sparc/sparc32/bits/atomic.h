/* Atomic operations.  sparc32 version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

#ifndef _BITS_ATOMIC_H
#define _BITS_ATOMIC_H	1

/* We have no compare and swap, just test and set.
   The following implementation contends on 64 global locks
   per library and assumes no variable will be accessed using atomic.h
   macros from two different libraries.  */

__make_section_unallocated
  (".gnu.linkonce.b.__sparc32_atomic_locks, \"aw\", %nobits");

volatile unsigned char __sparc32_atomic_locks[64]
  __attribute__ ((nocommon, section (".gnu.linkonce.b.__sparc32_atomic_locks"
				     __sec_comment),
		  visibility ("hidden")));

#define __sparc32_atomic_do_lock(addr) \
  do								      \
    {								      \
      unsigned int __old_lock;					      \
      unsigned int __idx = (((long) addr >> 2) ^ ((long) addr >> 12)) \
			   & 63;				      \
      do							      \
	__asm ("ldstub %1, %0"					      \
	       : "=r" (__old_lock),				      \
		 "=m" (__sparc32_atomic_locks[__idx])		      \
	       : "m" (__sparc32_atomic_locks[__idx]));		      \
      while (__old_lock);					      \
    }								      \
  while (0)

#define __sparc32_atomic_do_unlock(addr) \
  do								      \
    __sparc32_atomic_locks[(((long) addr >> 2)			      \
			    ^ ((long) addr >> 12)) & 63] = 0;	      \
  while (0)

/* The only basic operation needed is compare and exchange.  */
#define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  ({ __typeof (mem) __acev_memp = (mem);			      \
     __typeof (*mem) __acev_ret;				      \
     __typeof (*mem) __acev_newval = (newval);			      \
								      \
     __sparc32_atomic_do_lock (__acev_memp);			      \
     __acev_ret = *__acev_memp;					      \
     if (__acev_ret == (oldval))				      \
       *__acev_memp = __acev_newval;				      \
     __sparc32_atomic_do_unlock (__acev_memp);			      \
     __acev_ret; })

#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  ({ __typeof (mem) __aceb_memp = (mem);			      \
     int __aceb_ret;						      \
     __typeof (*mem) __aceb_newval = (newval);			      \
								      \
     __sparc32_atomic_do_lock (__aceb_memp);			      \
     __aceb_ret = 0;						      \
     if (*__aceb_memp == (oldval))				      \
       *__aceb_memp = __aceb_newval;				      \
     else							      \
       __aceb_ret = 1;						      \
     __sparc32_atomic_do_unlock (__aceb_memp);			      \
     __aceb_ret; })

#endif	/* bits/atomic.h */
