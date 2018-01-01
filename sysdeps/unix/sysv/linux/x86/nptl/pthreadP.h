/* Internal pthread header.  Linux/x86 version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include_next <nptl/pthreadP.h>

#ifndef _PTHREADP_H_X86
#define _PTHREADP_H_X86 1

extern struct pthread_unwind_buf ____pthread_unwind_buf_private;

_Static_assert (sizeof (____pthread_unwind_buf_private.cancel_jmp_buf)
		>= sizeof (struct __jmp_buf_tag),
		"size of cancel_jmp_buf < sizeof __jmp_buf_tag");

extern __pthread_unwind_buf_t ____pthread_unwind_buf;

_Static_assert (sizeof (____pthread_unwind_buf.__cancel_jmp_buf)
		>= sizeof (struct __jmp_buf_tag),
		"size of __cancel_jmp_buf < sizeof __jmp_buf_tag");

#endif
