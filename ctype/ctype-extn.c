/* Copyright (C) 1991 Free Software Foundation, Inc.
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

/* Real function versions of the non-ANSI ctype functions.  */

int DEFUN(isblank, (c), int c) { return __isctype ((c), _ISblank); }

int DEFUN(_tolower, (c), int c) { return __tolower(c); }
int DEFUN(_toupper, (c), int c) { return __toupper(c); }

int DEFUN(toascii, (c), int c) { return __toascii(c); }
int DEFUN(isascii, (c), int c) { return __isascii(c); }
