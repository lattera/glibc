/* Minimum guaranteed maximum values for system limits.  Hurd version.
   Copyright (C) 1993,94,96,98,2002 Free Software Foundation, Inc.
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

/* GNU has no arbitrary fixed limits on most of these things, so we
   don't define the macros.  Some things are unlimited.  Some are in
   fact limited but the limit is run-time dependent and fetched with
   `sysconf' or `pathconf'.  */

/* This one value is actually constrained by the `struct dirent'
   layout, in which the `d_namlen' member is only 8 bits wide.  */

#define NAME_MAX	255

/* POSIX.1 requires that we define NGROUPS_MAX (though none of the others
   is required).  GNU allows any number of supplementary groups,
   dynamically allocated.  So we pick a number which seems vaguely
   suitable, and `sysconf' will return a number at least as large.  */

#define NGROUPS_MAX	256

/* The maximum number of symbolic links that are allowed in a single file
   name resolution.  When a further link is encountered, the call returns
   ELOOP.  This name is a GNU extension; POSIX.1 has no such limit, and BSD
   calls it MAXSYMLINKS in <sys/param.h>.  (We define the name under
   _BSD_SOURCE even without _GNU_SOURCE because our <sys/param.h> uses it
   to define MAXSYMLINKS.)  */

#if defined __USE_GNU || defined __USE_BSD /* 1003.1a defines this */
#define	SYMLOOP_MAX	8
#endif
