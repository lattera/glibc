/* Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

#include "cname-lookup.h"

/* Provide real-function versions of all the wctype macros.  */

#define	func(name, type) \
  int									      \
  __##name (wint_t wc)							      \
  {									      \
    size_t idx;								      \
									      \
    idx = cname_lookup (wc);						      \
    if (idx == ~((size_t) 0))						      \
      return 0;								      \
									      \
    return __ctype32_b[idx] & type;					      \
  }									      \
  weak_alias (__##name, name)

#undef iswalnum
func (iswalnum, _ISwalnum)
#undef iswalpha
func (iswalpha, _ISwalpha)
#undef iswcntrl
func (iswcntrl, _ISwcntrl)
#undef iswdigit
func (iswdigit, _ISwdigit)
#undef iswlower
func (iswlower, _ISwlower)
#undef iswgraph
func (iswgraph, _ISwgraph)
#undef iswprint
func (iswprint, _ISwprint)
#undef iswpunct
func (iswpunct, _ISwpunct)
#undef iswspace
func (iswspace, _ISwspace)
#undef iswupper
func (iswupper, _ISwupper)
#undef iswxdigit
func (iswxdigit, _ISwxdigit)

wint_t
(towlower) (wc)
     wint_t wc;
{
  size_t idx;

  idx = cname_lookup (wc);
  if (idx == ~((size_t) 0))
    /* Character is not known.  Default action is to simply return it.  */
    return wc;

  return (wint_t) __ctype_toupper[idx];
}

wint_t
(towupper) (wc)
     wint_t wc;
{
  size_t idx;

  idx = cname_lookup (wc);
  if (idx == ~((size_t) 0))
    /* Character is not known.  Default action is to simply return it.  */
    return wc;

  return (wint_t) __ctype_toupper[idx];
}
