/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _SYS_KERNEL_TERMIOS_H
#define _SYS_KERNEL_TERMIOS_H 1
/* The following corresponds to the values from the Linux 2.0.28 kernel.  */

/* We need the definition of tcflag_t, cc_t, and speed_t.  */
#include <termbits.h>

#define __KERNEL_NCCS 19

struct __kernel_termios
  {
    tcflag_t c_iflag;		/* input mode flags */
    tcflag_t c_oflag;		/* output mode flags */
    tcflag_t c_cflag;		/* control mode flags */
    tcflag_t c_lflag;		/* local mode flags */
    cc_t c_cc[__KERNEL_NCCS];	/* control characters */
    cc_t c_line;		/* line discipline */
    int c_ispeed;               /* input speed */
    int c_ospeed;               /* output speed */
  };

#define _HAVE_C_ISPEED 1
#define _HAVE_C_OSPEED 1

#endif /* sys/kernel_termios.h */
