/* Dump registers.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

static const char *regnames[] =
{
  "\nr0 =", " sp =", " r2 =", " r3 =", " r4 =", " r5 =",
  "\nr6 =", " r7 =", " r8 =", " r9 =", " r10=", " r11=",
  "\nr12=", " r13=", " r14=", " r15=", " r16=", " r17=",
  "\nr18=", " r19=", " r20=", " r21=", " r22=", " r23=",
  "\nr24=", " r25=", " r26=", " r27=", " r28=", " r29=",
  "\nr30=", " r31=", " nip=", " msr=", " r3*=", " ctr=",
  "\nlr =", " xer=", " ccr=", " mq =", " trap=",
  "\naddress of fault=", " dsisr=",
};

static void
register_dump (int fd, void **ctx)
{
  char buffer[(sizeof (regnames) / sizeof (regnames[0])) * 8];
  char *bufferpos = buffer + sizeof (buffer);
  struct iovec iov[(sizeof (regnames) / sizeof (regnames[0])) * 2 + 1];
  int nr = 0;

#define ADD_STRING(str) \
  iov[nr].iov_base = (char *) str;					      \
  iov[nr].iov_len = strlen (str);					      \
  ++nr
#define ADD_HEX(str, len) \
  do {									      \
    char *s = _itoa_word ((unsigned long int) x, bufferpos, 16, 0);	      \
    while (bufferpos - s < 8)						      \
      *--s = '0';							      \
    iov[nr].iov_base = s;						      \
    iov[nr].iov_len = bufferpos - s;					      \
    bufferpos = s;							      \
  } while (0)

  /* Generate the output.  */
  ADD_STRING ("Register dump:\n\n");
  for (i = 0; i < sizeof (regnames)  / sizeof (regnames[0]); i++)
    {
      ADD_STRING (regnames[i]);
      ADD_HEX (ctx[i]);
    }

  /* Write the output.  */
  writev (fd, iov, nr);
}


#define REGISTER_DUMP \
  ctx += 8;	/* FIXME!!!!  Why is this necessary?  Is it necessary?  */    \
  register_dump (fd, ctx)
