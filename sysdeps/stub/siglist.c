/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <stddef.h>

CONST char *CONST _sys_siglist[] =
  {
    "Signal 0",
    "Aborted",
    "Erroneous arithmetic operation",
    "Illegal instruction",
    "Interrupt",
    "Invalid access to storage",
    "Terminated",
    "Hangup",
    "Quit",
    "Broken pipe",
    "Killed",
    "Alarm clock",
    "Stopped (signal)",
    "Stopped",
    "Continued",
    "Child exited",
    "Stopped (tty input)",
    "Stopped (tty output)",
    NULL
  };
