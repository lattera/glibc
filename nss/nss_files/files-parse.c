/* Common code for file-based database parsers in nss_files module.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


#define CONCAT(a,b) CONCAT1(a,b)
#define CONCAT1(a,b) a##b

#ifndef STRUCTURE
#define STRUCTURE ENTNAME
#endif


struct parser_data
  {
    struct CONCAT(ENTNAME,_data) entdata;
    char linebuffer[0];
  };

#define LINE_PARSER(BODY)						      \
static inline int							      \
parse_line (char *line, struct STRUCTURE *result,			      \
	    struct parser_data *data, int datalen)			      \
{									      \
  struct CONCAT(ENTNAME,_data) *const entdata __attribute__ ((unused))	      \
    = &data->entdata;		      					      \
  BODY;									      \
  TRAILING_LIST_PARSER;							      \
  return 1;								      \
}


/* Comments can come mid-line; trim the line at the first # seen.  */
#define MIDLINE_COMMENTS						      \
  {									      \
    char *p = strchr (line, '#');					      \
    if (p)								      \
      *p = '\0';							      \
  }

#define STRING_FIELD(variable, terminator_p, swallow)			      \
  {									      \
    variable = line;							      \
    while (!terminator_p (*line))					      \
      if (*++line == '\0')						      \
	return 0;							      \
    *line = '\0';							      \
    do									      \
      ++line;								      \
    while (swallow && terminator_p (*line));				      \
  }

#define INT_FIELD(variable, terminator_p, swallow, base, convert)	      \
  {									      \
    char *endp;								      \
    variable = convert (strtol (line, &endp, base));			      \
    if (endp == line)							      \
      return 0;								      \
    else if (terminator_p (*endp))					      \
      do								      \
	++endp;								      \
      while (swallow && terminator_p (*endp));				      \
    else if (*endp != '\0')						      \
      return 0;								      \
    line = endp;							      \
  }

#define ISCOLON(c) ((c) == ':')


#ifndef TRAILING_LIST_MEMBER
#define TRAILING_LIST_PARSER /* Nothing to do.  */
#else

#define TRAILING_LIST_PARSER						      \
{									      \
  char **list = parse_list (line, data, datalen);			      \
  if (list)								      \
    result->TRAILING_LIST_MEMBER = list;				      \
  else 									      \
    return 0;								      \
}

static inline char **
parse_list (char *line, struct parser_data *data, int datalen)
{
  char *eol, **list, **p;

  /* Find the end of the line buffer.  */
  eol = strchr (line, '\0');
  /* Adjust the pointer so it is aligned for storing pointers.  */
  eol += (eol - (char *) 0) % __alignof__ (char *);
  /* We will start the storage here for the vector of pointers.  */
  list = (char **) eol;

  p = list;
  while (1)
    {
      char *elt;

      if ((char *) &p[1] - (char *) data > datalen)
	{
	  /* We cannot fit another pointer in the buffer.  */
	  errno = ERANGE;
	  return NULL;
	}
      if (*line == '\0')
	break;

      elt = line;
      while (1)
	{
	  if (TRAILING_LIST_SEPARATOR_P (*line))
	    {
	      *p++ = elt;
	      *line = '\0';
	      do
		++line;
	      while (TRAILING_LIST_SEPARATOR_P (*line));
	      elt = line;
	    }
	  else if (*line == '\0' || *line == '\n')
	    {
	      /* End of the line.  */
	      if (line > elt)
		/* Last element.  */
		*p++ = elt;
	      *line = '\0';
	      break;
	    }
	  else
	    ++line;
	}
    }
  *p = NULL;

  return list;
}

#define LOOKUP_NAME(nameelt, aliaselt)					      \
{									      \
  char **ap;								      \
  if (! strcmp (name, result->nameelt))					      \
    break;								      \
  for (ap = result->aliaselt; *ap; ++ap)				      \
    if (! strcmp (name, *ap))						      \
      break;								      \
  if (*ap)								      \
    break;								      \
}

#endif
