/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 2002.

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

#include <ctype.h>
#include <string.h>
#include "wcsmbsload.h"


static inline struct __gconv_step *
wcsmbs_get_towc_func (__locale_t l)
{
  const char *charset;
  int use_translit;
  char *norm;
  size_t nsteps;

  charset = l->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX(CODESET)].string;

  /* Transliteration requested?  */
  use_translit = l->__locales[LC_CTYPE]->use_translit;

  /* Normalize the name.  */
  norm = norm_add_slashes (charset, use_translit ? "TRANSLIT" : NULL);

  return __wcsmbs_getfct ("INTERNAL", charset, &nsteps) ?: &__wcsmbs_to_wc;
}


static inline void
wcsmbs_free_funcs (struct __gconv_step *step)
{
  if (step != &__wcsmbs_to_wc)
    /* There is only one step.  */
    __gconv_close_transform (step, 1);
}


#define USE_IN_EXTENDED_LOCALE_MODEL	1
#include "mbsrtowcs.c"
