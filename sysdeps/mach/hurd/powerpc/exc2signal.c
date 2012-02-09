/* Translate Mach exception codes into signal numbers.  PowerPC version.
   Copyright (C) 1991,92,94,96,97,2001 Free Software Foundation, Inc.
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
#include <hurd/signal.h>
#include <mach/exception.h>

/* Translate the Mach exception codes, as received in an `exception_raise' RPC,
   into a signal number and signal subcode.  */

void
_hurd_exception2signal (struct hurd_signal_detail *detail, int *signo)
{
  detail->error = 0;

  switch (detail->exc)
    {
    default:
      *signo = SIGIOT;
      detail->code = detail->exc;
      break;

    case EXC_BAD_ACCESS:
      if (detail->exc_code == KERN_PROTECTION_FAILURE)
	*signo = SIGSEGV;
      else
	*signo = SIGBUS;
      detail->code = detail->exc_subcode;
      detail->error = detail->exc_code;
      break;

      /* XXX there has got to be something more here */

    case EXC_BAD_INSTRUCTION:
      *signo = SIGILL;
      detail->code = 0;
      break;

    case EXC_ARITHMETIC:
      *signo = SIGFPE;
      detail->code = 0;
      break;

    case EXC_EMULATION:
      *signo = SIGEMT;
      detail->code = 0;
      break;

    case EXC_SOFTWARE:
      *signo = SIGEMT;
      detail->code = 0;
      break;

    case EXC_BREAKPOINT:
      *signo = SIGTRAP;
      detail->code = 0;
    }
}
