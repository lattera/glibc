/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#define	__NO_CTYPE
#include <ctype.h>

/* Provide real-function versions of all the ctype macros.  */

#define	func(name, type) \
  int DEFUN(name, (c), int c) { return __isctype(c, type); }

func(isalnum, _ISalnum)
func(isalpha, _ISalpha)
func(iscntrl, _IScntrl)
func(isdigit, _ISdigit)
func(islower, _ISlower)
func(isgraph, _ISgraph)
func(isprint, _ISprint)
func(ispunct, _ISpunct)
func(isspace, _ISspace)
func(isupper, _ISupper)
func(isxdigit, _ISxdigit)

int
DEFUN(tolower, (c), int c)
{
  return __tolower (c);
}

int
DEFUN(toupper, (c), int c)
{
  return __toupper (c);
}
