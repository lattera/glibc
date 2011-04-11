/* Thread-local storage handling in the ELF dynamic linker.  s390 version.
   Copyright (C) 2003, 2004, 2011 Free Software Foundation, Inc.
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


/* Type used for the representation of TLS information in the GOT.  */
typedef struct
{
  unsigned long int ti_module;
  unsigned long int ti_offset;
} tls_index;


#ifdef SHARED
/* This is the prototype for the GNU version.  */
extern void *__tls_get_addr (tls_index *ti) attribute_hidden;
extern unsigned long __tls_get_offset (unsigned long got_offset);

# ifdef IS_IN_rtld
/* The special thing about the s390 TLS ABI is that we do not have the
   standard __tls_get_addr function but the __tls_get_offset function
   which differs in two important aspects:
   1) __tls_get_offset gets a got offset instead of a pointer to the
      tls_index structure
   2) __tls_get_offset returns the offset of the requested variable to
      the thread descriptor instead of a pointer to the variable.
 */
#  ifdef __s390x__
asm("\n\
	.text\n\
	.globl __tls_get_offset\n\
	.type __tls_get_offset, @function\n\
	.align 4\n\
__tls_get_offset:\n\
	la	%r2,0(%r2,%r12)\n\
	jg	__tls_get_addr\n\
");
#  elif defined __s390__
asm("\n\
	.text\n\
	.globl __tls_get_offset\n\
	.type __tls_get_offset, @function\n\
	.align 4\n\
__tls_get_offset:\n\
	basr	%r3,0\n\
0:	la	%r2,0(%r2,%r12)\n\
	l	%r4,1f-0b(%r3)\n\
	b	0(%r4,%r3)\n\
1:	.long	__tls_get_addr - 0b\n\
");
#  endif
# endif

# define GET_ADDR_OFFSET \
  (ti->ti_offset - (unsigned long) __builtin_thread_pointer ())

# define __TLS_GET_ADDR(__ti) \
  ({ extern char _GLOBAL_OFFSET_TABLE_[] attribute_hidden;		  \
     (void *) __tls_get_offset ((char *) (__ti) - _GLOBAL_OFFSET_TABLE_)  \
     + (unsigned long) __builtin_thread_pointer (); })

#endif

/* Value used for dtv entries for which the allocation is delayed.  */
#define TLS_DTV_UNALLOCATED	((void *) -1l)
