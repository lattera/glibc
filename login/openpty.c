/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

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

#include <fcntl.h>
#include <pty.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>

#include "pty-private.h"

int
openpty (pmast, pslave, pname, tio, wins)
     int *pmast;
     int *pslave;
     char *pname;
     struct termios *tio;
     struct winsize *wins;
{
  int pfd, tfd;
  char name[PTYNAMELEN];

  pfd = getpt ();
  if (pfd == -1)
    return -1;

  if (grantpt (pfd))
    goto bail;

  if (unlockpt (pfd))
    goto bail;

  if (ptsname_r (pfd, name, PTYNAMELEN) != 0)
    goto bail;

  tfd = open (name, O_RDWR);
  if (tfd == -1)
    goto bail;

  /* XXX Should we ignore errors here?  */
  if(tio)
    tcsetattr (tfd, TCSAFLUSH, tio);
  if (wins)
    ioctl (tfd, TIOCSWINSZ, wins);

  *pmast = pfd;
  *pslave = tfd;
  if (pname != NULL)
    strcpy (pname, name);
  return 0;

bail:
  close (pfd);
  return -1;
}
