/* Internal function for converting integers to string using locale
   specific digits.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2000.

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

#ifndef _I18N_ITOWA_H
#define _I18N_ITOWA_H
#include <sys/cdefs.h>

#include "../locale/outdigitswc.h"


extern wchar_t *_i18n_itowa (unsigned long long int value, wchar_t *buflim);

static inline wchar_t *
_i18n_itowa_word (unsigned long int value, wchar_t *buflim)
{
  do
    *--buflim = outdigitwc_value (value % 10);
  while ((value /= 10) != 0);					      \

  return buflim;
}

#endif	/* _i18n_itoa.h */
