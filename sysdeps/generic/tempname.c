/* Copyright (C) 1991, 92, 93, 95, 96, 97, 98 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define __need_size_t
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

/* Perform the "SVID path search malarkey" on DIR and PFX.  Write a
   template suitable for use in __gen_tempname into TMPL, bounded
   by TMPL_LEN. */
int
__path_search (tmpl, tmpl_len, dir, pfx)
     char *tmpl;
     size_t tmpl_len;
     const char *dir;
     const char *pfx;
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (__path_search)

/* Generate a (hopefully) unique temporary filename
   in DIR (if applicable), using template TMPL.
   If OPENIT is 1, open the file and return a fd.  If LARGEFILE is 1,
   use open64() to do that. */
int
__gen_tempname (tmpl, openit, largefile)
     char *tmpl;
     int openit;
     int largefile;
{
  __set_errno (ENOSYS);
  return -1;
}

stub_warning (__gen_tempname)
#include <stub-tag.h>
