/* Translate Mach exception codes into signal numbers.  i386 version.
Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <hurd/signal.h>
#include <mach/exception.h>

/* Translate the Mach exception codes, as received in an `exception_raise' RPC,
   into a signal number and signal subcode.  */

void
_hurd_exception2signal (int exception, int code, int subcode,
			int *signo, long int *sigcode, int *error)
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
      if (code == EXC_I386_INVOP)
	*sigcode = ILL_INVOPR_FAULT;
      else if (code == EXC_I386_STKFLT)
	*sigcode = ILL_STACK_FAULT;
      else
	*sigcode = 0;
      break;
      
    case EXC_ARITHMETIC:
      switch (code)
	{
	case EXC_I386_DIV:	/* integer divide by zero */
	  *signo = SIGFPE;
	  *sigcode = FPE_INTDIV_FAULT;
	  break;
	  
	case EXC_I386_INTO:	/* integer overflow */
	  *signo = SIGFPE;
	  *sigcode = FPE_INTOVF_TRAP;
	  break;

	  /* These aren't anywhere documented or used in Mach 3.0.  */
	case EXC_I386_NOEXT:
	case EXC_I386_EXTOVR:
	default:
	  *signo = SIGFPE;
	  *sigcode = 0;
	  break;

	case EXC_I386_EXTERR:
	  /* Subcode is the fp_status word saved by the hardware.
	     Give an error code corresponding to the first bit set.  */
	  if (subcode & FPS_IE)
	    {
	      *signo = SIGILL;
	      *sigcode = ILL_FPEOPR_FAULT;
	    }
	  else if (subcode & FPS_DE)
	    {
	      *signo = SIGFPE;
	      *sigcode = FPE_FLTDNR_FAULT;
	    }
	  else if (subcode & FPS_ZE)
	    {
	      *signo = SIGFPE;
	      *sigcode = FPE_FLTDIV_FAULT;
	    }
	  else if (subcode & FPS_OE)
	    {
	      *signo = SIGFPE;
	      *sigcode = FPE_FLTOVF_FAULT;
	    }
	  else if (subcode & FPS_UE)
	    {
	      *signo = SIGFPE;
	      *sigcode = FPE_FLTUND_FAULT;
	    }
	  else if (subcode & FPS_PE)
	    {
	      *signo = SIGFPE;
	      *sigcode = FPE_FLTINX_FAULT;
	    }
	  else
	    {
	      *signo = SIGFPE;
	      *sigcode = 0;
	    }
	  break;

	  /* These two can only be arithmetic exceptions if we 
	     are in V86 mode, which sounds like emulation to me.
	     (See Mach 3.0 i386/trap.c.)  */
	case EXC_I386_EMERR:
	  *signo = SIGFPE;
	  *sigcode = FPE_EMERR_FAULT;
	  break;
	case EXC_I386_BOUND:
	  *signo = SIGFPE;
	  *sigcode = FPE_EMBND_FAULT;
	  break;
	}
      break;

    case EXC_EMULATION:		
      /* 3.0 doesn't give this one, why, I don't know.  */
      *signo = SIGEMT;
      *sigcode = 0;
      break;

    case EXC_SOFTWARE:
      /* The only time we get this in Mach 3.0
	 is for an out of bounds trap.  */
      if (code == EXC_I386_BOUND)
	{
	  *signo = SIGFPE;
	  *sigcode = FPE_SUBRNG_FAULT;
	}
      else
	{
	  *signo = SIGEMT;
	  *sigcode = 0;
	}
      break;
      
    case EXC_BREAKPOINT:
      *signo = SIGTRAP;
      if (code == EXC_I386_SGL)
	*sigcode = DBG_SINGLE_TRAP;
      else if (code == EXC_I386_BPT)
	*sigcode = DBG_BRKPNT_FAULT;
      else
	*sigcode = 0;
      break;
    }
}
