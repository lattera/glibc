/* Dump registers.  PowerPC/Hurd version.
   Copyright (C) 1998, 2001 Free Software Foundation, Inc.
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

#include <sys/uio.h>
#include <stdio-common/_itoa.h>

/* This prints out the information in the following form: */
static const char dumpform[] = "\
Register dump:\n\
fp0-3:   0000030%0000031% 0000032%0000033% 0000034%0000035% 0000036%0000037%\n\
fp4-7:   0000038%0000039% 000003a%000003b% 000003c%000003d% 000003e%000003f%\n\
fp8-11:  0000040%0000041% 0000042%0000043% 0000044%0000045% 0000046%0000047%\n\
fp12-15: 0000048%0000049% 000004a%000004b% 000004c%000004d% 000004e%000004f%\n\
fp16-19: 0000050%0000051% 0000052%0000053% 0000054%0000055% 0000056%0000057%\n\
fp20-23: 0000058%0000059% 000005a%000005b% 000005c%000005d% 000005e%000005f%\n\
fp24-27: 0000060%0000061% 0000062%0000063% 0000064%0000065% 0000066%0000067%\n\
fp28-31: 0000068%0000069% 000006a%000006b% 000006c%000006d% 000006e%000006f%\n\
r0 =0000002% sp =0000003% r2 =0000004% r3 =0000005%\n\
r4 =0000006% r5 =0000007% r6 =0000008% r7 =0000009%   sr0=0000000% sr1=0000001%\n\
r8 =000000a% r9 =000000b% r10=000000c% r11=000000d%    cr=0000022% xer=0000023%\n\
r12=000000e% r13=000000f% r14=0000010% r15=0000011%    lr=0000024% ctr=0000025%\n\
r16=0000012% r17=0000013% r18=0000014% r19=0000015%    mq=0000026% fcr=0000071%\n\
r20=0000016% r21=0000017% r22=0000018% r23=0000019%   dar=0000028% dsi=0000029%\n\
r24=000001a% r25=000001b% r26=000001c% r27=000001d%   exc=000002a%\n\
r28=000001e% r29=000001f% r30=0000020% r31=0000021%\n\
";

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

   'dar' is the memory location, for traps 00300, 00400, 00600, 00A00.
   'dsisr' has the following bits under trap 00300:
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

#define xtoi(x) (x >= 'a' ? x + 10 - 'a' : x - '0')

static void
register_dump (int fd, struct sigcontext *ctx)
{
  char buffer[sizeof(dumpform)];
  char *bufferpos;
  unsigned regno;
  unsigned *regs = (unsigned *)(&ctx->sc_srr0);

  memcpy(buffer, dumpform, sizeof(dumpform));

  /* Generate the output.  */
  while ((bufferpos = memchr (buffer, '%', sizeof(dumpform))))
    {
      regno = xtoi (bufferpos[-1]) | xtoi (bufferpos[-2]) << 4;
      memset (bufferpos-2, '0', 3);
      _itoa_word (regs[regno], bufferpos+1, 16, 0);
    }

  /* Write the output.  */
  write (fd, buffer, sizeof(buffer));
}

#define REGISTER_DUMP \
  register_dump (fd, ctx)
