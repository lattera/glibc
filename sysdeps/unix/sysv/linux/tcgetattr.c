/* Copyright (C) 1992, 1995, 1997 Free Software Foundation, Inc.
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

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* The difference here is that the termios structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_termios.h>

/* Put the state of FD into *TERMIOS_P.  */
int
__tcgetattr (fd, termios_p)
      int fd;
      struct termios *termios_p;
{
  struct __kernel_termios k_termios;
  int retval;
  size_t cnt;

  retval = __ioctl (fd, TCGETS, &k_termios);

  termios_p->c_iflag = k_termios.c_iflag;
  termios_p->c_oflag = k_termios.c_oflag;
  termios_p->c_cflag = k_termios.c_cflag;
  termios_p->c_lflag = k_termios.c_lflag;
  termios_p->c_line = k_termios.c_line;
#ifdef _HAVE_C_ISPEED
  termios_p->c_ispeed = k_termios.c_ispeed;
#endif
#ifdef _HAVE_C_OSPEED
  termios_p->c_ospeed = k_termios.c_ospeed;
#endif
  for (cnt = 0; cnt < __KERNEL_NCCS; ++cnt)
    termios_p->c_cc[cnt] = k_termios.c_cc[cnt];

  return retval;
}

weak_alias (__tcgetattr, tcgetattr)
