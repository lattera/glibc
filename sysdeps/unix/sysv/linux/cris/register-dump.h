/* Dump registers.
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

#include <stddef.h>
#include <sys/uio.h>
#include <stdio-common/_itoa.h>

/* We will print the register dump in this format:

  R0: XXXXXXXX   R1: XXXXXXXX   R2: XXXXXXXX   R3: XXXXXXXX
  R4: XXXXXXXX   R5: XXXXXXXX   R6: XXXXXXXX   R7: XXXXXXXX
  R8: XXXXXXXX   R9: XXXXXXXX  R10: XXXXXXXX  R11: XXXXXXXX
 R12: XXXXXXXX  R13: XXXXXXXX   SP: XXXXXXXX   PC: XXXXXXXX
DCCR: XXXXXXXX  SRP: XXXXXXXX */

static void
hexvalue (unsigned long int value, char *buf, size_t len)
{
  char *cp = _itoa_word (value, buf + len, 16, 0);
  while (cp > buf)
    *--cp = '0';
}

static void register_dump (int fd, struct sigcontext *ctx)
{
  char regs[18][8];
  struct iovec iov[36], *next_iov = iov;
  struct pt_regs *rx = &ctx->regs;

#define ADD_STRING(str) \
  next_iov->iov_base = (char *) (str); \
  next_iov->iov_len = strlen (str); \
  ++next_iov
#define ADD_MEM(str, len) \
  next_iov->iov_base = (str); \
  next_iov->iov_len = (len); \
  ++next_iov

  /* Generate strings of register contents.  */
  hexvalue (rx->r0, regs[0], 8);
  hexvalue (rx->r1, regs[1], 8);
  hexvalue (rx->r2, regs[2], 8);
  hexvalue (rx->r3, regs[3], 8);
  hexvalue (rx->r4, regs[4], 8);
  hexvalue (rx->r5, regs[5], 8);
  hexvalue (rx->r6, regs[6], 8);
  hexvalue (rx->r7, regs[7], 8);
  hexvalue (rx->r8, regs[8], 8);
  hexvalue (rx->r9, regs[9], 8);
  hexvalue (rx->r10, regs[10], 8);
  hexvalue (rx->r11, regs[11], 8);
  hexvalue (rx->r12, regs[12], 8);
  hexvalue (rx->r13, regs[13], 8);
  hexvalue (ctx->usp, regs[14], 8);
  hexvalue (rx->irp, regs[17], 8);
  hexvalue (rx->dccr, regs[15], 8);
  hexvalue (rx->srp, regs[16], 8);

  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\n  R0: ");
  ADD_MEM (regs[0], 8);
  ADD_STRING ("  R1: ");
  ADD_MEM (regs[1], 8);
  ADD_STRING ("  R2: ");
  ADD_MEM (regs[2], 8);
  ADD_STRING ("  R3: ");
  ADD_MEM (regs[3], 8);
  ADD_STRING ("\n  R4: ");
  ADD_MEM (regs[4], 8);
  ADD_STRING ("  R5: ");
  ADD_MEM (regs[5], 8);
  ADD_STRING ("  R6: ");
  ADD_MEM (regs[6], 8);
  ADD_STRING ("  R7: ");
  ADD_MEM (regs[7], 8);
  ADD_STRING ("\n  R8: ");
  ADD_MEM (regs[8], 8);
  ADD_STRING ("  R9: ");
  ADD_MEM (regs[9], 8);
  ADD_STRING (" R10: ");
  ADD_MEM (regs[10], 8);
  ADD_STRING (" R11: ");
  ADD_MEM (regs[11], 8);
  ADD_STRING ("\n R12: ");
  ADD_MEM (regs[12], 8);
  ADD_STRING (" R13: ");
  ADD_MEM (regs[13], 8);
  ADD_STRING ("  SP: ");
  ADD_MEM (regs[14], 8);
  ADD_STRING ("  PC: ");
  ADD_MEM (regs[17], 8);
  ADD_STRING ("\nDCCR: ");
  ADD_MEM (regs[15], 8);
  ADD_STRING (" SRP: ");
  ADD_MEM (regs[16], 4);

  /* Write the stuff out.  */
  writev (fd, iov, next_iov - iov);
}

#define REGISTER_DUMP register_dump (fd, ctx)
