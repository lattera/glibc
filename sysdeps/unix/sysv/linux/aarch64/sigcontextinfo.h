/* AArch64 definitions for signal handling calling conventions.
   Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

#include <stdint.h>
#include <sys/ucontext.h>

#define SIGCONTEXT siginfo_t *_si, ucontext_t *
#define GET_PC(ctx) ((void *) (uintptr_t) (ctx)->uc_mcontext.pc)

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
