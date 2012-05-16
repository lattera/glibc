/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

/* Definitions of offsets within the ucontext_t structure.  Note
   that for convenience we use __SIZEOF_POINTER__ for "long" and
   "ssize_t" fields (and their unsigned counterparts) as well.  */

#define UC_FLAGS_OFFSET 0
#define UC_LINK_OFFSET (UC_FLAGS_OFFSET + __SIZEOF_POINTER__)
#define UC_STACK_SP_OFFSET (UC_LINK_OFFSET + __SIZEOF_POINTER__)
#define UC_STACK_FLAGS_OFFSET (UC_STACK_SP_OFFSET + __SIZEOF_POINTER__)
#define UC_STACK_SIZE_OFFSET (UC_STACK_FLAGS_OFFSET + __SIZEOF_POINTER__)
#define UC_STACK_MCONTEXT_OFFSET \
  ((UC_STACK_SIZE_OFFSET + __SIZEOF_POINTER__ + REGSIZE - 1) & -REGSIZE)
#define UC_REG(i) (UC_STACK_MCONTEXT_OFFSET + ((i) * REGSIZE))
#define UC_NREGS 64
#define UC_SIGMASK_OFFSET UC_REG(UC_NREGS)
#define UC_SIZE (UC_SIGMASK_OFFSET + (_NSIG / 8))

/* From <asm/siginfo.h> */
#define SI_MAX_SIZE	128

/* From <asm/signal.h> */
#define _NSIG		64
#define SIG_BLOCK          0	/* for blocking signals */
#define SIG_UNBLOCK        1	/* for unblocking signals */
#define SIG_SETMASK        2	/* for setting the signal mask */
