/* POSIX.1 `sigaction' call for Linux/i386.
   Copyright (C) 1991, 95, 96, 97, 98, 99, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <kernel-features.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

/* We do not globally define the SA_RESTORER flag so do it here.  */
#define SA_RESTORER 0x04000000


extern int __syscall_rt_sigaction (int, const struct kernel_sigaction *__unbounded,
				   struct kernel_sigaction *__unbounded, size_t);

#if __ASSUME_REALTIME_SIGNALS == 0
/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  */
int __libc_missing_rt_sigs;
#endif

#ifdef __NR_rt_sigaction
static void restore_rt (void) asm ("__restore_rt");
#endif
static void restore (void) asm ("__restore");


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
#if __ASSUME_REALTIME_SIGNALS == 0
  struct old_kernel_sigaction k_newact, k_oldact;
#endif
  int result;

#ifdef __NR_rt_sigaction

  /* First try the RT signals.  */
# if __ASSUME_REALTIME_SIGNALS == 0
  if (!__libc_missing_rt_sigs)
# endif
    {
      struct kernel_sigaction kact, koact;
# if __ASSUME_REALTIME_SIGNALS == 0
      int saved_errno = errno;
# endif

      if (act)
	{
	  kact.k_sa_handler = act->sa_handler;
	  memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
	  kact.sa_flags = act->sa_flags | SA_RESTORER;

	  kact.sa_restorer = ((act->sa_flags & SA_SIGINFO)
			      ? &restore_rt : &restore);
	}

      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      result = INLINE_SYSCALL (rt_sigaction, 4,
			       sig, act ? __ptrvalue (&kact) : NULL,
			       oact ? __ptrvalue (&koact) : NULL, _NSIG / 8);

# if __ASSUME_REALTIME_SIGNALS == 0
      if (result >= 0 || errno != ENOSYS)
# endif
	{
	  if (oact && result >= 0)
	    {
	      oact->sa_handler = koact.k_sa_handler;
	      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
	      oact->sa_flags = koact.sa_flags;
	      oact->sa_restorer = koact.sa_restorer;
	    }
	  return result;
	}

# if __ASSUME_REALTIME_SIGNALS == 0
      __set_errno (saved_errno);
      __libc_missing_rt_sigs = 1;
# endif
    }
#endif

#if __ASSUME_REALTIME_SIGNALS == 0
  if (act)
    {
      k_newact.k_sa_handler = act->sa_handler;
      k_newact.sa_mask = act->sa_mask.__val[0];
      k_newact.sa_flags = act->sa_flags | SA_RESTORER;

      k_newact.sa_restorer = &restore;
    }

  asm volatile ("pushl %%ebx\n"
		"movl %2, %%ebx\n"
		"int $0x80\n"
		"popl %%ebx"
		: "=a" (result)
		: "0" (SYS_ify (sigaction)), "r" (sig),
		  "c" (act ? __ptrvalue (&k_newact) : 0),
		  "d" (oact ? __ptrvalue (&k_oldact) : 0));

  if (result < 0)
    {
      __set_errno (-result);
      return -1;
    }

  if (oact)
    {
      oact->sa_handler = k_oldact.k_sa_handler;
      oact->sa_mask.__val[0] = k_oldact.sa_mask;
      oact->sa_flags = k_oldact.sa_flags;
      oact->sa_restorer = k_oldact.sa_restorer;
    }

  return 0;
#endif
}

weak_alias (__libc_sigaction, __sigaction)
weak_alias (__libc_sigaction, sigaction)

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
   ".align 16\n"				\
   "__" #name ":\n"				\
   "	movl $" #syscall ", %eax\n"		\
   "	int  $0x80"				\
   );

#ifdef __NR_rt_sigaction
/* The return code for realtime-signals.  */
RESTORE (restore_rt, __NR_rt_sigreturn)
#endif

/* For the boring old signals.  */
# undef RESTORE2
# define RESTORE2(name, syscall) \
asm						\
  (						\
   ".align 8\n"					\
   "__" #name ":\n"				\
   "	popl %eax\n"				\
   "	movl $" #syscall ", %eax\n"		\
   "	int  $0x80"				\
   );

RESTORE (restore, __NR_sigreturn)
