/* Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
#include "wchar-lookup.h"

/* If the program is compiled without optimization the following declaration
   is not visible in the header.   */
extern unsigned int *__ctype32_b;

/* These are not exported.  */
extern const uint32_t *__ctype32_toupper;
extern const uint32_t *__ctype32_tolower;
extern const char *__ctype32_wctype[12];
extern const char *__ctype32_wctrans[2];

/* Provide real-function versions of all the wctype macros.  */

#define	func(name, type) \
  int									      \
  __##name (wint_t wc)							      \
  {									      \
    if (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_HASH_SIZE) != 0)		      \
      {									      \
	/* Old locale format.  */					      \
	size_t idx;							      \
									      \
	idx = cname_lookup (wc);					      \
	if (idx == ~((size_t) 0))					      \
	  return 0;							      \
									      \
	return __ctype32_b[idx] & _ISwbit (type);			      \
      }									      \
    else								      \
      {									      \
	/* New locale format.  */					      \
	return wctype_table_lookup (__ctype32_wctype[type], wc);	      \
      }									      \
  }									      \
  weak_alias (__##name, name)

#undef iswalnum
func (iswalnum, __ISwalnum)
#undef iswalpha
func (iswalpha, __ISwalpha)
#undef iswcntrl
func (iswcntrl, __ISwcntrl)
#undef iswdigit
func (iswdigit, __ISwdigit)
#undef iswlower
func (iswlower, __ISwlower)
#undef iswgraph
func (iswgraph, __ISwgraph)
#undef iswprint
func (iswprint, __ISwprint)
#undef iswpunct
func (iswpunct, __ISwpunct)
#undef iswspace
func (iswspace, __ISwspace)
#undef iswupper
func (iswupper, __ISwupper)
#undef iswxdigit
func (iswxdigit, __ISwxdigit)

wint_t
(towlower) (wc)
     wint_t wc;
{
  if (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_HASH_SIZE) != 0)
    {
      /* Old locale format.  */
      size_t idx;

      idx = cname_lookup (wc);
      if (idx == ~((size_t) 0))
	/* Character is not known.  Default action is to simply return it.  */
	return wc;

      return (wint_t) __ctype32_tolower[idx];
    }
  else
    {
      /* New locale format.  */
      return wctrans_table_lookup (__ctype32_wctrans[1], wc);
    }
}

wint_t
(towupper) (wc)
     wint_t wc;
{
  if (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_HASH_SIZE) != 0)
    {
      /* Old locale format.  */
      size_t idx;

      idx = cname_lookup (wc);
      if (idx == ~((size_t) 0))
	/* Character is not known.  Default action is to simply return it.  */
	return wc;

      return (wint_t) __ctype32_toupper[idx];
    }
  else
    {
      /* New locale format.  */
      return wctrans_table_lookup (__ctype32_wctrans[0], wc);
    }
}
