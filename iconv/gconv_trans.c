/* Transliteration using the locale's data.
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

#include <stdint.h>

#include "gconv_int.h"
#include "../locale/localeinfo.h"


int
gconv_transliterate (struct __gconv_step *step,
		     struct __gconv_step_data *step_data,
		     __const unsigned char *inbufstart,
		     __const unsigned char **inbufp,
		     __const unsigned char *inbufend,
		     unsigned char *outbufstart,
		     unsigned char **outbufp, unsigned char *outbufend,
		     size_t *irreversible)
{
  /* Find out about the locale's transliteration.  */
  uint_fast32_t size = _NL_CURRENT_WORD (LC_CTYPE,
					 _NL_CTYPE_TRANSLIT_HASH_SIZE);
  uint_fast32_t layers = _NL_CURRENT_WORD (LC_CTYPE,
					   _NL_CTYPE_TRANSLIT_HASH_LAYERS);

  /* If there is no transliteration information in the locale don't do
     anything and return the error.  */
  if (size == 0)
    return __GCONV_ILLEGAL_INPUT;

  /* XXX For now we don't do anything.  */
  return __GCONV_ILLEGAL_INPUT;
}
