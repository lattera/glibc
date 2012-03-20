/* Dump registers.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <sys/uio.h>
#include <_itoa.h>

/* We will print the register dump in this format:

  R0: XXXXXXXX   R1: XXXXXXXX   R2: XXXXXXXX   R3: XXXXXXXX
  R4: XXXXXXXX   R5: XXXXXXXX   R6: XXXXXXXX   R7: XXXXXXXX
  R8: XXXXXXXX   R9: XXXXXXXX  R10: XXXXXXXX  R11: XXXXXXXX
 R12: XXXXXXXX  R13: XXXXXXXX  R14: XXXXXXXX  R15: XXXXXXXX

MACL: XXXXXXXX MACH: XXXXXXXX

  PC: XXXXXXXX   PR: XXXXXXXX  GBR: XXXXXXXX   SR: XXXXXXXX

 FR0: XXXXXXXX  FR1: XXXXXXXX  FR2: XXXXXXXX  FR3: XXXXXXXX 
 FR4: XXXXXXXX  FR5: XXXXXXXX  FR6: XXXXXXXX  FR7: XXXXXXXX 
 FR8: XXXXXXXX  FR9: XXXXXXXX FR10: XXXXXXXX FR11: XXXXXXXX 
FR12: XXXXXXXX FR13: XXXXXXXX FR14: XXXXXXXX FR15: XXXXXXXX 

 XR0: XXXXXXXX  XR1: XXXXXXXX  XR2: XXXXXXXX  XR3: XXXXXXXX 
 XR4: XXXXXXXX  XR5: XXXXXXXX  XR6: XXXXXXXX  XR7: XXXXXXXX 
 XR8: XXXXXXXX  XR9: XXXXXXXX XR10: XXXXXXXX XR11: XXXXXXXX 
XR12: XXXXXXXX XR13: XXXXXXXX XR14: XXXXXXXX XR15: XXXXXXXX 

FPSCR: XXXXXXXX FPUL: XXXXXXXX

 */

static void
hexvalue (unsigned long int value, char *buf, size_t len)
{
  char *cp = _itoa_word (value, buf + len, 16, 0);
  while (cp > buf)
    *--cp = '0';
}

static void
register_dump (int fd, struct sigcontext *ctx)
{
  char regs[22][8];
  struct iovec iov[112];
  size_t nr = 0;

#define ADD_STRING(str) \
  iov[nr].iov_base = (char *) str;					      \
  iov[nr].iov_len = strlen (str);					      \
  ++nr
#define ADD_MEM(str, len) \
  iov[nr].iov_base = str;						      \
  iov[nr].iov_len = len;						      \
  ++nr

  /* Generate strings of register contents.  */
  hexvalue (ctx->sc_regs[0], regs[0], 8);
  hexvalue (ctx->sc_regs[1], regs[1], 8);
  hexvalue (ctx->sc_regs[2], regs[2], 8);
  hexvalue (ctx->sc_regs[3], regs[3], 8);
  hexvalue (ctx->sc_regs[4], regs[4], 8);
  hexvalue (ctx->sc_regs[5], regs[5], 8);
  hexvalue (ctx->sc_regs[6], regs[6], 8);
  hexvalue (ctx->sc_regs[7], regs[7], 8);
  hexvalue (ctx->sc_regs[8], regs[8], 8);
  hexvalue (ctx->sc_regs[9], regs[9], 8);
  hexvalue (ctx->sc_regs[10], regs[10], 8);
  hexvalue (ctx->sc_regs[11], regs[11], 8);
  hexvalue (ctx->sc_regs[12], regs[12], 8);
  hexvalue (ctx->sc_regs[13], regs[13], 8);
  hexvalue (ctx->sc_regs[14], regs[14], 8);
  hexvalue (ctx->sc_regs[15], regs[15], 8);
  hexvalue (ctx->sc_macl, regs[16], 8);
  hexvalue (ctx->sc_mach, regs[17], 8);
  hexvalue (ctx->sc_pc, regs[18], 8);
  hexvalue (ctx->sc_pr, regs[19], 8);
  hexvalue (ctx->sc_gbr, regs[20], 8);
  hexvalue (ctx->sc_sr, regs[21], 8);

  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\n  R0: ");
  ADD_MEM (regs[0], 8);
  ADD_STRING ("   R1: ");
  ADD_MEM (regs[1], 8);
  ADD_STRING ("   R2: ");
  ADD_MEM (regs[2], 8);
  ADD_STRING ("   R3: ");
  ADD_MEM (regs[3], 8);
  ADD_STRING ("\n  R4: ");
  ADD_MEM (regs[4], 8);
  ADD_STRING ("   R5: ");
  ADD_MEM (regs[5], 8);
  ADD_STRING ("   R6: ");
  ADD_MEM (regs[6], 8);
  ADD_STRING ("   R7: ");
  ADD_MEM (regs[7], 8);
  ADD_STRING ("\n  R8: ");
  ADD_MEM (regs[8], 8);
  ADD_STRING ("   R9: ");
  ADD_MEM (regs[9], 8);
  ADD_STRING ("  R10: ");
  ADD_MEM (regs[10], 8);
  ADD_STRING ("  R11: ");
  ADD_MEM (regs[11], 8);
  ADD_STRING ("\n R12: ");
  ADD_MEM (regs[12], 8);
  ADD_STRING ("  R13: ");
  ADD_MEM (regs[13], 8);
  ADD_STRING ("  R14: ");
  ADD_MEM (regs[14], 8);
  ADD_STRING ("  R15: ");
  ADD_MEM (regs[15], 8);

  ADD_STRING ("\n\nMACL: ");
  ADD_MEM (regs[16], 8);
  ADD_STRING (" MACH: ");
  ADD_MEM (regs[17], 8);

  ADD_STRING ("\n\n  PC: ");
  ADD_MEM (regs[18], 8);
  ADD_STRING ("   PR: ");
  ADD_MEM (regs[19], 8);
  ADD_STRING ("  GBR: ");
  ADD_MEM (regs[20], 8);
  ADD_STRING ("   SR: ");
  ADD_MEM (regs[21], 8);

  ADD_STRING ("\n");

  /* Write the stuff out.  */
  writev (fd, iov, nr);
}


#define REGISTER_DUMP register_dump (fd, &ctx)
