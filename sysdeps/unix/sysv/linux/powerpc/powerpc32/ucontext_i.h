/* Offsets and other constants needed in the *context() function
   implementation.
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

#define SIG_BLOCK	0
#define SIG_SETMASK	2

#define _FRAME_BACKCHAIN	0
#define _FRAME_LR_SAVE	4
#define _FRAME_PARM_SAVE1	8
#define _FRAME_PARM_SAVE2	12
#define _FRAME_PARM_SAVE3	16
#define _FRAME_PARM_SAVE4	20

#define _UC_LINK	4
#define _UC_STACK_SP	8
#define _UC_STACK_SIZE	16
#define _UC_REGS_PTR	48
#define _UC_SIGMASK	52
#define _UC_REG_SPACE	180

/* offsets within mcontext_t */
#define _UC_GREGS	0
#define _UC_FREGS	192
#define _UC_VREGS	464
#define _UC_VSCR	976
#define _UC_VRSAVE	980
