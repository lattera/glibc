/* Copyright (C) 1995 Free Software Foundation, Inc.

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
#include <libintl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "localedef.h"
#include "hash.h"

/* Data structure for representing charmap database.  */
struct charmap charmap_data;

/* Line number in charmap file.  */
static unsigned int line_no;

/* Prototypes for local functions.  */
static void read_prolog (FILE *infile);
static unsigned long read_body (FILE *infile);


/* Read complete table of symbolic names for character set from file.  If
   this file does not exist or is not readable a default file is tried.
   If this also is not readable no character map is defined.  */
void
charmap_read (const char *filename)
{
  unsigned long max_char;
  long path_max = pathconf (".", _PC_PATH_MAX);
  char buf[path_max];
  FILE *infile = NULL;

  /* Initialize charmap data.  */
  charmap_data.codeset_name = NULL;
  charmap_data.mb_cur_max = -1;
  charmap_data.mb_cur_min = -1;
  charmap_data.escape_char = '\\';
  charmap_data.comment_char = '#';

  if (filename != NULL)
    {
      strcpy (buf, filename);
      infile = fopen (filename, "r");
      if (infile == NULL && filename[0] != '/')
        {
          snprintf (buf, path_max, "%s/%s", CHARMAP_PATH, filename);
          infile = fopen (buf, "r");
        }
    }
  if (infile == NULL)
    {
      if (filename != NULL)
	error (0, errno, gettext ("input file `%s' not found"), filename);

      snprintf (buf, path_max, "%s/%s", CHARMAP_PATH, DEFAULT_CHARMAP);
      infile = fopen (buf, "r");

      if (infile == NULL)
	error (4, errno, gettext ("input file `%s' not found"), filename); 
    }

  charmap_data.filename = buf;
  init_hash (&charmap_data.table, 500);
  line_no = 0;

  /* Read the prolog of the charmap file.  */
  read_prolog (infile);

  /* Last works on the charmap tables global data.  */
  if (charmap_data.mb_cur_max == -1)
    charmap_data.mb_cur_max = 1;
  if (charmap_data.mb_cur_min == -1)
    charmap_data.mb_cur_min = charmap_data.mb_cur_max;

  if ((size_t) charmap_data.mb_cur_max > sizeof (long))
    {
      error (2, 0, gettext ("program limitation: for now only upto %Zu "
			    "bytes per character are allowed"), sizeof (long));
    }

  /* Now process all entries.  */
  max_char = read_body (infile);

  /* We don't need the file anymore.  */
  fclose (infile);


  /* Determine the optimal table size when using the simple modulo hashing
     function.  */
  if (max_char >= 256)
    {
      int size;
      /* Current best values, initialized to some never reached high value.  */
      int best_count = 10000;
      int best_size = 10000;
      int best_product = best_count * best_size;

      /* Give warning.  */
      error (-1, 0, gettext ("computing character table size: this may take "
			     "a while"));

      for (size = 256; size <= best_product; ++size)
	{
	  /* Array with slot counters.  */
	  int cnt[size];
	  /* Current character.  */
	  int ch;
	  /* Maximal number of characters in any slot.  */
	  int maxcnt = 0;
	  /* Product of current size and maximal count.  */
	  int product = 0;
	  /* Iteration pointer through hashing table.  */
	  char *ptr = NULL;

	  /* Initializes counters to zero.  */
	  memset(cnt, 0, size * sizeof (int));

	  /* Iterate through whole hashing table.  */
	  while (product < best_product
		 && iterate_table (&charmap_data.table, (void **) &ptr,
				   (void **) &ch))
	    {
	      /* Increment slot counter.  */
	      ++cnt[ch % size];
	      /* Test for current maximum.  */
	      if (cnt[ch % size] > maxcnt)
		{
		  maxcnt = cnt[ch % size];
		  product = maxcnt * size;
		}
	    }

	  if (product < best_product)
	    {
	      best_count = maxcnt;
	      best_size = size;
	      best_product = best_count * best_size;
	    }
	}

      charmap_data.hash_size = best_size;
      charmap_data.hash_layers = best_count;
    }
  else
    {
      charmap_data.hash_size = 256;
      charmap_data.hash_layers = 1;
    }
}


#define SYNTAX_ERROR							     \
  do { error (0, 0, gettext ("%s:%u: syntax error in charmap file"),	     \
	      charmap_data.filename, line_no);                               \
       goto end_of_loop; } while (0)

/* Read the prolog of the charmap file until the line containing `CHARMAP'.
   All possible entries are processed.  */
static void
read_prolog (FILE *infile)
{
  size_t bufsize = sysconf (_SC_LINE_MAX);
  char buf[bufsize];

  while (1)
    {
      char *cp = buf;
      char len;

      /* Read the next line.  */
      fgets (buf, bufsize, infile);
      len = strlen (buf);

      /* On EOF simply return.  */
      if (len == 0 || buf[len - 1] != '\n')
	error (4, 0, gettext ("%s: unexpected end of file in charmap"),
	       charmap_data.filename);

      /* This is the next line.  */
      ++line_no;

      /* Comments and empty lines are ignored.  */
      if (len == 1 || buf[0] == charmap_data.comment_char)
	continue;

      buf[len - 1] = '\0';

      /* Throw away leading white spaces.  This is not defined in POSIX.2
	 so don't do it if conformance is requested.  */
      if (!posix_conformance)
	while (isspace (*cp))
	  ++cp;

      /* If `CHARMAP' is read the prolog is over.  */
      if (strncmp (cp, "CHARMAP", 7) == 0
	  && (!posix_conformance || cp[7] == '\0'))
	return;

      /* Now it can be only one of special symbols defining the charmap
	 parameters.  All are beginning with '<'.  */
      if (*cp != '<')
	SYNTAX_ERROR;

      ++cp;
      if (strncmp (cp, "code_set_name>", 14) == 0)
	{
	  char *startp;

#define cp_to_arg(no,pred)						      \
	  cp += no;							      \
	  while (isspace (*cp))						      \
	    ++cp;							      \
	  if (*cp == '\0' || !pred (*cp))				      \
            SYNTAX_ERROR;

	  cp_to_arg (14,isgraph)

	  if (charmap_data.codeset_name != NULL)
	    {
	      error (0, 0, gettext ("%s:%u: duplicate code set name "
				    "specification"),
		     charmap_data.filename, line_no);
	      free (charmap_data.codeset_name);
	    }

	  startp = cp;
	  while (*cp != '\0' && isgraph (*cp) && !isspace (*cp))
	    ++cp;

	  charmap_data.codeset_name = (char *) xmalloc (cp - startp + 1);
	  strncpy (startp, startp, cp - startp);
	}
      else if (strncmp (cp, "mb_cur_max>", 11) == 0)
	{
          int new_val;
	  cp_to_arg (11,isdigit)

	  if (charmap_data.mb_cur_max != -1)
	    error (0, 0,
		   gettext ("%s:%u: duplicate definition of mb_cur_max"),
		   charmap_data.filename, line_no);

	  new_val = (int) strtol (cp, &cp, posix_conformance ? 10 : 0);
	  if (new_val < 1)
	    error (0, 0, gettext ("%s:%u: illegal value for mb_cur_max: %d"),
		   charmap_data.filename, line_no, new_val);
	  else
	    charmap_data.mb_cur_max = new_val;
	}
      else if (strncmp (cp, "mb_cur_min>", 11) == 0)
	{
          int new_val;
	  cp_to_arg (11,isdigit)

	  if (charmap_data.mb_cur_max != -1)
	    error (0, 0,
		   gettext ("%s:%u: duplicate definition of mb_cur_min"),
		   charmap_data.filename, line_no);

	  new_val = (int) strtol (cp, &cp, posix_conformance ? 10 : 0);
	  if (new_val < 1)
	    error (0, 0, gettext ("%s:%u: illegal value for mb_cur_min: %d"),
		   charmap_data.filename, line_no, new_val);
	  else
	    charmap_data.mb_cur_min = new_val;
	}
      else if (strncmp (cp, "escape_char>", 12) == 0)
	{
	  cp_to_arg (12, isgraph)
	  charmap_data.escape_char = *cp;
	}
      else if (strncmp (cp, "comment_char>", 13) == 0)
	{
	  cp_to_arg (13, isgraph)
	  charmap_data.comment_char = *cp;
	}
      else
	SYNTAX_ERROR;
      end_of_loop:
    }
}
#undef cp_to_arg


static unsigned long
read_body (FILE *infile)
{
  unsigned long max_char = 0;
  size_t bufsize = sysconf (_SC_LINE_MAX);
  char buf[bufsize];
  char name_str[bufsize / 2];
  char code_str[bufsize / 2];

  while (1)
    {
      char *cp = buf;
      size_t len;

      /* Read the next line.  */
      fgets (buf, bufsize, infile);
      len = strlen (buf);

      /* On EOF simply return.  */
      if (len == 0)
	error (0, 0, gettext ("%s: `END CHARMAP' is missing"),
	       charmap_data.filename);

      /* This is the next line.  */
      ++line_no;

      if (len == bufsize - 1)
	{
	  error (0, 0, gettext ("%s:%u: line too long;  use `getconf "
				"LINE_MAX' to get the current maximum line"
				"length"), charmap_data.filename, line_no);
	  do
	    {
	      fgets (buf, bufsize, infile);
	      len = strlen (buf);
	    }
	  while (len == bufsize - 1);
	  continue;
	}

      /* Comments and empty lines are ignored.  */
      if (len == 1 || buf[0] == charmap_data.comment_char)
	continue;

      buf[len - 1] = '\0';

      /* Throw away leading white spaces.  This is not defined in POSIX.2
	 so don't do it if conformance is requested.  */
      if (!posix_conformance)
	while (isspace (*cp))
	  ++cp;

      if (*cp == '<')
	{
	  char *end1p, *end2p, *start2p;
	  size_t cnt = 0;
	  unsigned long char_value = 0;

	  if (sscanf (cp + 1, "%s %s", name_str, code_str) != 2)
	    SYNTAX_ERROR;

	  end1p = cp = name_str;
	  while (*cp != '\0' && *cp != '>')
	    {
	      if (*cp == charmap_data.escape_char)
		if (*++cp == '\0')
		  SYNTAX_ERROR;
	      *end1p++ = *cp++;
	    }
	  if (*cp == '\0')
	    /* No final '>'.  Make error condition.  */
	    end1p = name_str;
	  else
	    ++cp;

	  *end1p = '\0';
	  
	  if (*cp == '.' && *++cp == '.' && *++cp == '.' && *++cp == '<')
	    {
	      /* This might be the alternate form.  */
	      start2p = end2p = ++cp;
	      while (*cp != '\0' && *cp != '>')
		{
		  if (*cp == charmap_data.escape_char)
		    if (*++cp == '\0')
		      SYNTAX_ERROR;
		  *end2p = *cp++;
		}
	      if (*cp == '\0')
		/* NO final '>'.  Make error condition.  */
		end2p = start2p;
	      else
		++cp;
	    }
	  else
	    start2p = end2p = NULL;


	  if (end1p == name_str || (start2p != NULL && start2p != end2p)
	      || *cp != '\0'
	      || *code_str != charmap_data.escape_char)
	    SYNTAX_ERROR;

	  cp = code_str;
	  do
	    {
	      char *begin;
	      long val;

	      switch (*++cp)
		{
		case 'd':
		  val = strtol ((begin = cp + 1), &cp, 10);
		  break;
		case 'x':
		  val = strtol ((begin = cp + 1), &cp, 16);
		  break;
		default:
		  val = strtol ((begin = cp), &cp, 8);
		  break;
		}
	      if (begin == cp)
		SYNTAX_ERROR;

	      if (posix_conformance && cp - begin < 2)
		error (0, 0, gettext ("%s:%u: byte constant has less than "
				      "two digits"),
		       charmap_data.filename, line_no);

	      if (val < 0 || val > 255)
		{
		  error (0, 0, gettext ("%s:%u: character encoding must be "
					"given in 8-bit bytes"),
			 charmap_data.filename, line_no);
		  goto end_of_loop;
		}

	      if (cnt < (size_t) charmap_data.mb_cur_max)
		{
		  if (cnt < sizeof (long))  /* FIXME */
		    char_value = (char_value << 8) | val;
		}
	      else
		{
		  error (0, 0, gettext ("%s:%u: number of bytes in character "
					"definition exceeds `mb_cur_max'"),
			 charmap_data.filename, line_no);
		  break;
		}
	      ++cnt;
	    }
	  while (*cp == charmap_data.escape_char);

	  /* Ignore the rest of the line (comment).  */
	  if (end2p == NULL)
	    {
	      if (insert_entry (&charmap_data.table, name_str,
				end1p - name_str, (void *) char_value))
		error (0, 0, gettext ("%s:%u: duplicate entry"),
		       charmap_data.filename, line_no);

	      max_char = MAX (max_char, char_value);
	    }
	  else
	    {
	      char *en1, *en2, *start1p;
	      long n1, n2, n;

	      start1p = name_str;

 	      while (*start1p == *start2p && !isdigit (*start1p)
		     && start1p < end1p)
		  ++start1p, ++start2p;

	      n1 = strtol (start1p, &en1, 10);
	      n2 = strtol (start2p, &en2, 10);

	      if (en1 - start1p != en2 - start2p || en1 != end1p
		  || en2 != end2p)
		SYNTAX_ERROR;

	      if (n1 > n2)
		error (0, 0, gettext ("%s:%u: starting character is bigger "
				      "than last"),
		       charmap_data.filename, line_no);

	      n = n1;
	      while (n <= n2)
		{
		  snprintf(start1p, en1 - start1p, "%0*d", en1 - start1p, n);

		  if (insert_entry (&charmap_data.table, name_str,
				    en1 - name_str,
				    (void *) (char_value + n - n1)))
		    error (0, 0, gettext ("%s:%u: duplicate entry"),
			   charmap_data.filename, line_no);

		  max_char = MAX (max_char, char_value + n - n1);
		  ++n;
		}
	    }
	}
      else
	{
	  if (strncmp (cp, "END CHARMAP", 11) == 0)
	    return max_char;
	  
	  SYNTAX_ERROR;
	}
      end_of_loop:
    }

  return max_char;
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
