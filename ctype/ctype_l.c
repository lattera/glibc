/* Copyright (C) 1991, 1992, 1997, 2001 Free Software Foundation, Inc.
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

#define	__NO_CTYPE
#include <ctype.h>

/* Provide real-function versions of all the ctype macros.  */

#define	func(name, type) \
  int name (int c, __locale_t l) { return __isctype_l (c, type, l); }

func (__isalnum_l, _ISalnum)
func (__isalpha_l, _ISalpha)
func (__iscntrl_l, _IScntrl)
func (__isdigit_l, _ISdigit)
func (__islower_l, _ISlower)
func (__isgraph_l, _ISgraph)
func (__isprint_l, _ISprint)
func (__ispunct_l, _ISpunct)
func (__isspace_l, _ISspace)
func (__isupper_l, _ISupper)
func (__isxdigit_l, _ISxdigit)

int
(__tolower_l) (int c, __locale_t l)
{
  return l->__ctype_tolower[c];
}

int
(__toupper_l) (int c, __locale_t l)
{
  return l->__ctype_toupper[c];
}
