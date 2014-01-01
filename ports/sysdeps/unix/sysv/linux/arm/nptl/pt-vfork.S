/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <tcb-offsets.h>

/* Save the PID value.  */
#define SAVE_PID \
	GET_TLS (r2);							\
	NEGOFF_ADJ_BASE2 (r2, r0, PID_OFFSET); /* Save the TLS addr in r2. */ \
	ldr	r3, NEGOFF_OFF1 (r2, PID_OFFSET); /* Load the saved PID.  */  \
	rsb	r0, r3, #0;		/* Negate it.  */		     \
	str	r0, NEGOFF_OFF1 (r2, PID_OFFSET); /* Store the temp PID.  */

/* Restore the old PID value in the parent.  */
#define RESTORE_PID \
	cmp	r0, #0;			/* If we are the parent... */	\
	it	ne;							\
	strne	r3, NEGOFF_OFF1 (r2, PID_OFFSET); /* restore the saved PID.  */

#include "../vfork.S"
