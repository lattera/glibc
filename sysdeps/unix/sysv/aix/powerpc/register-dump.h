/* Dump registers.
   Copyright (C) 1998 Free Software Foundation, Inc.
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
#include <sys/ucontext.h>

/* We will print the register dump in this format:

Register dump:
fp0-3:   0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp4-7:   0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp8-11:  0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp12-15: 0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp16-19: 0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp20-23: 0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp24-27: 0000000000000000 0000000000000000 0000000000000000 0000000000000000
fp28-31: 0000000000000000 0000000000000000 0000000000000000 0000000000000000

r00-07 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
r08-15 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
r16-23 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 
r24-31 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 

  trap 00000000   iar 00000000 msr 00000000 cr 00000000
    lr 00000000   ctr 00000000 xer 00000000 mq 00000000
   tid 00000000 fpscr 00000000

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
  char regs[108][8];
  struct iovec iov[38];
  struct __mstsafe  *reg_state;
  int i;
  int ii;
  size_t nr = 0;

#define ADD_STRING(str) \
  iov[nr].iov_base = (char *) str;                                            \
  iov[nr].iov_len = strlen (str);                                             \
  ++nr
#define ADD_MEM(str, len) \
  iov[nr].iov_base = str;                                                     \
  iov[nr].iov_len = len;                                                      \
  ++nr

  reg_state = (struct __mstsafe  *)&ctx->sc_jmpbuf.__jmp_context;

  hexvalue (reg_state->__excp_type, regs[0], 8);
  hexvalue (reg_state->__iar, regs[1], 8);
  hexvalue (reg_state->__msr, regs[2], 8);
  hexvalue (reg_state->__cr, regs[3], 8);
  hexvalue (reg_state->__lr, regs[4], 8);
  hexvalue (reg_state->__ctr, regs[5], 8);
  hexvalue (reg_state->__xer, regs[6], 8);
  hexvalue (reg_state->__mq, regs[7], 8);
  hexvalue (reg_state->__tid, regs[8], 8);
  hexvalue (reg_state->__fpscr, regs[9], 8);

  ii=10;
  for (i = 0; i <= 96; i++,ii++)
    hexvalue (reg_state->__gpr[i], regs[ii], 8);

  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\nfp0-3:   ");
  ADD_MEM (regs[42], 8);
  ADD_MEM (regs[43], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[44], 8);
  ADD_MEM (regs[45], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[46], 8);
  ADD_MEM (regs[47], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[48], 8);
  ADD_MEM (regs[49], 8);
  ADD_STRING ("\nfp4-7:   ");
  ADD_MEM (regs[50], 8);
  ADD_MEM (regs[51], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[52], 8);
  ADD_MEM (regs[53], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[54], 8);
  ADD_MEM (regs[55], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[56], 8);
  ADD_MEM (regs[57], 8);
  ADD_STRING ("\nfp8-11:  ");
  ADD_MEM (regs[58], 8);
  ADD_MEM (regs[59], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[60], 8);
  ADD_MEM (regs[61], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[62], 8);
  ADD_MEM (regs[63], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[64], 8);
  ADD_MEM (regs[65], 8);
  ADD_STRING ("\nfp12-15: ");
  ADD_MEM (regs[66], 8);
  ADD_MEM (regs[67], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[68], 8);
  ADD_MEM (regs[69], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[70], 8);
  ADD_MEM (regs[71], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[72], 8);
  ADD_MEM (regs[73], 8);
  ADD_STRING ("\nfp16-19: ");
  ADD_MEM (regs[74], 8);
  ADD_MEM (regs[75], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[76], 8);
  ADD_MEM (regs[78], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[79], 8);
  ADD_MEM (regs[80], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[81], 8);
  ADD_MEM (regs[82], 8);
  ADD_STRING ("\nfp20-23: ");
  ADD_MEM (regs[83], 8);
  ADD_MEM (regs[84], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[85], 8);
  ADD_MEM (regs[86], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[87], 8);
  ADD_MEM (regs[88], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[89], 8);
  ADD_MEM (regs[90], 8);
  ADD_STRING ("\nfp24-27: ");
  ADD_MEM (regs[91], 8);
  ADD_MEM (regs[92], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[93], 8);
  ADD_MEM (regs[94], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[95], 8);
  ADD_MEM (regs[96], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[97], 8);
  ADD_MEM (regs[98], 8);
  ADD_STRING ("\nfp28-31: ");
  ADD_MEM (regs[99], 8);
  ADD_MEM (regs[100], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[101], 8);
  ADD_MEM (regs[102], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[103], 8);
  ADD_MEM (regs[104], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[105], 8);
  ADD_MEM (regs[106], 8);
  ADD_STRING ("\n\nr00-07 ");
  ADD_MEM (regs[10], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[11], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[12], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[13], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[14], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[15], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[16], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[17], 8);
  ADD_STRING ("\nr08-15 ");
  ADD_MEM (regs[18], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[19], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[20], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[21], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[22], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[23], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[24], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[25], 8);
  ADD_STRING ("\nr16-23 ");
  ADD_MEM (regs[26], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[27], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[28], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[29], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[30], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[31], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[32], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[33], 8);
  ADD_STRING ("\nr24-31 ");
  ADD_MEM (regs[34], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[35], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[36], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[37], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[38], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[39], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[40], 8);
  ADD_STRING (" ");
  ADD_MEM (regs[41], 8);
  ADD_STRING ("\n\n  trap ");
  ADD_MEM (regs[0], 8);
  ADD_STRING ("   iar ");
  ADD_MEM (regs[1], 8);
  ADD_STRING (" msr ");
  ADD_MEM (regs[2], 8);
  ADD_STRING (" cr ");
  ADD_MEM (regs[3], 8);
  ADD_STRING ("\n    lr ");
  ADD_MEM (regs[4], 8);
  ADD_STRING ("   ctr ");
  ADD_MEM (regs[5], 8);
  ADD_STRING (" xer ");
  ADD_MEM (regs[6], 8);
  ADD_STRING (" mq ");
  ADD_MEM (regs[7], 8);
  ADD_STRING ("\n   tid ");
  ADD_MEM (regs[8], 8);
  ADD_STRING (" fpscr ");
  ADD_MEM (regs[9], 8);
  ADD_STRING ("\n");

  /* Write the stuff out.  */
  writev (fd, iov, nr);
}

#define REGISTER_DUMP register_dump (fd, ctx)

