/* Copyright (C) 1992-2015 Free Software Foundation, Inc.
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

/* We need to avoid the header declaration of scandir64, because
   the types don't match scandir and then the compiler will
   complain about the mismatch when we do the alias below.  */
#define scandir64       __renamed_scandir64

#include <dirent.h>

#undef  scandir64

#include <fcntl.h>

#ifndef SCANDIR
# define SCANDIR scandir
# define SCANDIRAT scandirat
# define DIRENT_TYPE struct dirent
#endif


int
SCANDIR (dir, namelist, select, cmp)
     const char *dir;
     DIRENT_TYPE ***namelist;
     int (*select) (const DIRENT_TYPE *);
     int (*cmp) (const DIRENT_TYPE **, const DIRENT_TYPE **);
{
  return SCANDIRAT (AT_FDCWD, dir, namelist, select, cmp);
}

#ifdef _DIRENT_MATCHES_DIRENT64
weak_alias (scandir, scandir64)
#endif
