/* Definition for thread-local data handling.  linuxthreads/i386 version.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#ifndef _TLS_H
#define _TLS_H

#include <stddef.h>

/* Type for the dtv.  */
typedef union dtv
{
  size_t counter;
  void *pointer;
} dtv_t;


typedef struct
{
  void *tcb;
  dtv_t *dtv;
} tcbhead_t;


/* Get the thread descriptor definition.  */
#include <linuxthreads/descr.h>


/* We can support TLS only if the floating-stack support is available.  */
#if FLOATING_STACKS && defined HAVE_TLS_SUPPORT

/* Get system call information.  */
# include <sysdep.h>

/* Signal that TLS support is available.  */
# define USE_TLS	1

/* This is the size of the initial TCB.  */
# define TLS_INIT_TCB_SIZE sizeof (tcbhead_t)

/* Alignment requirements for the initial TCB.  */
# define TLS_INIT_TCB_ALIGN __alignof__ (tcbhead_t)

/* This is the size of the TCB.  */
# define TLS_TCB_SIZE sizeof (struct _pthread_descr_struct)

/* Alignment requirements for the TCB.  */
# define TLS_TCB_ALIGN __alignof__ (struct _pthread_descr_struct)

/* The TCB can have any size and the memory following the address the
   thread pointer points to is unspecified.  Allocate the TCB there.  */
# define TLS_TCB_AT_TP	1

/* Code to initially initialize the thread pointer.  This might need
   special attention since 'errno' is not yet available and if the
   operation can cause a failure 'errno' must not be touched.  */
# define TLS_INIT_TP(descr, dtvp) \
  do {									      \
    void *_descr = (descr);						      \
    struct modify_ldt_ldt_s ldt_entry =					      \
      { 0, (unsigned long int) _descr, 1048576, 1, 0, 0, 1, 0, 1, 0 };	      \
    int result;								      \
    tcbhead_t *head = _descr;						      \
									      \
    head->tcb = _descr;							      \
    head->dtv = dtvp;							      \
									      \
    asm ("pushl %%ebx\n\t"						      \
	 "movl $1, %%ebx\n\t"						      \
	 "int $0x80\n\t"						      \
	 "popl %%ebx"							      \
	 : "=a" (result)						      \
	 : "0" (__NR_modify_ldt), "d" (sizeof (ldt_entry)), "c" (&ldt_entry));\
									      \
    if (__builtin_expect (result, 0) != 0)				      \
      /* Nothing else we can do.  */					      \
      asm ("hlt");							      \
  } while (0)


/* Return the address of the dtv for the current thread.  */
# define THREAD_DTV() \
  ({ struct _pthread_descr_struct *__descr;				      \
     THREAD_GETMEM (__descr, p_header.data.dtvp); })


#endif

#endif	/* tls.h */
