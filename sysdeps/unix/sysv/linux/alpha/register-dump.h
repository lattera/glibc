/* Dump registers.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <stddef.h>
#include <string.h>

/* We will print the register dump in this format:

    V0: XXXXXXXXXXXXXXXX    T0: XXXXXXXXXXXXXXXX    T1: XXXXXXXXXXXXXXXX
    T2: XXXXXXXXXXXXXXXX    T3: XXXXXXXXXXXXXXXX    T4: XXXXXXXXXXXXXXXX
    T5: XXXXXXXXXXXXXXXX    T6: XXXXXXXXXXXXXXXX    T7: XXXXXXXXXXXXXXXX
    S0: XXXXXXXXXXXXXXXX    S1: XXXXXXXXXXXXXXXX    S2: XXXXXXXXXXXXXXXX
    S3: XXXXXXXXXXXXXXXX    S4: XXXXXXXXXXXXXXXX    S5: XXXXXXXXXXXXXXXX
    S6: XXXXXXXXXXXXXXXX    A0: XXXXXXXXXXXXXXXX    A1: XXXXXXXXXXXXXXXX
    A2: XXXXXXXXXXXXXXXX    A3: XXXXXXXXXXXXXXXX    A4: XXXXXXXXXXXXXXXX
    A5: XXXXXXXXXXXXXXXX    T8: XXXXXXXXXXXXXXXX    T9: XXXXXXXXXXXXXXXX
   T10: XXXXXXXXXXXXXXXX   T11: XXXXXXXXXXXXXXXX    RA: XXXXXXXXXXXXXXXX
   T12: XXXXXXXXXXXXXXXX    AT: XXXXXXXXXXXXXXXX    GP: XXXXXXXXXXXXXXXX
    SP: XXXXXXXXXXXXXXXX    PC: XXXXXXXXXXXXXXXX

   FP0: XXXXXXXXXXXXXXXX   FP1: XXXXXXXXXXXXXXXX   FP2: XXXXXXXXXXXXXXXX
   FP3: XXXXXXXXXXXXXXXX   FP4: XXXXXXXXXXXXXXXX   FP5: XXXXXXXXXXXXXXXX
   FP6: XXXXXXXXXXXXXXXX   FP7: XXXXXXXXXXXXXXXX   FP8: XXXXXXXXXXXXXXXX
   FP9: XXXXXXXXXXXXXXXX  FP10: XXXXXXXXXXXXXXXX  FP11: XXXXXXXXXXXXXXXX
  FP12: XXXXXXXXXXXXXXXX  FP13: XXXXXXXXXXXXXXXX  FP14: XXXXXXXXXXXXXXXX
  FP15: XXXXXXXXXXXXXXXX  FP16: XXXXXXXXXXXXXXXX  FP17: XXXXXXXXXXXXXXXX
  FP18: XXXXXXXXXXXXXXXX  FP19: XXXXXXXXXXXXXXXX  FP20: XXXXXXXXXXXXXXXX
  FP21: XXXXXXXXXXXXXXXX  FP22: XXXXXXXXXXXXXXXX  FP23: XXXXXXXXXXXXXXXX
  FP24: XXXXXXXXXXXXXXXX  FP25: XXXXXXXXXXXXXXXX  FP26: XXXXXXXXXXXXXXXX
  FP27: XXXXXXXXXXXXXXXX  FP28: XXXXXXXXXXXXXXXX  FP29: XXXXXXXXXXXXXXXX
  FP30: XXXXXXXXXXXXXXXX  FPCR: XXXXXXXXXXXXXXXX

   TA0: XXXXXXXXXXXXXXXX   TA1: XXXXXXXXXXXXXXXX   TA2: XXXXXXXXXXXXXXXX
*/

#define NREGS (32+32+3)

static const char __attribute__((aligned(8))) regnames[NREGS][8] =
{
  "    V0: ", "    T0: ", "    T1: ",
  "    T2: ", "    T3: ", "    T4: ",
  "    T5: ", "    T6: ", "    T7: ",
  "    S0: ", "    S1: ", "    S2: ",
  "    S3: ", "    S4: ", "    S5: ",
  "    S6: ", "    A0: ", "    A1: ",
  "    A2: ", "    A3: ", "    A4: ",
  "    A5: ", "    T8: ", "    T9: ",
  "   T10: ", "   T11: ", "    RA: ",
  "   T12: ", "    AT: ", "    GP: ",
  "    SP: ", "    PC: ",

  "   FP0: ", "   FP1: ", "   FP2: ",
  "   FP3: ", "   FP4: ", "   FP5: ",
  "   FP6: ", "   FP7: ", "   FP8: ",
  "   FP9: ", "  FP10: ", "  FP11: ",
  "  FP12: ", "  FP13: ", "  FP14: ",
  "  FP15: ", "  FP16: ", "  FP17: ",
  "  FP18: ", "  FP19: ", "  FP20: ",
  "  FP21: ", "  FP22: ", "  FP23: ",
  "  FP24: ", "  FP25: ", "  FP26: ",
  "  FP27: ", "  FP28: ", "  FP29: ",
  "  FP30: ", "  FPCR: ",

  "   TA0: ", "   TA1: ", "   TA2: "
};

#define O(FIELD, LF)  offsetof(struct sigcontext, FIELD) + LF

static const int offsets[NREGS] =
{
  O(sc_regs[0], 0),  O(sc_regs[1], 0),  O(sc_regs[2], 1),
  O(sc_regs[3], 0),  O(sc_regs[4], 0),  O(sc_regs[5], 1),
  O(sc_regs[6], 0),  O(sc_regs[7], 0),  O(sc_regs[8], 1),
  O(sc_regs[9], 0),  O(sc_regs[10], 0), O(sc_regs[11], 1),
  O(sc_regs[12], 0), O(sc_regs[13], 0), O(sc_regs[14], 1),
  O(sc_regs[15], 0), O(sc_regs[16], 0), O(sc_regs[17], 1),
  O(sc_regs[18], 0), O(sc_regs[19], 0), O(sc_regs[20], 1),
  O(sc_regs[21], 0), O(sc_regs[22], 0), O(sc_regs[23], 1),
  O(sc_regs[24], 0), O(sc_regs[25], 0), O(sc_regs[26], 1),
  O(sc_regs[27], 0), O(sc_regs[28], 0), O(sc_regs[29], 1),
  O(sc_regs[30], 0), O(sc_pc, 2),

  O(sc_fpregs[0], 0),  O(sc_fpregs[1], 0),  O(sc_fpregs[2], 1),
  O(sc_fpregs[3], 0),  O(sc_fpregs[4], 0),  O(sc_fpregs[5], 1),
  O(sc_fpregs[6], 0),  O(sc_fpregs[7], 0),  O(sc_fpregs[8], 1),
  O(sc_fpregs[9], 0),  O(sc_fpregs[10], 0), O(sc_fpregs[11], 1),
  O(sc_fpregs[12], 0), O(sc_fpregs[13], 0), O(sc_fpregs[14], 1),
  O(sc_fpregs[15], 0), O(sc_fpregs[16], 0), O(sc_fpregs[17], 1),
  O(sc_fpregs[18], 0), O(sc_fpregs[19], 0), O(sc_fpregs[20], 1),
  O(sc_fpregs[21], 0), O(sc_fpregs[22], 0), O(sc_fpregs[23], 1),
  O(sc_fpregs[24], 0), O(sc_fpregs[25], 0), O(sc_fpregs[26], 1),
  O(sc_fpregs[27], 0), O(sc_fpregs[28], 0), O(sc_fpregs[29], 1),
  O(sc_fpregs[30], 0), O(sc_fpcr, 2),

  O(sc_traparg_a0, 0),  O(sc_traparg_a1, 0),  O(sc_traparg_a2, 1)
};

#undef O

static void
register_dump (int fd, struct sigcontext *ctx)
{
  char buf[NREGS*(8+16) + 25 + 80];
  char *p = buf;
  size_t i;

  p = stpcpy (p, "Register dump:\n\n");

  for (i = 0; i < NREGS; ++i)
    {
      int this_offset, this_lf;
      unsigned long val;
      signed long j;

      this_offset = offsets[i];
      this_lf = this_offset & 7;

      val = *(unsigned long *)(((size_t)ctx + this_offset) & -8);

      memcpy (p, regnames[i], 8);
      p += 8;

      for (j = 60; j >= 0; j -= 4)
	{
	  unsigned long x = (val >> j) & 15;
	  x += x < 10 ? '0' : 'a' - 10;
	  *p++ = x;
	}

      if (this_lf > 0)
	{
	  if (this_lf > 1)
	    *p++ = '\n';
	  *p++ = '\n';
	}
    }

  write (fd, buf, p - buf);
}

#define REGISTER_DUMP register_dump (fd, ctx)
