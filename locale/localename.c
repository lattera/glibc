/* current locale setting names
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#include "localeinfo.h"

/* Name of current locale for each individual category.
   Each is malloc'd unless it is _nl_C_name.  */
const char *_nl_current_names[] attribute_hidden =
  {
#define DEFINE_CATEGORY(category, category_name, items, a) \
    [category] = _nl_C_name,
#include "categories.def"
#undef	DEFINE_CATEGORY
    [LC_ALL] = _nl_C_name		/* For LC_ALL.  */
  };

const char *
attribute_hidden
__current_locale_name (int category)
{
  return (_NL_CURRENT_LOCALE == &_nl_global_locale
	  ? _nl_current_names[category]
	  : _NL_CURRENT_LOCALE->__locales[category]->name);
}
