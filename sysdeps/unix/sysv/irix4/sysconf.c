/* Copyright (C) 1994, 1995, 1997, 2002, 2004 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sys/syssgi.h>

extern int __syssgi ((int, ...);

/* Get the value of the system variable NAME.  */
long int
__sysconf (name)
     int name;
{
  if (name == _SC_TZNAME_MAX)
    return __tzname_max ();

  return __syssgi (SGI_SYSCONF, name);
}

weak_alias (__sysconf, sysconf)
libc_hidden_def (__sysconf)
