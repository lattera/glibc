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

#include <langinfo.h>
#include <libintl.h>

#include "localedef.h"
#include "token.h"


/* defined in locfile-lex.c: flag to indicate that unknown element names
   are allowed.  */
extern int reject_new_char;


#define SYNTAX_ERROR                                                         \
    error (0, 0, gettext ("%s:%Zd: syntax error in locale definition file"), \
	   locfile_data.filename, locfile_data.line_no);

void
collate_input (int token)
{
  int read_order_start = 0;

  while (1)
    {
      char *ptr;
      int len;

      if (token == TOK_END)
	/* This is the end of the category.  */
	{
	  token = xlocfile_lex (&ptr, &len);

	  if (token != _NL_NUM_LC_COLLATE)
	    {
	      error (0, 0, gettext ("%s:%Zd: category `%s' does not end "
				    "with `END %s'"), locfile_data.filename,
			 locfile_data.line_no, "LC_COLLATE", "LC_COLLATE");
	      ignore_to_eol (0, 0);
	    }
	  else
	    ignore_to_eol (0, 1);

	  /* Start next category.  */
	  break;
	}

#if 0
      /* Process line.  */
      if (read_order_start == 0)
	/* We're still in the preambel.  */
	{
	  switch (token)
	    {
	    case TOK_COLLATING_ELEMENT:
	      reject_new_char = 0;
	      token = xlocfile_lex (&ptr, &len);
	      reject_new_char = 1;
	      if (token == TOK_CHAR)
		{
		  error (0, 0, gettext ("%s:%Zd: symbolic name must not be "
					"duplicate name in charmap"),
			 locfile_data.filename, locfile_data.line_no);
		  ignore_to_eol (0, 0);
		  break;
		}
	      else if (token != TOK_ILL_CHAR)
		{
		  SYNTAX_ERROR;
		  ignore_to_eol (0, 0);
		  break;
		}
	      else
		{
		  char elem_name[len + 1];
		  memcpy (elem_name, ptr, len);
		  elem_name[len] = '\0';

		  /* Test whether defined in symbol table.  */

		  token = xlocfile_lex (&ptr, &len);
		  if (token != TOK_FROM)
		    {
		      SYNTAX_ERROR;
		      ignore_to_eol (0, 0);
		      break;
		    }

		  token  = xlocfile_lex (&ptr, &len);
                  if (token != TOK_STRING)
		    {
		      SYNTAX_ERROR;
		      ignore_to_eol (0, 0);
		      break;
		    }

		  /* Insert collating element into table.  */

		  /* Rest of the line should be empty.  */
		  ignore_to_eol (0, 1);
		}
	      break;
	    case TOK_COLLATING_SYMBOL:
	      reject_new_char = 0;
	      token = xlocfile_lex (&ptr, &len);
	      reject_new_char = 1;
	      if (token == TOK_CHAR)
		{
		  error (0, 0, gettext ("%s:%Zd: symbolic name must not "
					"duplicate name in charmap"),
			 locfile_data.filename, locfile_data.line_no);
		  ignore_to_eol (0, 0);
		  break;
		}
	      else if (token != TOK_ILL_CHAR)
		{
		  SYNTAX_ERROR;
		  ignore_to_eol (0, 0);
		  break;
		}
	      else
		{
		  /* Test whether defined in element table.  */

		  /* Insert collating symbol into table.  */

		  ignore_to_eol (0, 1);
		}
	    case TOK_ORDER_START:
	      nsort_rules = 0;

	      do
		{
		  token = xlocfile_lex (&ptr, &len);

		  if (nsort_rules == 0 && token == ENDOFLINE)
		    break;

		  if (token != TOK_BACKWARD && token != TOK_FORWARD
		      && token != TOK_POSITION)
		    {
		      SYNTAX_ERROR;
		      break;
		    }

		  switch (token)
		    {
		    case TOK_BACKWARD:
		      if ((sort_rule[nsort_rules] & FORWARD_BIT) != 0)
			error (0, 0, gettext ("%s:%Zd: directives `forward' "
					      "and `backward' are mutually "
					      "exclusive"),
			       locfile_data.filename, locfile_data.lineno);
		      else
			sort_rule[nsort_rules] |= BACKWARD_BIT;
		      break;
		    case TOK_FORWARD:
		      if ((sort_rule[nsort_rules] & BACKWARD_BIT) != 0)
			error (0, 0, gettext ("%s:%Zd: directives `forward' "
					      "and `backward' are mutually "
					      "exclusive"),
			       locfile_data.filename, locfile_data.lineno);
                      else
                        sort_rule[nsort_rules] |= FORWARD_BIT;
		      break;
		    case TOK_POSITION:
		      sort_rule[nsort_rules] |= POSITION_BIT;
		      break;
		    }

		  ++nsort_rules;

		  
		}
	      break;
	    default:
	      SYNTAX_ERROR;
	      ignore_to_eol (token, 0);
	    }
	}
      else
	{
	}
#endif

      ignore_to_eol(token,0);
      /* Get next token.  */
      token = xlocfile_lex (&ptr, &len);
    }
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
