/* Dump registers.
   Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <sys/uio.h>
#include <stdio-common/_itoa.h>

/* This prints out the information in the following form: */
static const char dumpform[] =
"Register dump:\n\
r0 =0000000% sp =0000000% r2 =0000000% r3 =0000000% r4 =0000000% r5 =0000000%
r6 =0000000% r7 =0000000% r8 =0000000% r9 =0000000% r10=0000000% r11=0000000%
r12=0000000% r13=0000000% r14=0000000% r15=0000000% r16=0000000% r17=0000000%
r18=0000000% r19=0000000% r20=0000000% r21=0000000% r22=0000000% r23=0000000%
r24=0000000% r25=0000000% r26=0000000% r27=0000000% r28=0000000% r29=0000000%
r30=0000000% r31=0000000% sr0=0000000% msr=0000000% r3*=0000000% ctr=0000000%
lr =0000000% xer=0000000% ccr=0000000% sr1=0000000% trap=0000000%
address of fault=0000000% dsisr=0000000%\n";
/* Most of the fields are self-explanatory.  'sr0' is the next
   instruction to execute, from SRR0, which may have some relationship
   with the instruction that caused the exception.  'r3*' is the value
   that will be returned in register 3 when the current system call
   returns.  'sr1' is SRR1, bits 16-31 of which are copied from the MSR:

   16 - External interrupt enable
   17 - Privilege level (1=user, 0=supervisor)
   18 - FP available
   19 - Machine check enable (if clear, processor locks up on machine check)
   20 - FP exception mode bit 0 (FP exceptions recoverable)
   21 - Single-step trace enable
   22 - Branch trace enable
   23 - FP exception mode bit 1
   25 - exception prefix (if set, exceptions are taken from 0xFFFnnnnn,
        otherwise from 0x000nnnnn).
   26 - Instruction address translation enabled.
   27 - Data address translation enabled.
   30 - Exception is recoverable (otherwise, don't try to return).
   31 - Little-endian mode enable.

   'Trap' is the address of the exception:

   00200 - Machine check exception (memory parity error, for instance)
   00300 - Data access exception (memory not mapped, see dsisr for why)
   00400 - Instruction access exception (memory not mapped)
   00500 - External interrupt
   00600 - Alignment exception (see dsisr for more information)
   00700 - Program exception (illegal/trap instruction, FP exception)
   00800 - FP unavailable (should not be seen by user code)
   00900 - Decrementer exception (for instance, SIGALRM)
   00A00 - I/O controller interface exception
   00C00 - System call exception (for instance, kill(3)).
   00E00 - FP assist exception (optional FP instructions, etc.)

   'address of fault' is the memory location that wasn't mapped
   (from the DAR). 'dsisr' has the following bits under trap 00300:
   0 - direct-store error exception
   1 - no page table entry for page
   4 - memory access not permitted
   5 - trying to access I/O controller space or using lwarx/stwcx on
       non-write-cached memory
   6 - access was store
   9 - data access breakpoint hit
   10 - segment table search failed to find translation (64-bit ppcs only)
   11 - I/O controller instruction not permitted
   For trap 00400, the same bits are set in SRR1 instead.
   For trap 00600, bits 12-31 of the DSISR set to allow emulation of
   the instruction without actually having to read it from memory.
*/

static void
register_dump (int fd, void **ctx)
{
  char buffer[sizeof(dumpform)];
  char *bufferpos = buffer;
  int i = 0;

  memcpy(buffer, dumpform, sizeof(dumpform));

  ctx += 8;	/* FIXME!!!!  Why is this necessary?  Is it necessary?  */

  /* Generate the output.  */
  while ((bufferpos = memchr (bufferpos, '%', sizeof(dumpform))))
    {
      *bufferpos++ = '0';
      _itoa_word ((unsigned long int)(ctx[i]), bufferpos, 16, 0);
      i++;
    }

  /* Write the output.  */
  write (fd, buffer, sizeof(buffer));
}


#define REGISTER_DUMP \
  register_dump (fd, ctx)
