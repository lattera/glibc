/* Copyright (C) 1992, 1996 Free Software Foundation, Inc.
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

#define NGROUPS_MAX 0		/* No supplementary groups.  */
#define ARG_MAX 5120
#define CHILD_MAX 25
#define OPEN_MAX 60
#define LINK_MAX 1000
#define MAX_CANON 256

/* For SVR3, this is 14.  For SVR4, it is 255, at least on ufs
   file systems, even though the System V limits.h incorrectly
   defines it as 14.  Giving it a value which is too large
   is harmless (it is a maximum).  */
#define NAME_MAX 255

#define PATH_MAX 1024
