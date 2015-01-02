/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.
   Based on work contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <sys/uio.h>
#include <_itoa.h>

/* We will print the register dump in this format:

 R0:  XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R8:  XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R16: XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R24: XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R32: XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R40: XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R48: XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R52: XXXXXXXX  TP: XXXXXXXX  SP: XXXXXXXX  LR: XXXXXXXX

 PC:  XXXXXXXX  ICS: X  FAULTNUM: XX

 */

static void
hexvalue (unsigned long int value, char *buf, size_t len)
{
  char *cp = _itoa_word (value, buf + len, 16, 0);
  while (cp > buf)
    *--cp = '0';
}

static void
register_dump (int fd, mcontext_t *ctx)
{
  char regs[59][8];
  struct iovec iov[143];
  size_t nr = 0;
  unsigned int i;

#define ADD_STRING(str) \
  iov[nr].iov_base = (char *) str;					      \
  iov[nr].iov_len = strlen (str);					      \
  ++nr
#define ADD_MEM(str, len) \
  iov[nr].iov_base = str;						      \
  iov[nr].iov_len = len;						      \
  ++nr

  /* Generate strings of register contents.  */
  for (i = 0; i < 56; ++i)
    hexvalue (ctx->gregs[i], regs[i], 8);
  hexvalue (ctx->pc, regs[56], 8);
  hexvalue (ctx->ics, regs[57], 1);
  hexvalue (ctx->faultnum, regs[58], 2);

  /* Generate the output.  */
  for (i = 0; i < 52;)
    {
      const char *prefixes[] = {
        "Register dump:\n\n R0:  ",
        "\n R8:  ",
        "\n R16: ",
        "\n R24:  ",
        "\n R32:  ",
        "\n R40:  ",
        "\n R48:  "
      };
      ADD_STRING (prefixes[i / 8]);
      do
        {
          ADD_MEM (regs[i], 8);
          ADD_STRING (" ");
        }
      while (++i % 8 && i < 52);
    }
  ADD_STRING ("\n R52: ");
  ADD_MEM (regs[52], 8);
  ADD_STRING ("  TP: ");
  ADD_MEM (regs[53], 8);
  ADD_STRING ("  SP: ");
  ADD_MEM (regs[54], 8);
  ADD_STRING ("  LR: ");
  ADD_MEM (regs[55], 8);
  ADD_STRING ("\n\n PC:  ");
  ADD_MEM (regs[56], 8);
  ADD_STRING ("  ICS: ");
  ADD_MEM (regs[57], 1);
  ADD_STRING ("  FAULTNUM: ");
  ADD_MEM (regs[58], 2);
  ADD_STRING ("\n");

  /* Write the stuff out.  */
  writev (fd, iov, nr);
}


#define REGISTER_DUMP register_dump (fd, &ctx->uc_mcontext)
