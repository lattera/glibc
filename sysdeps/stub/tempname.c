/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <stdio.h>

/* Generate a (hopefully) unique temporary filename
   in DIR (if applicable), using prefix PFX.
   If DIR_SEARCH is nonzero, perform directory searching
   malarky as per the SVID for tempnam.
   Return the generated filename or NULL if one could not
   be generated, putting the length of the string in *LENPTR.  */
char *
DEFUN(__stdio_gen_tempname, (dir, pfx, dir_search, lenptr),
      CONST char *dir AND CONST char *pfx AND
      int dir_search AND size_t *lenptr AND
      FILE **streamptr)
{
  *lenptr = 0;
  errno = ENOSYS;
  return NULL;
}


#ifdef	 HAVE_GNU_LD

#include <gnu-stabs.h>

stub_warning(__stdio_gen_tempname);

#endif	/* GNU stabs.  */
