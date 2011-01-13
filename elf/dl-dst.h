/* Handling of dynamic sring tokens.
   Copyright (C) 1999,2001-2004,2006,2007,2011 Free Software Foundation, Inc.
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

#include "trusted-dirs.h"

/* Determine the number of DST elements in the name.  Only if IS_PATH is
   nonzero paths are recognized (i.e., multiple, ':' separated filenames).  */
#define DL_DST_COUNT(name, is_path) \
  ({									      \
    size_t __cnt = 0;							      \
    const char *__sf = strchr (name, '$');				      \
									      \
    if (__builtin_expect (__sf != NULL, 0))				      \
      __cnt = _dl_dst_count (__sf, is_path);			      \
									      \
    __cnt; })


#ifdef SHARED
# define IS_RTLD(l) (l) == &GL(dl_rtld_map)
#else
# define IS_RTLD(l) 0
#endif
/* Guess from the number of DSTs the length of the result string.  */
#define DL_DST_REQUIRED(l, name, len, cnt) \
  ({									      \
    size_t __len = (len);						      \
    size_t __cnt = (cnt);						      \
									      \
    if (__cnt > 0)							      \
      {									      \
	size_t dst_len;							      \
	/* Now we make a guess how many extra characters on top of the	      \
	   length of S we need to represent the result.  We know that	      \
	   we have CNT replacements.  Each at most can use		      \
	     MAX (MAX (strlen (ORIGIN), strlen (_dl_platform)),		      \
		  strlen (DL_DST_LIB))					      \
	   minus 4 (which is the length of "$LIB").			      \
									      \
	   First get the origin string if it is not available yet.	      \
	   This can only happen for the map of the executable or, when	      \
	   auditing, in ld.so.  */					      \
	DL_DST_REQ_STATIC (l)						      \
	if ((l)->l_origin == NULL)					      \
	  {								      \
	    assert ((l)->l_name[0] == '\0' || IS_RTLD (l));		      \
	    (l)->l_origin = _dl_get_origin ();				      \
	    dst_len = ((l)->l_origin && (l)->l_origin != (char *) -1	      \
			  ? strlen ((l)->l_origin) : 0);		      \
	  }								      \
	else								      \
	  dst_len = (l)->l_origin == (char *) -1			      \
	    ? 0 : strlen ((l)->l_origin);				      \
	dst_len = MAX (MAX (dst_len, GLRO(dl_platformlen)),		      \
		       strlen (DL_DST_LIB));				      \
	if (dst_len > 4)						      \
	  __len += __cnt * (dst_len - 4);				      \
      }									      \
									      \
    __len; })

#ifdef SHARED
# define DL_DST_REQ_STATIC(l) /* nothing */
#else
# define DL_DST_REQ_STATIC(l) \
  if ((l) == NULL)							      \
    {									      \
      const char *origin = _dl_get_origin ();				      \
      dst_len = (origin && origin != (char *) -1 ? strlen (origin) : 0);      \
    }									      \
  else
#endif
