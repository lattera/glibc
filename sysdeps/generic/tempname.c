/* Copyright (C) 1991, 92, 93, 95-98, 99 Free Software Foundation, Inc.
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

#define __need_size_t
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

/* Perform the "SVID path search malarkey" on DIR and PFX.  Write a
   template suitable for use in __gen_tempname into TMPL, bounded
   by TMPL_LEN. */
int
__path_search (tmpl, tmpl_len, dir, pfx, try_tmpdir)
     char *tmpl;
     size_t tmpl_len;
     const char *dir;
     const char *pfx;
     int try_tmpdir;
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (__path_search)

/* Generate a (hopefully) unique temporary filename
   in DIR (if applicable), using template TMPL.
   KIND determines what to do with that name.  It may be one of:
     __GT_FILE:		create a file and return a read-write fd.
     __GT_BIGFILE:	same, but use open64() (or equivalent).
     __GT_DIR:		create a directory.
     __GT_NOCREATE:	just find a name not currently in use.
 */

int
__gen_tempname (tmpl, kind)
     char *tmpl;
     int kind;
{
  __set_errno (ENOSYS);
  return -1;
}

stub_warning (__gen_tempname)
#include <stub-tag.h>
