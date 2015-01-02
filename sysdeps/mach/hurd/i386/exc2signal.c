/* Translate Mach exception codes into signal numbers.  i386 version.
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
      if (detail->exc_code == KERN_INVALID_ADDRESS
	  || detail->exc_code == KERN_PROTECTION_FAILURE
	  || detail->exc_code == KERN_WRITE_PROTECTION_FAILURE)
	*signo = SIGSEGV;
      else
	*signo = SIGBUS;
      detail->code = detail->exc_subcode;
      detail->error = detail->exc_code;
      break;

    case EXC_BAD_INSTRUCTION:
      *signo = SIGILL;
      if (detail->exc_code == EXC_I386_INVOP)
	detail->code = ILL_INVOPR_FAULT;
      else if (detail->exc_code == EXC_I386_STKFLT)
	detail->code = ILL_STACK_FAULT;
      else
	detail->code = 0;
      break;

    case EXC_ARITHMETIC:
      switch (detail->exc_code)
	{
	case EXC_I386_DIV:	/* integer divide by zero */
	  *signo = SIGFPE;
	  detail->code = FPE_INTDIV_FAULT;
	  break;

	case EXC_I386_INTO:	/* integer overflow */
	  *signo = SIGFPE;
	  detail->code = FPE_INTOVF_TRAP;
	  break;

	  /* These aren't anywhere documented or used in Mach 3.0.  */
	case EXC_I386_NOEXT:
	case EXC_I386_EXTOVR:
	default:
	  *signo = SIGFPE;
	  detail->code = 0;
	  break;

	case EXC_I386_EXTERR:
	  /* Subcode is the fp_status word saved by the hardware.
	     Give an error code corresponding to the first bit set.  */
	  if (detail->exc_subcode & FPS_IE)
	    {
	      *signo = SIGILL;
	      detail->code = ILL_FPEOPR_FAULT;
	    }
	  else if (detail->exc_subcode & FPS_DE)
	    {
	      *signo = SIGFPE;
	      detail->code = FPE_FLTDNR_FAULT;
	    }
	  else if (detail->exc_subcode & FPS_ZE)
	    {
	      *signo = SIGFPE;
	      detail->code = FPE_FLTDIV_FAULT;
	    }
	  else if (detail->exc_subcode & FPS_OE)
	    {
	      *signo = SIGFPE;
	      detail->code = FPE_FLTOVF_FAULT;
	    }
	  else if (detail->exc_subcode & FPS_UE)
	    {
	      *signo = SIGFPE;
	      detail->code = FPE_FLTUND_FAULT;
	    }
	  else if (detail->exc_subcode & FPS_PE)
	    {
	      *signo = SIGFPE;
	      detail->code = FPE_FLTINX_FAULT;
	    }
	  else
	    {
	      *signo = SIGFPE;
	      detail->code = 0;
	    }
	  break;

	  /* These two can only be arithmetic exceptions if we
	     are in V86 mode, which sounds like emulation to me.
	     (See Mach 3.0 i386/trap.c.)  */
	case EXC_I386_EMERR:
	  *signo = SIGFPE;
	  detail->code = FPE_EMERR_FAULT;
	  break;
	case EXC_I386_BOUND:
	  *signo = SIGFPE;
	  detail->code = FPE_EMBND_FAULT;
	  break;
	}
      break;

    case EXC_EMULATION:
      /* 3.0 doesn't give this one, why, I don't know.  */
      *signo = SIGEMT;
      detail->code = 0;
      break;

    case EXC_SOFTWARE:
      /* The only time we get this in Mach 3.0
	 is for an out of bounds trap.  */
      if (detail->exc_code == EXC_I386_BOUND)
	{
	  *signo = SIGFPE;
	  detail->code = FPE_SUBRNG_FAULT;
	}
      else
	{
	  *signo = SIGEMT;
	  detail->code = 0;
	}
      break;

    case EXC_BREAKPOINT:
      *signo = SIGTRAP;
      if (detail->exc_code == EXC_I386_SGL)
	detail->code = DBG_SINGLE_TRAP;
      else if (detail->exc_code == EXC_I386_BPT)
	detail->code = DBG_BRKPNT_FAULT;
      else
	detail->code = 0;
      break;
    }
}
