/* Copyright (C) 1993, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>

/* The difference here is that the termios structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_termios.h>


/* Set the state of FD to *TERMIOS_P.  */
int
tcsetattr (fd, optional_actions, termios_p)
     int fd;
     int optional_actions;
     const struct termios *termios_p;
{
  struct __kernel_termios k_termios;
  unsigned long int cmd;

  switch (optional_actions)
    {
    case TCSANOW:
      cmd = TCSETS;
      break;
    case TCSADRAIN:
      cmd = TCSETSW;
      break;
    case TCSAFLUSH:
      cmd = TCSETSF;
      break;
    default:
      __set_errno (EINVAL);
      return -1;
    }

  k_termios.c_iflag = termios_p->c_iflag;
  k_termios.c_oflag = termios_p->c_oflag;
  k_termios.c_cflag = termios_p->c_cflag;
  k_termios.c_lflag = termios_p->c_lflag;
  k_termios.c_line = termios_p->c_line;
#ifdef _HAVE_C_ISPEED
  k_termios.c_ispeed = termios_p->c_ispeed;
#endif
#ifdef _HAVE_C_OSPEED
  k_termios.c_ospeed = termios_p->c_ospeed;
#endif
  memcpy (&k_termios.c_cc[0], &termios_p->c_cc[0],
	  __KERNEL_NCCS * sizeof (cc_t));

  return __ioctl (fd, cmd, &k_termios);
}
