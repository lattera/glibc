/* Copyright (C) 1999-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>, 1999.

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

#include <sys/ucontext.h>

#define SIGCONTEXT siginfo_t *_si, ucontext_t *

/* The sigcontext structure changed between 2.0 and 2.1 kernels.  On any
   modern system we should be able to assume that the "new" format will be
   in use.  */

#define GET_PC(ctx)	((void *) (ctx)->uc_mcontext.arm_pc)

/* There is no reliable way to get the sigcontext unless we use a
   three-argument signal handler.  */
#define __sigaction(sig, act, oact) ({ \
  (act)->sa_flags |= SA_SIGINFO; \
  (__sigaction) (sig, act, oact); \
})

#define sigaction(sig, act, oact) ({ \
  (act)->sa_flags |= SA_SIGINFO; \
  (sigaction) (sig, act, oact); \
})
