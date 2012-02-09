/* Copyright (C) 1996, 1997, 1998, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stddef.h>
#include <libintl.h>

/* This is a list of all known signal numbers.  */

const char *const _sys_siglist[] =
  {
    N_("Signal 0"),
    N_("Hangup"),
    N_("Interrupt"),
    N_("Quit"),
    N_("Illegal instruction"),
    N_("Trace/breakpoint trap"),
    N_("IOT trap"),
    N_("EMT trap"),
    N_("Floating point exception"),
    N_("Killed"),
    N_("Bus error"),
    N_("Segmentation fault"),
    N_("Bad system call"),
    N_("Broken pipe"),
    N_("Alarm clock"),
    N_("Terminated"),
    N_("Urgent I/O condition"),
    N_("Stopped (signal)"),
    N_("Stopped"),
    N_("Continued"),
    N_("Child exited"),
    N_("Stopped (tty input)"),
    N_("Stopped (tty output)"),
    N_("I/O possible"),
    N_("CPU time limit exceeded"),
    N_("File size limit exceeded"),
    N_("Virtual timer expired"),
    N_("Profiling timer expired"),
    N_("Window changed"),
    N_("Resource lost"),
    N_("User defined signal 1"),
    N_("User defined signal 2"),
    NULL
  };
strong_alias (_sys_siglist, _sys_siglist_internal)

weak_alias (_sys_siglist, sys_siglist)
