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
#include <stdlib.h>
#include <gconv.h>
#include <wchar.h>
#include <wcsmbsload.h>

#include <assert.h>

#ifndef EILSEQ
# define EILSEQ EINVAL
#endif


/* This is the private state used if PS is NULL.  */
static mbstate_t state;

size_t
__wcsrtombs (dst, src, len, ps)
     char *dst;
     const wchar_t **src;
     size_t len;
     mbstate_t *ps;
{
  struct gconv_step_data data;
  int status;
  size_t result = 0;

  /* Tell where we want the result.  */
  data.is_last = 1;
  data.statep = ps ?: &state;

  /* Make sure we use the correct function.  */
  update_conversion_ptrs ();

  /* We have to handle DST == NULL special.  */
  if (dst == NULL)
    {
      char buf[256];		/* Just an arbitrary value.  */
      const wchar_t *srcend = *src + __wcslen (*src) + 1;
      const wchar_t *inbuf = *src;
      size_t written;

      data.outbuf = buf;
      data.outbufend = buf + sizeof (buf);

      do
	{
	  status = (*__wcsmbs_gconv_fcts.tomb->fct) (__wcsmbs_gconv_fcts.tomb,
						     &data,
						     (const char **) &inbuf,
						     (const char *) srcend,
						     &written, 0);
	  result += written;
	}
      while (status == GCONV_FULL_OUTPUT);

      if ((status == GCONV_OK || status == GCONV_EMPTY_INPUT)
	  && data.outbuf[-1] == '\0')
	/* Don't count the NUL character in.  */
	--result;
    }
  else
    {
      /* This code is based on the safe assumption that all internal
	 multi-byte encodings use the NUL byte only to mark the end
	 of the string.  */
      const wchar_t *srcend = *src + __wcsnlen (*src, len * MB_CUR_MAX) + 1;

      data.outbuf = dst;
      data.outbufend = dst + len;

      status = (*__wcsmbs_gconv_fcts.tomb->fct) (__wcsmbs_gconv_fcts.tomb,
						 &data, (const char **) src,
						 (const char *) srcend,
						 &result, 0);

      /* We have to determine whether the last character converted
	 is the NUL character.  */
      if ((status == GCONV_OK || status == GCONV_EMPTY_INPUT
	   || status == GCONV_FULL_OUTPUT)
	  && data.outbuf[-1] == '\0')
	{
	  assert (data.outbuf != dst);
	  assert (__mbsinit (data.statep));
	  *src = NULL;
	  --result;
	}
    }

  /* There must not be any problems with the conversion but illegal input
     characters.  */
  assert (status == GCONV_OK || status == GCONV_EMPTY_INPUT
	  || status == GCONV_ILLEGAL_INPUT
	  || status == GCONV_INCOMPLETE_INPUT || status == GCONV_FULL_OUTPUT);

  if (status != GCONV_OK && status != GCONV_FULL_OUTPUT
      && status != GCONV_EMPTY_INPUT)
    {
      result = (size_t) -1;
      __set_errno (EILSEQ);
    }

  return result;
}
weak_alias (__wcsrtombs, wcsrtombs)
