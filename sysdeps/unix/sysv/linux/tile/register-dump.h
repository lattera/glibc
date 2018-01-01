/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

 R0:  XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R4:  XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R8:  XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R12: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R16: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R20: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R24: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R28: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R32: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R36: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R40: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R44: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R48: XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX
 R52: XXXXXXXXXXXXXXXX  TP: XXXXXXXXXXXXXXXX
 SP:  XXXXXXXXXXXXXXXX  LR: XXXXXXXXXXXXXXXX

 PC:  XXXXXXXXXXXXXXXX  ICS: X  FAULTNUM: XX

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
  char regs[59][16];
  struct iovec iov[132];
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
    hexvalue (ctx->gregs[i], regs[i], 16);
  hexvalue (ctx->pc, regs[56], 16);
  hexvalue (ctx->ics, regs[57], 1);
  hexvalue (ctx->faultnum, regs[58], 2);

  /* Generate the output.  */
  for (i = 0; i < 56;)
    {
      const char *prefixes[] = {
        "Register dump:\n\n R0:  ",
        "\n R4:  ",
        "\n R8:  ",
        "\n R12: ",
        "\n R16: ",
        "\n R20:  ",
        "\n R24:  ",
        "\n R28:  ",
        "\n R32:  ",
        "\n R36:  ",
        "\n R40:  ",
        "\n R44:  ",
        "\n R48:  "
      };
      ADD_STRING (prefixes[i / 4]);
      do
        {
          ADD_MEM (regs[i], 16);
          ADD_STRING (" ");
        }
      while (++i % 4);
    }
  ADD_STRING ("\n R52: ");
  ADD_MEM (regs[52], 16);
  ADD_STRING ("  TP: ");
  ADD_MEM (regs[53], 16);
  ADD_STRING ("\n SP: ");
  ADD_MEM (regs[54], 16);
  ADD_STRING ("  LR: ");
  ADD_MEM (regs[55], 16);
  ADD_STRING ("\n\n PC:  ");
  ADD_MEM (regs[56], 16);
  ADD_STRING ("  ICS: ");
  ADD_MEM (regs[57], 1);
  ADD_STRING ("  FAULTNUM: ");
  ADD_MEM (regs[58], 2);
  ADD_STRING ("\n");

  /* Write the stuff out.  */
  writev (fd, iov, nr);
}


#define REGISTER_DUMP register_dump (fd, &ctx->uc_mcontext)
