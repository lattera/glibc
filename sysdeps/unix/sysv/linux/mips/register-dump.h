/* Dump registers.
   Copyright (C) 2000, 2001, 2002, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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

 R0   XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R8   XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R16  XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 R24  XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
            pc       lo       hi
      XXXXXXXX XXXXXXXX XXXXXXXX
 The FPU registers will not be printed.
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
  char regs[38][8];
  struct iovec iov[38 * 2 + 10];
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
  for (i = 0; i < 32; i++)
    hexvalue (ctx->sc_regs[i], regs[i], 8);
  hexvalue (ctx->sc_pc, regs[32], 8);
  hexvalue (ctx->sc_mdhi, regs[33], 8);
  hexvalue (ctx->sc_mdlo, regs[34], 8);

  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\n R0   ");
  for (i = 0; i < 8; i++)
    {
      ADD_MEM (regs[i], 8);
      ADD_STRING (" ");
    }
  ADD_STRING ("\n R8   ");
  for (i = 8; i < 16; i++)
    {
      ADD_MEM (regs[i], 8);
      ADD_STRING (" ");
    }
  ADD_STRING ("\n R16  ");
  for (i = 16; i < 24; i++)
    {
      ADD_MEM (regs[i], 8);
      ADD_STRING (" ");
    }
  ADD_STRING ("\n R24  ");
  for (i = 24; i < 32; i++)
    {
      ADD_MEM (regs[i], 8);
      ADD_STRING (" ");
    }
  ADD_STRING ("\n            pc       lo       hi\n      ");
  for (i = 32; i < 35; i++)
    {
      ADD_MEM (regs[i], 8);
      ADD_STRING (" ");
    }
  ADD_STRING ("\n");

  /* Write the stuff out.  */
  writev (fd, iov, nr);
}


#define REGISTER_DUMP register_dump (fd, ctx)
