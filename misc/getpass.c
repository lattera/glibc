/* Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

/* It is desireable to use this bit on systems that have it.
   The only bit of terminal state we want to twiddle is echoing, which is
   done in software; there is no need to change the state of the terminal
   hardware.  */

#ifndef TCSASOFT
#define TCSASOFT 0
#endif

char *
getpass (prompt)
     const char *prompt;
{
  FILE *in, *out;
  struct termios t;
  int echo_off;
  static char *buf = NULL;
  static size_t bufsize = 0;
  ssize_t nread;

  /* Try to write to and read from the terminal if we can.
     If we can't open the terminal, use stderr and stdin.  */

  in = fopen ("/dev/tty", "w+");
  if (in == NULL)
    {
      in = stdin;
      out = stderr;
    }
  else
    out = in;

  /* Turn echoing off if it is on now.  */

  if (tcgetattr (fileno (in), &t) == 0)
    {
      if (t.c_lflag & ECHO)
	{
	  t.c_lflag &= ~ECHO;
	  echo_off = tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t) == 0;
	  t.c_lflag |= ECHO;
	}
      else
	echo_off = 0;
    }
  else
    echo_off = 0;

  /* Write the prompt.  */
  fputs (prompt, out);
  fflush (out);

  /* Read the password.  */
  nread = __getline (&buf, &bufsize, in);
  if (nread < 0 && buf != NULL)
    buf[0] = '\0';
  else if (buf[nread - 1] == '\n')
    /* Remove the newline.  */
    buf[nread - 1] = '\0';

  /* Restore echoing.  */
  if (echo_off)
    (void) tcsetattr (fileno (in), TCSAFLUSH|TCSASOFT, &t);

  if (in != stdin)
    /* We opened the terminal; now close it.  */
    fclose (in);

  return buf;
}
