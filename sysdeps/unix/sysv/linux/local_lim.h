/* Minimum guaranteed maximum values for system limits.  Hurd version.

Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

/* Linux has a fixed limit of supplementary groups allocated with a
   process.  This value is determined by the size of the `groups'
   member of the `task_struct' structure in <linux/sched.h>.  */
   
#define NGROUPS_MAX	32


/* Maximum size of file names.  Not all file system types support
   this size but it is only a maximum value.  */

#define NAME_MAX	255
