/* Translate Mach exception codes into signal numbers.  Stub version.
   Copyright (C) 1991-2015 Free Software Foundation, Inc.
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

#include <hurd.h>

/* This file must be modified with machine-dependent details.  */
#error "need to write sysdeps/mach/hurd/MACHINE/exc2signal.c"

/* Translate the Mach exception codes, as received in an `exception_raise' RPC,
   into a signal number and signal subcode.  */

void
_hurd_exception2signal (int exception, int code, int subcode,
			int *signo, int *sigcode, int *error)
{
  *error = 0;

  switch (exception)
    {
    default:
      *signo = SIGIOT;
      *sigcode = exception;
      break;

    case EXC_BAD_ACCESS:
      if (code == KERN_PROTECTION_FAILURE)
	*signo = SIGSEGV;
      else
	*signo = SIGBUS;
      *sigcode = subcode;
      *error = code;
      break;

    case EXC_BAD_INSTRUCTION:
      *signo = SIGILL;
      *sigcode = 0;
      break;

    case EXC_ARITHMETIC:
      *signo = SIGFPE;
      *sigcode = 0;
      break;

    case EXC_EMULATION:
    case EXC_SOFTWARE:
      *signo = SIGEMT;
      *sigcode = 0;
      break;

    case EXC_BREAKPOINT:
      *signo = SIGTRAP;
      *sigcode = 0;
      break;
    }
}
