/* Copyright (C) 1991, 92, 93, 95, 96 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdio.h>

/* Generate a (hopefully) unique temporary filename
   in DIR (if applicable), using prefix PFX.
   If DIR_SEARCH is nonzero, perform directory searching
   malarkey as per the SVID for tempnam.
   Return the generated filename or NULL if one could not
   be generated, putting the length of the string in *LENPTR.  */
char *
__stdio_gen_tempname (buf, bufsize, dir, pfx, dir_search, lenptr, streamptr)
     char *buf;
     size_t bufsize;
     const char *dir;
     const char *pfx;
     int dir_search;
     size_t *lenptr;
     FILE **streamptr;
{
  *lenptr = 0;
  __set_errno (ENOSYS);
  return NULL;
}


stub_warning (__stdio_gen_tempname)
