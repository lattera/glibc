/* POSIX.1 `sigaction' call for Linux/i386.
   Copyright (C) 1991-2012 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <ldsodefs.h>

#include <kernel-features.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

/* We do not globally define the SA_RESTORER flag so do it here.  */
#define SA_RESTORER 0x04000000


/* Using the hidden attribute here does not change the code but it
   helps to avoid warnings.  */
#ifdef __NR_rt_sigaction
extern void restore_rt (void) asm ("__restore_rt") attribute_hidden;
#endif
extern void restore (void) asm ("__restore") attribute_hidden;


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
  int result;

  struct kernel_sigaction kact, koact;

  if (act)
    {
      kact.k_sa_handler = act->sa_handler;
      kact.sa_flags = act->sa_flags;
      memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));

      if (GLRO(dl_sysinfo_dso) == NULL)
	{
	  kact.sa_flags |= SA_RESTORER;

	  kact.sa_restorer = ((act->sa_flags & SA_SIGINFO)
			      ? &restore_rt : &restore);
	}
    }

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  result = INLINE_SYSCALL (rt_sigaction, 4,
			   sig, act ? __ptrvalue (&kact) : NULL,
			   oact ? __ptrvalue (&koact) : NULL, _NSIG / 8);

  if (oact && result >= 0)
    {
      oact->sa_handler = koact.k_sa_handler;
      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
      oact->sa_flags = koact.sa_flags;
      oact->sa_restorer = koact.sa_restorer;
    }
  return result;
}
libc_hidden_def (__libc_sigaction)

#ifdef WRAPPER_INCLUDE
# include WRAPPER_INCLUDE
#endif

#ifndef LIBC_SIGACTION
weak_alias (__libc_sigaction, __sigaction)
libc_hidden_weak (__sigaction)
weak_alias (__libc_sigaction, sigaction)
#endif

/* NOTE: Please think twice before making any changes to the bits of
   code below.  GDB needs some intimate knowledge about it to
   recognize them as signal trampolines, and make backtraces through
   signal handlers work right.  Important are both the names
   (__restore and __restore_rt) and the exact instruction sequence.
   If you ever feel the need to make any changes, please notify the
   appropriate GDB maintainer.  */

#define RESTORE(name, syscall) RESTORE2 (name, syscall)
#define RESTORE2(name, syscall) \
asm						\
  (						\
   ".text\n"					\
   "	.align 16\n"				\
   "__" #name ":\n"				\
   "	movl $" #syscall ", %eax\n"		\
   "	int  $0x80"				\
   );

#ifdef __NR_rt_sigaction
/* The return code for realtime-signals.  */
RESTORE (restore_rt, __NR_rt_sigreturn)
#endif

/* For the boring old signals.  */
#undef RESTORE2
#define RESTORE2(name, syscall) \
asm						\
  (						\
   ".text\n"					\
   "	.align 8\n"				\
   "__" #name ":\n"				\
   "	popl %eax\n"				\
   "	movl $" #syscall ", %eax\n"		\
   "	int  $0x80"				\
   );

RESTORE (restore, __NR_sigreturn)
