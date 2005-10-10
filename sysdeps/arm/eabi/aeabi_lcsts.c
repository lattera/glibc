/* Link-time constants for ARM EABI.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

/* The ARM EABI requires that we provide ISO compile-time constants as
   link-time constants.  Some portable applications may reference these.  */

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#define eabi_constant2(X,Y) const int __aeabi_##X attribute_hidden = Y
#define eabi_constant(X) const int __aeabi_##X attribute_hidden = X

eabi_constant (EDOM);
eabi_constant (ERANGE);
eabi_constant (EILSEQ);

eabi_constant (MB_LEN_MAX);

eabi_constant (LC_COLLATE);
eabi_constant (LC_CTYPE);
eabi_constant (LC_MONETARY);
eabi_constant (LC_NUMERIC);
eabi_constant (LC_TIME);
eabi_constant (LC_ALL);

/* The value of __aeabi_JMP_BUF_SIZE is the number of doublewords in a
   jmp_buf.  */
eabi_constant2 (JMP_BUF_SIZE, sizeof (jmp_buf) / 8);

eabi_constant (SIGABRT);
eabi_constant (SIGFPE);
eabi_constant (SIGILL);
eabi_constant (SIGINT);
eabi_constant (SIGSEGV);
eabi_constant (SIGTERM);

eabi_constant2 (IOFBF, _IOFBF);
eabi_constant2 (IOLBF, _IOLBF);
eabi_constant2 (IONBF, _IONBF);
eabi_constant (BUFSIZ);
eabi_constant (FOPEN_MAX);
eabi_constant (TMP_MAX);
eabi_constant (FILENAME_MAX);
eabi_constant (L_tmpnam);

eabi_constant (CLOCKS_PER_SEC);
