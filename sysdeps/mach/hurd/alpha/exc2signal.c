/* Translate Mach exception codes into signal numbers.  Alpha version.
   Copyright (C) 1994,97,2002 Free Software Foundation, Inc.
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

    case EXC_BAD_INSTRUCTION:
      *signo = SIGILL;
      detail->code = detail->exc_code;
      break;

    case EXC_ARITHMETIC:
      *signo = SIGFPE;
      detail->code = detail->exc_code;
      break;
      break;

    case EXC_EMULATION:
      /* 3.0 doesn't give this one, why, I don't know.  */
      *signo = SIGEMT;
      detail->code = detail->exc_code;
      break;

    case EXC_SOFTWARE:
      *signo = SIGEMT;
      detail->code = detail->exc_code;
      break;

    case EXC_BREAKPOINT:
      *signo = SIGTRAP;
      detail->code = detail->exc_code;
      break;
    }
}
