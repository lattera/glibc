/* Handling of dynamic sring tokens.
   Copyright (C) 1999, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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
#ifndef IS_IN_rtld
# define _dl_dst_count GLRO(dl_dst_count)
#endif


/* Guess from the number of DSTs the length of the result string.  */
#define DL_DST_REQUIRED(l, name, len, cnt) \
  ({									      \
    size_t __len = (len);						      \
    size_t __cnt = (cnt);						      \
									      \
    if (__cnt > 0)							      \
      {									      \
	size_t origin_len;						      \
	/* Now we make a guess how many extra characters on top of the	      \
	   length of S we need to represent the result.  We know that	      \
	   we have CNT replacements.  Each at most can use		      \
	     MAX (strlen (ORIGIN), strlen (_dl_platform))		      \
	   minus 7 (which is the length of "$ORIGIN").			      \
									      \
	   First get the origin string if it is not available yet.	      \
	   This can only happen for the map of the executable.  */	      \
	if ((l)->l_origin == NULL)					      \
	  {								      \
	    assert ((l)->l_name[0] == '\0');				      \
	    (l)->l_origin = _dl_get_origin ();				      \
	    origin_len = ((l)->l_origin && (l)->l_origin != (char *) -1	      \
			  ? strlen ((l)->l_origin) : 0);		      \
	  }								      \
	else								      \
	  origin_len = (l)->l_origin == (char *) -1			      \
	    ? 0 : strlen ((l)->l_origin);				      \
									      \
	__len += __cnt * (MAX (origin_len, GLRO(dl_platformlen)) - 7);	      \
      }									      \
									      \
    __len; })

#ifndef IS_IN_rtld
# define _dl_get_origin GLRO(dl_get_origin)
# define _dl_dst_substitute GLRO(dl_dst_substitute)
#endif
