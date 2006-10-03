/* Copyright (C) 1991, 1992, 1997 Free Software Foundation, Inc.
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

#include <signal.h>
#include <68881-sw.h>


/* The signal that is sent when a 68881 instruction
   is executed and there is no 68881.  */
#ifndef	TRAPSIG
#define	TRAPSIG	SIGILL
#endif

/* Zero if no 68881, one if we have a 68881, or -1 if we don't know yet.  */
static int have_fpu = -1;


/* Signal handler for the trap that happens if we don't have a 68881.  */
static void
trap (sig)
     int sig;
{
  have_fpu = 0;
}

/* This function is called by functions that want to switch.
   The calling function must be a `struct switch_caller' in data space.
   It determines whether a 68881 is present, and modifies its caller
   to be a static jump to either the 68881 version or the soft version.
   It then returns into the function it has chosen to do the work.  */
void
__68881_switch (dummy)
     int dummy;
{
  void **return_address_location = &((void **) &dummy)[-1];
  struct switch_caller *const caller
    = (struct switch_caller *) (((short int *) *return_address_location) - 1);

  if (have_fpu < 0)
    {
      /* Figure out whether or not we have a 68881.  */
      __sighandler_t handler = signal (TRAPSIG, trap);
      if (handler == SIG_ERR)
	/* We can't figure it out, so assume we don't have a 68881.
	   This assumption will never cause us any problems other than
	   lost performance, while the reverse assumption could cause
	   the program to crash.  */
	have_fpu = 0;
      else
	{
	  /* We set `have_fpu' to nonzero, and then execute a 68881
	     no-op instruction.  If we have a 68881, this will do nothing.
	     If we don't have one, this will trap and the signal handler
	     will clear `have_fpu'.  */
	  have_fpu = 1;
	  asm ("fnop");

	  /* Restore the old signal handler.  */
	  (void) signal (TRAPSIG, handler);
	}
    }

  /* Modify the caller to be a jump to the appropriate address.  */
  caller->insn = JMP;
  caller->target = have_fpu ? caller->fpu : caller->soft;

  /* Make the address we will return to be the target we have chosen.
     Our return will match the `jsr' done by the caller we have
     just modified, and it will be just as if that had instead
     been a `jmp' to the new target.  */
  *return_address_location = caller->target;
}
