/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <stdarg.h>

/* Perform process tracing functions.  REQUEST is one of the values
   in <sys/ptrace.h>, and determines the action to be taken.
   For all requests except PTRACE_TRACEME, PID specifies the process to be
   traced.

   PID and the other arguments described above for the various requests should
   appear (those that are used for the particular request) as:
     pid_t PID, void *ADDR, int DATA, void *ADDR2
   after PID.  */
int
DEFUN(ptrace, (request), enum __ptrace_request request DOTS)
{
  pid_t pid;
  PTR addr;
  PTR addr2;
  int data;
  va_list ap;

  switch (request)
    {
    case PTRACE_TRACEME:
    case PTRACE_CONT:
    case PTRACE_KILL:
    case PTRACE_SINGLESTEP:
    case PTRACE_ATTACH:
    case PTRACE_DETACH:
      break;

    case PTRACE_PEEKTEXT:
    case PTRACE_PEEKDATA:
    case PTRACE_PEEKUSER:
    case PTRACE_GETREGS:
    case PTRACE_SETREGS:
#ifdef PTRACE_GETFPREGS
    case PTRACE_GETFPGEGS:
#endif
    case PTRACE_SETFPREGS:
    case PTRACE_GETFPAREGS:
    case PTRACE_SETFPAREGS:
      va_start(ap, request);
      pid = va_arg(ap, pid_t);
      addr = va_arg(ap, PTR);
      va_end(ap);
      break;

    case PTRACE_POKETEXT:
    case PTRACE_POKEDATA:
    case PTRACE_POKEUSER:
      va_start(ap, request);
      pid = va_arg(ap, pid_t);
      addr = va_arg(ap, PTR);
      data = va_arg(ap, int);
      va_end(ap);
      break;

    case PTRACE_READDATA:
    case PTRACE_WRITEDATA:
    case PTRACE_READTEXT:
    case PTRACE_WRITETEXT:
      va_start(ap, request);
      pid = va_arg(ap, pid_t);
      addr = va_arg(ap, PTR);
      data = va_arg(ap, int);
      addr2 = va_arg(ap, PTR);
      va_end(ap);
      break;

    default:
      errno = EINVAL;
      return -1;
    }

  errno = ENOSYS;
  return -1;
}


#ifdef	 HAVE_GNU_LD

#include <gnu-stabs.h>

stub_warning(ptrace);

#endif	/* GNU stabs.  */
