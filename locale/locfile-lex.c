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
#include <langinfo.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "localedef.h"
#include "token.h"


/* Include the hashing table for the keywords.  */
const struct locale_keyword* in_word_set (register const char *str,
                                          register int len);
#include "keyword.h"


/* Contains the status of reading the locale definition file.  */
struct locfile_data locfile_data;

/* This is a flag used while collation input.  This is the only place
   where element names beside the ones defined in the character map are
   allowed.  There we must not give error messages.  */
int reject_new_char = 1;

/* Prototypes for local functions.  */
static int get_char (void);


#define LD locfile_data

/* Opens the locale definition file and initializes the status data structure
   for following calls of `locfile_lex'.  */
void
locfile_open (const char *fname)
{
  if (fname == NULL)
    /* We read from stdin.  */
    LD.filename = "<stdin>";
  else
    {
      if (freopen (fname, "r", stdin) == NULL)
	error (4, 0, gettext ("input file `%s' not found"), fname);
      LD.filename = fname;
    }

  /* Set default values.  */
  LD.escape_char = '\\';
  LD.comment_char = '#';

  LD.bufsize = sysconf (_SC_LINE_MAX);
  LD.buf = (char *) xmalloc (LD.bufsize);
  LD.strbuf = (char *) xmalloc (LD.bufsize);

  LD.buf_ptr = LD.returned_tokens = LD.line_no = 0;

  /* Now sign that we want immediately read a line.  */
  LD.continue_line = 1;
  LD.buf[LD.buf_ptr] = '\0';
}


int
xlocfile_lex (char **token, int *token_len)
{
  int retval = locfile_lex (token, token_len);

  if (retval == 0)
    /* I.e. end of file.  */
    error (4, 0, gettext ("%s: unexpected end of file in locale defintion "
			  "file"), locfile_data.filename);

  return retval;
}

int
locfile_lex (char **token, int *token_len)
{
  int start_again;
  int retval = 0;

  do
    {
      int start_ptr;

      start_again = 0;

      /* Read the next line.  Skip over empty lines and comments.  */
      if ((LD.buf[LD.buf_ptr] == '\0' && LD.continue_line != 0)
	  || LD.buf_ptr >= LD.bufsize
	  || (posix_conformance == 0 && LD.buf[LD.buf_ptr] == LD.comment_char))
	do
	  {
	    size_t linelen;

	    LD.buf_ptr = 0;

	    if (fgets (LD.buf, LD.bufsize, stdin) == NULL)
	      {
		/* This makes subsequent calls also return EOF.  */
		LD.buf[0] = '\0';
		return 0;
	      }

	    /* Increment line number counter.  */
	    ++LD.line_no;

	    /* We now have to look whether this line is continued and
	       whether it at all fits into our buffer.  */
	    linelen = strlen (LD.buf);

	    if (linelen == LD.bufsize - 1)
	      /* The did not fit into the buffer.  */
	      error (2, 0, gettext ("%s:%Zd: line too long;  use "
				    "`getconf LINE_MAX' to get the maximum "
				    "line length"), LD.filename, LD.line_no);

	    /* Remove '\n' at end of line.  */
	    if (LD.buf[linelen - 1] == '\n')
	      LD.buf[--linelen] = '\0';

	    if (linelen > 0 && LD.buf[linelen - 1] == LD.escape_char)
	      {
		LD.buf[--linelen] = '\0';
		LD.continue_line = 1;
	      }
	    else
	      LD.continue_line = 0;

	    while (isspace (LD.buf[LD.buf_ptr]))
	      ++LD.buf_ptr;

	    /* We are not so restrictive and allow white spaces before
	       a comment.  */
	    if (posix_conformance == 0
		&& LD.buf[LD.buf_ptr] == LD.comment_char
		&& LD.buf_ptr != 0)
	      error (0, 0, gettext ("%s:%Zd: comment does not start in "
				    "column 1"), LD.filename, LD.line_no);
	  }
	while (LD.buf[LD.buf_ptr] == '\0'
	       || LD.buf[LD.buf_ptr] == LD.comment_char);


      /* Get information for return values.  */
      *token = LD.buf + LD.buf_ptr;
      start_ptr = LD.buf_ptr;

      /* If no further character is in the line this is the end of a logical
	 line.  This information is needed in the parser.  */
      if (LD.buf[LD.buf_ptr] == '\0')
	{
	  LD.buf_ptr = LD.bufsize;
	  retval = TOK_ENDOFLINE;
	}
      else if (isalpha (LD.buf[LD.buf_ptr]))
	/* The token is an identifier.  The POSIX standard does not say
	   what characters might be contained but offical POSIX locale
	   definition files contain beside alnum characters '_', '-' and
	   '+'.  */
	{
	  const struct locale_keyword *kw;

	  do
	    ++LD.buf_ptr;
	  while (isalnum (LD.buf[LD.buf_ptr]) || LD.buf[LD.buf_ptr] == '_'
		 || LD.buf[LD.buf_ptr] == '-' || LD.buf[LD.buf_ptr] == '+');

	  /* Look in table of keywords.  */
	  kw = in_word_set (*token, LD.buf_ptr - start_ptr);
	  if (kw == NULL)
	    retval = TOK_IDENT;
	  else
	    {
	      if (kw->token_id == TOK_ESCAPE_CHAR
		  || kw->token_id == TOK_COMMENT_CHAR)
		/* `escape_char' and `comment_char' are keywords for the
		   lexer.  Do not give them to the parser.  */
		{
		  start_again = 1;

		  if (!isspace (LD.buf[LD.buf_ptr])
		      || (posix_conformance && LD.returned_tokens > 0))
		    error (0, 0, gettext ("%s:%Zd: syntax error in locale "
					  "definition file"),
			   LD.filename, LD.line_no);

		  do
		    ++LD.buf_ptr;
		  while (isspace (LD.buf[LD.buf_ptr]));

		  kw->token_id == TOK_ESCAPE_CHAR
		    ? LD.escape_char
		    : LD.comment_char = LD.buf[LD.buf_ptr++];

		  ignore_to_eol (0, posix_conformance);
		}
	      else
		/* It is one of the normal keywords.  */
		retval = kw->token_id;
	    }

	  *token_len = LD.buf_ptr - start_ptr;
	}
      else if (LD.buf[LD.buf_ptr] == '"')
	/* Read a string.  All symbolic character descriptions are expanded.
	   This has to be done in a local buffer because a simple symbolic
	   character like <A> may expand to upto 6 bytes.  */
	{
	  char *last = LD.strbuf;

	  ++LD.buf_ptr;
	  while (LD.buf[LD.buf_ptr] != '"')
	    {
	      int pre = LD.buf_ptr;
	      int char_val = get_char (); /* token, token_len); */

	      if (char_val == 0)
		{
		  error (4, 0, gettext ("%s:%Zd: unterminated string at end "
					"of line"), LD.filename, LD.line_no);
		  /* NOTREACHED */
		}

	      if (char_val > 0)
		/* Unknown characters are simply not stored.  */
		last += char_to_utf (last, char_val);
	      else
		{
		  char tmp[LD.buf_ptr - pre + 1];
		  memcpy (tmp, &LD.buf[pre], LD.buf_ptr - pre);
		  tmp[LD.buf_ptr - pre] = '\0';
		  error (0, 0, gettext ("%s:%Zd: character `%s' not defined"),
			 LD.filename, LD.line_no, tmp);
		}
	    }
	  if (LD.buf[LD.buf_ptr] != '\0')
	    ++LD.buf_ptr;

	  *last = '\0';
	  *token = LD.strbuf;
	  *token_len = last  - LD.strbuf;
	  retval = TOK_STRING;
	}
      else if (LD.buf[LD.buf_ptr] == '.' && LD.buf[LD.buf_ptr + 1] == '.'
	       && LD.buf[LD.buf_ptr + 2] == '.')
	{
	  LD.buf_ptr += 3;
	  retval = TOK_ELLIPSIS;
	}
      else if (LD.buf[LD.buf_ptr] == LD.escape_char)
	{
	  char *endp;

	  ++LD.buf_ptr;
	  switch (LD.buf[LD.buf_ptr])
	    {
	    case 'x':
	      if (isdigit (LD.buf[++LD.buf_ptr]))
		{
		  retval = strtol (&LD.buf[LD.buf_ptr], &endp, 16);
		  if (endp - (LD.buf + LD.buf_ptr) < 2 || retval > 255)
		    retval = 'x';
		  else
		    LD.buf_ptr = endp - LD.buf;
		}
	      else
		retval = 'x';
	      break;
	    case 'd':
	      if (isdigit (LD.buf[++LD.buf_ptr]))
		{
		  retval = strtol (&LD.buf[LD.buf_ptr], &endp, 10);
		  if (endp - (LD.buf + LD.buf_ptr) < 2 || retval > 255)
		    retval = 'd';
		  else
		    LD.buf_ptr = endp - LD.buf;
		}
	      else
		retval = 'd';
	      break;
	    case '0'...'9':
	      retval = strtol (&LD.buf[LD.buf_ptr], &endp, 8);
	      if (endp - (LD.buf + LD.buf_ptr) < 2 || retval > 255)
		retval = LD.buf[LD.buf_ptr++];
	      else
		LD.buf_ptr = endp - LD.buf;
	      break;
	    case 'a':
	      retval = '\a';
	      ++LD.buf_ptr;
	      break;
	    case 'b':
	      retval = '\b';
	      ++LD.buf_ptr;
	      break;
	    case 'f':
	      retval = '\f';
	      ++LD.buf_ptr;
	      break;
	    case 'n':
	      retval = '\n';
	      ++LD.buf_ptr;
	      break;
	    case 'r':
	      retval = '\r';
	      ++LD.buf_ptr;
	      break;
	    case 't':
	      retval = '\t';
	      ++LD.buf_ptr;
	      break;
	    case 'v':
	      retval = '\v';
	      ++LD.buf_ptr;
	      break;
	    default:
	      retval = LD.buf[LD.buf_ptr++];
	      break;
 	    }
	}
      else if (isdigit (LD.buf[LD.buf_ptr]))
	{
	  char *endp;

	  *token_len = strtol (&LD.buf[LD.buf_ptr], &endp, 10);
	  LD.buf_ptr = endp - LD.buf;
	  retval = TOK_NUMBER;
	}
      else if (LD.buf[LD.buf_ptr] == '-' && LD.buf[LD.buf_ptr + 1] == '1')
	{
	  LD.buf_ptr += 2;
	  retval = TOK_MINUS1;
	}
      else
	{
	  int ch = get_char (); /* token, token_len); */
	  if (ch != -1)
	    {
	      *token_len = ch;
	      retval = TOK_CHAR;
	    }
	  else
	    retval = TOK_ILL_CHAR;
	}

      /* Ignore white space.  */
      while (isspace (LD.buf[LD.buf_ptr]))
	++LD.buf_ptr;
    }
  while (start_again != 0);

  ++LD.returned_tokens;
  return retval;
}


/* Code a character with UTF-8 if the character map has multi-byte
   characters.  */
int
char_to_utf (char *buf, int char_val)
{
  if (charmap_data.mb_cur_max == 1)
    {
      *buf++ = char_val;
      return 1;
    }
  else
    {
/* The number of bits coded in each character.  */
#define CBPC 6
      static struct coding_tab
        {
          int mask;
          int val;
        }
      tab[] =
        {
          { 0x7f,       0x00 },
          { 0x7ff,      0xc0 },
          { 0xffff,     0xe0 },
          { 0x1fffff,   0xf0 },
          { 0x3ffffff,  0xf8 },
          { 0x7fffffff, 0xfc },
          { 0, }
        };
      struct coding_tab *t;
      int c;
      int cnt = 1;

      for (t = tab; char_val > t->mask; ++t, ++cnt)
	;

      c = cnt;

      buf += cnt;
      while (c > 1)
	{
	  *--buf = 0x80 | (char_val & ((1 << CBPC) - 1));
	  char_val >>= CBPC;
	  --c;
	}

      *--buf = t->val | char_val;

      return cnt;
    }
}


/* Ignore rest of line upto ENDOFLINE token, starting with given token.
   If WARN_FLAG is set warn about any token but ENDOFLINE.  */
void
ignore_to_eol (int token, int warn_flag)
{
  if (token == TOK_ENDOFLINE)
    return;

  if (LD.buf[LD.buf_ptr] != '\0' && warn_flag)
    error (0, 0, gettext ("%s:%Zd: trailing garbage at end of line"),
	   locfile_data.filename, locfile_data.line_no);

  while (LD.continue_line)
    {
      LD.continue_line = 0;

      /* Increment line number counter.  */
      ++LD.line_no;

      if (fgets (LD.buf, LD.bufsize, stdin) != NULL)
	{
	  /* We now have to look whether this line is continued and
	     whether it at all fits into our buffer.  */
	  int linelen = strlen (LD.buf);

	  if (linelen == LD.bufsize - 1)
	    /* The did not fit into the buffer.  */
	    error (2, 0, gettext ("%s:%Zd: line too long;  use `getconf "
				  "LINE_MAX' to get the current maximum "
				  "line length"), LD.filename, LD.line_no);

	  /* Remove '\n' at end of line.  */
	  if (LD.buf[linelen - 1] == '\n')
	    --linelen;

	  if (LD.buf[linelen - 1] == LD.escape_char)
	    LD.continue_line = 1;
	}
    }
 
  /* This causes to begin the next line.  */
  LD.buf_ptr = LD.bufsize;
}


/* Return the value of the character at the beginning of the input buffer.
   Symbolic character constants are expanded.  */
static int
get_char (void)
{
  if (LD.buf[LD.buf_ptr] == '<')
    /* This is a symbolic character name.  */
    {
      int char_val;
      char *startp = LD.buf + (++LD.buf_ptr);
      char *endp = startp;

      while (LD.buf[LD.buf_ptr] != '>' && isprint (LD.buf[LD.buf_ptr]))
	{
	  if (LD.buf[LD.buf_ptr] == '\0'
	      || (LD.buf[LD.buf_ptr] == LD.escape_char
		  && LD.buf[++LD.buf_ptr] == '\0'))
	    break;

	  *endp++ = LD.buf[LD.buf_ptr++];
	}

      if (LD.buf[LD.buf_ptr] != '>' && LD.buf[LD.buf_ptr] == '\0')
	{
	  error (0, 0, gettext ("%s:%Zd: end of line in character symbol"),
		 LD.filename, LD.line_no);

	  if (startp == endp)
	    return -1;
	}
      else
	++LD.buf_ptr;

      char_val = find_char (startp, endp - startp);
      if (char_val == -1 && verbose != 0 && reject_new_char != 0)
	{
	  /* Locale defintions are often given very general.  Missing
	     characters are only reported when explicitely requested.  */
	  char tmp[endp - startp + 3];

	  tmp[0] = '<';
	  memcpy (tmp + 1, startp, endp - startp);
	  tmp[endp - startp + 1] = '>';
	  tmp[endp - startp + 2] = '\0';

	  error (0, 0, gettext ("%s:%Zd: character `%s' not defined"),
		 LD.filename, LD.line_no, tmp);
	}
      
      return char_val;
    }
  else
    return (int) LD.buf[LD.buf_ptr++];
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
