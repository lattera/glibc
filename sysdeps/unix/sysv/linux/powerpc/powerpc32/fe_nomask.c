/* Procedure definition for FE_NOMASK_ENV for Linux/ppc.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sysdep.h>
#include <sys/prctl.h>
#include "kernel-features.h"

#if __ASSUME_NEW_PRCTL_SYSCALL == 0
/* This is rather fiddly under Linux.  We don't have direct access,
   and there is no system call, but we can change the bits 
   in a signal handler's context...  */

static struct sigaction oact;

static void
fe_nomask_handler (int signum, struct sigcontext *sc)
{
  sc->regs->msr |= 0x900ul;  /* FE0 | FE1 */
  sigaction (SIGUSR1, &oact, NULL);
}
#endif

const fenv_t *
__fe_nomask_env (void)
{
#if __ASSUME_NEW_PRCTL_SYSCALL == 0
# if defined PR_SET_FPEXC && defined PR_FP_EXC_PRECISE
  int result = INLINE_SYSCALL (prctl, 2, PR_SET_FPEXC, PR_FP_EXC_PRECISE);

  if (result == -1 && errno == EINVAL)
# endif
    {
      struct sigaction act;

      act.sa_handler = (sighandler_t) fe_nomask_handler;
      sigemptyset (&act.sa_mask);
      act.sa_flags = 0;

      sigaction (SIGUSR1, &act, &oact);
      raise (SIGUSR1);
    }
#else
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC, PR_FP_EXC_PRECISE);
#endif

  return FE_ENABLED_ENV;
}
