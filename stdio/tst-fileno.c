/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <stdio.h>
#include <unistd.h>

static int
DEFUN(check, (name, stream, fd), CONST char *name AND FILE *stream AND int fd)
{
  int sfd = fileno (stream);
  printf ("(fileno (%s) = %d) %c= %d\n", name, sfd, sfd == fd ? '=' : '!', fd);
  return sfd != fd;
}

int
DEFUN_VOID(main)
{
  exit (check ("stdin", stdin, STDIN_FILENO) ||
	check ("stdout", stdout, STDOUT_FILENO) ||
	check ("stderr", stderr, STDERR_FILENO));
}
