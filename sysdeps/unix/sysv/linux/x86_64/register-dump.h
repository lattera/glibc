/* Dump registers.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

/* We will print the register dump in this format:

 RAX: XXXXXXXXXXXXXXXX   RBX: XXXXXXXXXXXXXXXX  RCX: XXXXXXXXXXXXXXXX
 RDX: XXXXXXXXXXXXXXXX   RSI: XXXXXXXXXXXXXXXX  RDI: XXXXXXXXXXXXXXXX
 RBP: XXXXXXXXXXXXXXXX   R8 : XXXXXXXXXXXXXXXX  R9 : XXXXXXXXXXXXXXXX
 R10: XXXXXXXXXXXXXXXX   R11: XXXXXXXXXXXXXXXX  R12: XXXXXXXXXXXXXXXX
 R13: XXXXXXXXXXXXXXXX   R14: XXXXXXXXXXXXXXXX  R15: XXXXXXXXXXXXXXXX
 RSP: XXXXXXXXXXXXXXXX

 RIP: XXXXXXXXXXXXXXXX   EFLAGS: XXXXXXXX

 CS:  XXXX   DS: XXXX   ES: XXXX   FS: XXXX   GS: XXXX

 Trap:  XXXXXXXX   Error: XXXXXXXX   OldMask: XXXXXXXX
 RSP/SIGNAL: XXXXXXXXXXXXXXXX  CR2: XXXXXXXX

 FPUCW: XXXXXXXX   FPUSW: XXXXXXXX   TAG: XXXXXXXX
 IPOFF: XXXXXXXX   CSSEL: XXXX   DATAOFF: XXXXXXXX   DATASEL: XXXX

 ST(0) XXXX XXXXXXXXXXXXXXXX   ST(1) XXXX XXXXXXXXXXXXXXXX
 ST(2) XXXX XXXXXXXXXXXXXXXX   ST(3) XXXX XXXXXXXXXXXXXXXX
 ST(4) XXXX XXXXXXXXXXXXXXXX   ST(5) XXXX XXXXXXXXXXXXXXXX
 ST(6) XXXX XXXXXXXXXXXXXXXX   ST(7) XXXX XXXXXXXXXXXXXXXX

 mxcsr: XXXX
 XMM0 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM1 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM2 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM3 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM4 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM5 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM6 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM7 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM8 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM9 : XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM10: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM11: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM12: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM13: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 XMM14: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XMM15: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

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
  char regs[29][16];
  char fpregs[32][8];
  char xmmregs[16][32];
  struct iovec iov[147];
  size_t nr = 0;
  int i;

#define ADD_STRING(str) \
  iov[nr].iov_base = (char *) str;					      \
  iov[nr].iov_len = strlen (str);					      \
  ++nr
#define ADD_MEM(str, len) \
  iov[nr].iov_base = str;						      \
  iov[nr].iov_len = len;						      \
  ++nr

  /* Generate strings of register contents.  */
  hexvalue (ctx->rax, regs[0], 16);
  hexvalue (ctx->rbx, regs[1], 16);
  hexvalue (ctx->rcx, regs[2], 16);
  hexvalue (ctx->rdx, regs[3], 16);
  hexvalue (ctx->rsi, regs[4], 16);
  hexvalue (ctx->rdi, regs[5], 16);
  hexvalue (ctx->rbp, regs[6], 16);
  hexvalue (ctx->r8, regs[7], 16);
  hexvalue (ctx->r9, regs[8], 16);
  hexvalue (ctx->r10, regs[9], 16);
  hexvalue (ctx->r11, regs[10], 16);
  hexvalue (ctx->r12, regs[11], 16);
  hexvalue (ctx->r13, regs[12], 16);
  hexvalue (ctx->r14, regs[13], 16);
  hexvalue (ctx->r15, regs[14], 16);
  hexvalue (ctx->rsp, regs[15], 16);
  hexvalue (ctx->rip, regs[16], 16);

  hexvalue (ctx->eflags, regs[17], 8);
  hexvalue (ctx->cs, regs[18], 4);
  hexvalue (ctx->ds, regs[19], 4);
  hexvalue (ctx->es, regs[20], 4);
  hexvalue (ctx->fs, regs[21], 4);
  hexvalue (ctx->gs, regs[22], 4);
  /* hexvalue (ctx->ss, regs[23], 4); */
  hexvalue (ctx->trapno, regs[24], 8);
  hexvalue (ctx->err, regs[25], 8);
  hexvalue (ctx->oldmask, regs[26], 8);
  hexvalue (ctx->rsp_at_signal, regs[27], 16);
  hexvalue (ctx->cr2, regs[28], 8);

  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\n RAX: ");
  ADD_MEM (regs[0], 16);
  ADD_STRING ("   RBX: ");
  ADD_MEM (regs[1], 16);
  ADD_STRING ("   RCX: ");
  ADD_MEM (regs[2], 16);
  ADD_STRING ("\n RDX: ");
  ADD_MEM (regs[3], 16);
  ADD_STRING ("   RSI: ");
  ADD_MEM (regs[4], 16);
  ADD_STRING ("   RDI: ");
  ADD_MEM (regs[5], 16);
  ADD_STRING ("\n RBP: ");
  ADD_MEM (regs[6], 16);
  ADD_STRING ("   R8 : ");
  ADD_MEM (regs[7], 16);
  ADD_STRING ("   R9 : ");
  ADD_MEM (regs[8], 16);
  ADD_STRING ("\n R10: ");
  ADD_MEM (regs[9], 16);
  ADD_STRING ("   R11: ");
  ADD_MEM (regs[10], 16);
  ADD_STRING ("   R12: ");
  ADD_MEM (regs[11], 16);
  ADD_STRING ("\n R13: ");
  ADD_MEM (regs[12], 16);
  ADD_STRING ("   R14: ");
  ADD_MEM (regs[13], 16);
  ADD_STRING ("   R15: ");
  ADD_MEM (regs[14], 16);
  ADD_STRING ("\n RSP: ");
  ADD_MEM (regs[15], 16);
  ADD_STRING ("\n\n RIP: ");
  ADD_MEM (regs[16], 16);
  ADD_STRING ("   EFLAGS: ");
  ADD_MEM (regs[17], 8);
  ADD_STRING ("\n\n CS: ");
  ADD_MEM (regs[18], 4);
  ADD_STRING ("   DS: ");
  ADD_MEM (regs[19], 4);
  ADD_STRING ("   ES: ");
  ADD_MEM (regs[20], 4);
  ADD_STRING ("   FS: ");
  ADD_MEM (regs[21], 4);
  ADD_STRING ("   GS: ");
  ADD_MEM (regs[22], 4);
  /*
  ADD_STRING ("   SS: ");
  ADD_MEM (regs[23], 4);
  */
  ADD_STRING ("\n\n Trap: ");
  ADD_MEM (regs[24], 8);
  ADD_STRING ("   Error: ");
  ADD_MEM (regs[25], 8);
  ADD_STRING ("   OldMask: ");
  ADD_MEM (regs[26], 8);
  ADD_STRING ("\n RSP/signal: ");
  ADD_MEM (regs[27], 8);
  ADD_STRING ("   CR2: ");
  ADD_MEM (regs[28], 8);

  if (ctx->fpstate != NULL)
    {

      /* Generate output for the FPU control/status registers.  */
      hexvalue (ctx->fpstate->cw, fpregs[0], 8);
      hexvalue (ctx->fpstate->sw, fpregs[1], 8);
      hexvalue (ctx->fpstate->tag, fpregs[2], 8);
      hexvalue (ctx->fpstate->ipoff, fpregs[3], 8);
      hexvalue (ctx->fpstate->cssel, fpregs[4], 4);
      hexvalue (ctx->fpstate->dataoff, fpregs[5], 8);
      hexvalue (ctx->fpstate->datasel, fpregs[6], 4);

      ADD_STRING ("\n\n FPUCW: ");
      ADD_MEM (fpregs[0], 8);
      ADD_STRING ("   FPUSW: ");
      ADD_MEM (fpregs[1], 8);
      ADD_STRING ("   TAG: ");
      ADD_MEM (fpregs[2], 8);
      ADD_STRING ("\n IPOFF: ");
      ADD_MEM (fpregs[3], 8);
      ADD_STRING ("   CSSEL: ");
      ADD_MEM (fpregs[4], 4);
      ADD_STRING ("   DATAOFF: ");
      ADD_MEM (fpregs[5], 8);
      ADD_STRING ("   DATASEL: ");
      ADD_MEM (fpregs[6], 4);

      /* Now the real FPU registers.  */
      hexvalue (ctx->fpstate->_st[0].exponent, fpregs[7], 8);
      hexvalue (ctx->fpstate->_st[0].significand[3] << 16
		| ctx->fpstate->_st[0].significand[2], fpregs[8], 8);
      hexvalue (ctx->fpstate->_st[0].significand[1] << 16
		| ctx->fpstate->_st[0].significand[0], fpregs[9], 8);
      hexvalue (ctx->fpstate->_st[1].exponent, fpregs[10], 8);
      hexvalue (ctx->fpstate->_st[1].significand[3] << 16
		| ctx->fpstate->_st[1].significand[2], fpregs[11], 8);
      hexvalue (ctx->fpstate->_st[1].significand[1] << 16
		| ctx->fpstate->_st[1].significand[0], fpregs[12], 8);
      hexvalue (ctx->fpstate->_st[2].exponent, fpregs[13], 8);
      hexvalue (ctx->fpstate->_st[2].significand[3] << 16
		| ctx->fpstate->_st[2].significand[2], fpregs[14], 8);
      hexvalue (ctx->fpstate->_st[2].significand[1] << 16
		| ctx->fpstate->_st[2].significand[0], fpregs[15], 8);
      hexvalue (ctx->fpstate->_st[3].exponent, fpregs[16], 8);
      hexvalue (ctx->fpstate->_st[3].significand[3] << 16
		| ctx->fpstate->_st[3].significand[2], fpregs[17], 8);
      hexvalue (ctx->fpstate->_st[3].significand[1] << 16
		| ctx->fpstate->_st[3].significand[0], fpregs[18], 8);
      hexvalue (ctx->fpstate->_st[4].exponent, fpregs[19], 8);
      hexvalue (ctx->fpstate->_st[4].significand[3] << 16
		| ctx->fpstate->_st[4].significand[2], fpregs[20], 8);
      hexvalue (ctx->fpstate->_st[4].significand[1] << 16
		| ctx->fpstate->_st[4].significand[0], fpregs[21], 8);
      hexvalue (ctx->fpstate->_st[5].exponent, fpregs[22], 8);
      hexvalue (ctx->fpstate->_st[5].significand[3] << 16
		| ctx->fpstate->_st[5].significand[2], fpregs[23], 8);
      hexvalue (ctx->fpstate->_st[5].significand[1] << 16
		| ctx->fpstate->_st[5].significand[0], fpregs[24], 8);
      hexvalue (ctx->fpstate->_st[6].exponent, fpregs[25], 8);
      hexvalue (ctx->fpstate->_st[6].significand[3] << 16
		| ctx->fpstate->_st[6].significand[2], fpregs[26], 8);
      hexvalue (ctx->fpstate->_st[6].significand[1] << 16
		| ctx->fpstate->_st[6].significand[0], fpregs[27], 8);
      hexvalue (ctx->fpstate->_st[7].exponent, fpregs[28], 8);
      hexvalue (ctx->fpstate->_st[7].significand[3] << 16
		| ctx->fpstate->_st[7].significand[2], fpregs[29], 8);
      hexvalue (ctx->fpstate->_st[7].significand[1] << 16
		| ctx->fpstate->_st[7].significand[0], fpregs[30], 8);

      hexvalue (ctx->fpstate->mxcsr, fpregs[31], 4);

      for (i = 0; i < 16; i++)
	hexvalue (ctx->fpstate->_xmm[i].element[3] << 24
		  | ctx->fpstate->_xmm[i].element[2] << 16
		  | ctx->fpstate->_xmm[i].element[1] << 8
		  | ctx->fpstate->_xmm[i].element[0], xmmregs[i], 32);


      ADD_STRING ("\n\n ST(0) ");
      ADD_MEM (fpregs[7], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[8], 8);
      ADD_MEM (fpregs[9], 8);
      ADD_STRING ("   ST(1) ");
      ADD_MEM (fpregs[10], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[11], 8);
      ADD_MEM (fpregs[12], 8);
      ADD_STRING ("\n ST(2) ");
      ADD_MEM (fpregs[13], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[14], 8);
      ADD_MEM (fpregs[15], 8);
      ADD_STRING ("   ST(3) ");
      ADD_MEM (fpregs[16], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[17], 8);
      ADD_MEM (fpregs[18], 8);
      ADD_STRING ("\n ST(4) ");
      ADD_MEM (fpregs[19], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[20], 8);
      ADD_MEM (fpregs[21], 8);
      ADD_STRING ("   ST(5) ");
      ADD_MEM (fpregs[22], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[23], 8);
      ADD_MEM (fpregs[24], 8);
      ADD_STRING ("\n ST(6) ");
      ADD_MEM (fpregs[25], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[26], 8);
      ADD_MEM (fpregs[27], 8);
      ADD_STRING ("   ST(7) ");
      ADD_MEM (fpregs[28], 4);
      ADD_STRING (" ");
      ADD_MEM (fpregs[29], 8);
      ADD_MEM (fpregs[30], 8);

      ADD_STRING ("\n mxcsr: ");
      ADD_MEM (fpregs[31], 4);

      ADD_STRING ("\n XMM0: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM1: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM2: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM3: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM4: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM5: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM6: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM7: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM8: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM9: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM10: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM11: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM12: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM13: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING ("\n XMM14: ");
      ADD_MEM (xmmregs[0], 32);
      ADD_STRING (" XMM15: ");
      ADD_MEM (xmmregs[0], 32);

    }

  ADD_STRING ("\n");

  /* Write the stuff out.  */
  writev (fd, iov, nr);
}


#define REGISTER_DUMP register_dump (fd, &ctx)
