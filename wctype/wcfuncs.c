/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#define	__NO_WCTYPE
#include <wctype.h>
#include <ctype.h>	/* For __ctype_tolower and __ctype_toupper.  */

/* Provide real-function versions of all the wctype macros.  */

#define	func(name, type) \
  int name (wc) wint_t wc; { return __iswctype (wc, type); }

func (iswalnum, _ISwalnum)
func (iswalpha, _ISwalpha)
func (iswcntrl, _ISwcntrl)
func (iswdigit, _ISwdigit)
func (iswlower, _ISwlower)
func (iswgraph, _ISwgraph)
func (iswprint, _ISwprint)
func (iswpunct, _ISwpunct)
func (iswspace, _ISwspace)
func (iswupper, _ISwupper)
func (iswxdigit, _ISwxdigit)

wint_t
towlower (wc)
     wint_t wc;
{
  return __towctrans (wc, __ctype_tolower);
}

wint_t
towupper (wc)
     wint_t wc;
{
  return __towctrans (wc, __ctype_toupper);
}
