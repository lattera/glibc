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
#include <sys/resource.h>
#include <errno.h>
#include <sys/syssgi.h>

extern int __syssgi __P ((int, ...));

/* Return resource usage information on process indicated by WHO
   and put it in *USAGE.  Returns 0 for success, -1 for failure.  */
int
DEFUN(__getrusage, (who, usage),
      enum __rusage_who who AND struct rusage *usage)
{
  return __syssgi (SGI_RUSAGE, who, usage);
}
