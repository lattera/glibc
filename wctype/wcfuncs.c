/* Copyright (C) 1996-2001, 2002 Free Software Foundation, Inc.
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

#include <wctype.h>
#include <locale/localeinfo.h>

#include "wchar-lookup.h"

/* Provide real-function versions of all the wctype macros.  */

#define	func(name, type)						      \
  extern int __##name (wint_t __wc);					      \
  int									      \
  __##name (wint_t wc)							      \
  {									      \
    size_t i = _NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_CLASS_OFFSET) + type;    \
    const char *desc = _NL_CURRENT (LC_CTYPE, i);			      \
    return wctype_table_lookup (desc, wc);				      \
  }									      \
  weak_alias (__##name, name)

#undef iswalnum
func (iswalnum, __ISwalnum)
libc_hidden_weak (iswalnum)
#undef iswalpha
func (iswalpha, __ISwalpha)
libc_hidden_weak (iswalpha)
#undef iswblank
func (iswblank, __ISwblank)
#undef iswcntrl
func (iswcntrl, __ISwcntrl)
#undef iswdigit
func (iswdigit, __ISwdigit)
libc_hidden_weak (iswdigit)
#undef iswlower
func (iswlower, __ISwlower)
libc_hidden_weak (iswlower)
#undef iswgraph
func (iswgraph, __ISwgraph)
#undef iswprint
func (iswprint, __ISwprint)
#undef iswpunct
func (iswpunct, __ISwpunct)
#undef iswspace
func (iswspace, __ISwspace)
libc_hidden_weak (iswspace)
#undef iswupper
func (iswupper, __ISwupper)
#undef iswxdigit
func (iswxdigit, __ISwxdigit)
libc_hidden_weak (iswxdigit)

#undef towlower
wint_t
towlower (wc)
     wint_t wc;
{
  size_t i = _NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_MAP_OFFSET) + __TOW_tolower;
  const char *desc = _NL_CURRENT (LC_CTYPE, i);
  return wctrans_table_lookup (desc, wc);
}
libc_hidden_def (towlower)

#undef towupper
wint_t
towupper (wc)
     wint_t wc;
{
  size_t i = _NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_MAP_OFFSET) + __TOW_toupper;
  const char *desc = _NL_CURRENT (LC_CTYPE, i);
  return wctrans_table_lookup (desc, wc);
}
libc_hidden_def (towupper)
