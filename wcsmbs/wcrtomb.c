/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1996.

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

#include <errno.h>
#include <gconv.h>
#include <stdlib.h>
#include <wchar.h>
#include <wcsmbsload.h>

#include <assert.h>

#ifndef EILSEQ
# define EILSEQ EINVAL
#endif


/* This is the private state used if PS is NULL.  */
static mbstate_t state;

size_t
__wcrtomb (char *s, wchar_t wc, mbstate_t *ps)
{
  char buf[MB_CUR_MAX];
  struct gconv_step_data data;
  int status;
  size_t result;
  size_t dummy;

  /* Tell where we want the result.  */
  data.outbuf = s;
  data.outbufend = s + MB_CUR_MAX;
  data.invocation_counter = 0;
  data.internal_use = 1;
  data.is_last = 1;
  data.statep = ps ?: &state;

  /* A first special case is if S is NULL.  This means put PS in the
     initial state.  */
  if (s == NULL)
    {
      data.outbuf = buf;
      wc = L'\0';
    }

  /* Make sure we use the correct function.  */
  update_conversion_ptrs ();

  /* If WC is the NUL character we write into the output buffer the byte
     sequence necessary for PS to get into the initial state, followed
     by a NUL byte.  */
  if (wc == L'\0')
    {
      status = (*__wcsmbs_gconv_fcts.tomb->fct) (__wcsmbs_gconv_fcts.tomb,
						 &data, NULL, NULL, &dummy, 1);

      if (status == GCONV_OK || status == GCONV_EMPTY_INPUT)
	*data.outbuf++ = '\0';
    }
  else
    {
      /* Do a normal conversion.  */
      const char *inbuf = (const char *) &wc;

      status = (*__wcsmbs_gconv_fcts.tomb->fct) (__wcsmbs_gconv_fcts.tomb,
						 &data, &inbuf,
						 inbuf + sizeof (wchar_t),
						 &dummy, 0);
    }

  /* There must not be any problems with the conversion but illegal input
     characters.  The output buffer must be large enough, otherwise the
     definition of MB_CUR_MAX is not correct.  All the other possible
     errors also must not happen.  */
  assert (status == GCONV_OK || status == GCONV_EMPTY_INPUT
	  || status == GCONV_ILLEGAL_INPUT
	  || status == GCONV_INCOMPLETE_INPUT
	  || status == GCONV_FULL_OUTPUT);

  if (status == GCONV_OK || status == GCONV_EMPTY_INPUT
      || status == GCONV_FULL_OUTPUT)
    result = data.outbuf - s;
  else
    {
      result = (size_t) -1;
      __set_errno (EILSEQ);
    }

  return result;
}
weak_alias (__wcrtomb, wcrtomb)
