/* Copyright (C) 1991, 1993, 1995 Free Software Foundation, Inc.
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
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>


/* Get the value of the system variable NAME.  */
long int
DEFUN(__sysconf, (name), int name)
{
  switch (name)
    {
    default:
      errno = EINVAL;
      return -1;

    case _SC_TZNAME_MAX:
      return __tzname_max ();

    case _SC_ARG_MAX:
    case _SC_CHILD_MAX:
    case _SC_CLK_TCK:
    case _SC_NGROUPS_MAX:
    case _SC_OPEN_MAX:
    case _SC_JOB_CONTROL:
    case _SC_SAVED_IDS:
    case _SC_VERSION:

    case _SC_BC_BASE_MAX:
    case _SC_BC_DIM_MAX:
    case _SC_BC_SCALE_MAX:
    case _SC_BC_STRING_MAX:
    case _SC_EQUIV_CLASS_MAX:
    case _SC_EXPR_NEST_MAX:
    case _SC_LINE_MAX:
    case _SC_RE_DUP_MAX:
    case _SC_2_VERSION:
    case _SC_2_C_BIND:
    case _SC_2_C_DEV:
    case _SC_2_FORT_DEV:
    case _SC_2_SW_DEV:

      break;
    }

  errno = ENOSYS;
  return -1;
}

weak_alias (__sysconf, sysconf)
