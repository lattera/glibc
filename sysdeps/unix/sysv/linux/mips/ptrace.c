/* Copyright (C) 1995, 1996, 1997, 1998, 2000, 2002, 2003, 2004
	Free Software Foundation, Inc.
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
#include <sgidefs.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <stdarg.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>
#include <sgidefs.h>

#if _MIPS_SIM == _ABIN32
__extension__ typedef long long int reg_type;
#else
typedef long int reg_type;
#endif

reg_type
ptrace (enum __ptrace_request request, ...)
{
  reg_type res, ret;
  va_list ap;
  pid_t pid;
  void *addr;
  reg_type data;

  va_start (ap, request);
  pid = va_arg (ap, pid_t);
  addr = va_arg (ap, void *);
  data = va_arg (ap, reg_type);
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
      /* We don't know the size of data, so the best we can do is ensure
	 that `data' is valid for at least one word.  */
      (void) CHECK_1 ((int *) data);
      break;

    case PTRACE_GETFPREGS:
    case PTRACE_SETFPREGS:
      /* We don't know the size of data, so the best we can do is ensure
	 that `data' is valid for at least one word.  */
      (void) CHECK_1 ((int *) data);
      break;

    case PTRACE_GETFPXREGS:
    case PTRACE_SETFPXREGS:
      /* We don't know the size of data, so the best we can do is ensure
	 that `data' is valid for at least one word.  */
      (void) CHECK_1 ((int *) data);
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
