/* Copyright (C) 1995-1998,2000,2003,2006 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <stdarg.h>
#include <signal.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

long int
ptrace (enum __ptrace_request request, ...)
{
  long int res, ret;
  va_list ap;
  pid_t pid;
  void *addr, *data;

  va_start (ap, request);
  pid = va_arg (ap, pid_t);
  addr = va_arg (ap, void *);
  data = va_arg (ap, void *);
  va_end (ap);

  if (request > 0 && request < 4)
    data = &ret;

#if __BOUNDED_POINTERS__
  switch (request)
    {
    case PTRACE_PEEKTEXT:
    case PTRACE_PEEKDATA:
    case PTRACE_PEEKUSER:
    case PTRACE_POKETEXT:
    case PTRACE_POKEDATA:
    case PTRACE_POKEUSER:
      (void) CHECK_1 ((int *) addr);
      (void) CHECK_1 ((int *) data);
      break;

    case PTRACE_GETREGS:
    case PTRACE_SETREGS:
#ifdef __i386__
      (void) CHECK_1 ((struct user_regs_struct *) data);
#else
      /* We don't know the size of data, so the best we can do is ensure
	 that `data' is valid for at least one word.  */
      (void) CHECK_1 ((int *) data);
#endif
      break;

    case PTRACE_GETFPREGS:
    case PTRACE_SETFPREGS:
#ifdef __i386__
      (void) CHECK_1 ((struct user_fpregs_struct *) data);
#else
      /* We don't know the size of data, so the best we can do is ensure
	 that `data' is valid for at least one word.  */
      (void) CHECK_1 ((int *) data);
#endif
      break;

    case PTRACE_GETFPXREGS:
    case PTRACE_SETFPXREGS:
#ifdef __i386__
      (void) CHECK_1 ((struct user_fpxregs_struct *) data);
#else
      /* We don't know the size of data, so the best we can do is ensure
	 that `data' is valid for at least one word.  */
      (void) CHECK_1 ((int *) data);
#endif
      break;

    case PTRACE_GETSIGINFO:
    case PTRACE_SETSIGINFO:
      (void) CHECK_1 ((siginfo_t *) data);
      break;

    case PTRACE_GETEVENTMSG:
      (void) CHECK_1 ((unsigned long *) data);
      break;

    case PTRACE_SETOPTIONS:
      (void) CHECK_1 ((long *) data);
      break;

    case PTRACE_TRACEME:
    case PTRACE_CONT:
    case PTRACE_KILL:
    case PTRACE_SINGLESTEP:
    case PTRACE_ATTACH:
    case PTRACE_DETACH:
    case PTRACE_SYSCALL:
      /* Neither `data' nor `addr' needs any checks.  */
      break;
    };
#endif

  res = INLINE_SYSCALL (ptrace, 4, request, pid,
			__ptrvalue (addr), __ptrvalue (data));
  if (res >= 0 && request > 0 && request < 4)
    {
      __set_errno (0);
      return ret;
    }

  return res;
}
