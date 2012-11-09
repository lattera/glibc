/* Copyright (C) 2009-2012 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <tcb-offsets.h>

/* Save the PID value.  */
#define SAVE_PID							\
	mrs	x2, tpidr_el0;						\
	sub	x2, x2, #PTHREAD_SIZEOF;				\
	ldr	w3, [x2, #PTHREAD_PID_OFFSET];				\
	neg	w0, w3;							\
	str	w0, [x2, #PTHREAD_PID_OFFSET]

/* Restore the old PID value in the parent.  */
#define RESTORE_PID                                                     \
	cbz	x0, 1f;							\
	str	w3, [x2, #PTHREAD_PID_OFFSET];				\
1:

#include "../vfork.S"
