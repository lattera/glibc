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
#include <locale.h>
#include <stdio.h>
#include <regex.h>

#include "localedef.h"

/* These are defined in locfile-parse.c.   */
extern struct cat_item LC_MESSAGES_desc[];
extern char *LC_MESSAGES_values[];

void
messages_check(void)
{
  int item_no;

  /* First general check for existence.  */
  for (item_no = 0; item_no < category[LC_MESSAGES].number; ++item_no)
    if (LC_MESSAGES_values[item_no] == NULL)
      {
	int errcode;

	errcode = LC_MESSAGES_desc[item_no].status == std ? 5 : 0;

	error (errcode, 0, gettext ("item `%s' of category `%s' undefined"),
	       LC_MESSAGES_desc[item_no].name, "LC_MESSAGES");
      }
    else
      {
	/* Some fields need special tests.  */
	if (LC_MESSAGES_desc[item_no].item_id == YESEXPR
	    || LC_MESSAGES_desc[item_no].item_id == NOEXPR)
	  /* The expression has to be a POSIX extended regular expression.  */
	  {
	    regex_t re;
	    int result;

	    result = regcomp (&re, LC_MESSAGES_values[item_no], REG_EXTENDED);

	    if (result != 0)
	      {
		char errbuf[BUFSIZ];

		(void) regerror (result, &re, errbuf, BUFSIZ);
		error (0, 0, gettext ("no correct regular expression for "
				      "item `%s' in category `%s': %s"),
		       LC_MESSAGES_desc[item_no].name, "LC_MESSAGES", errbuf);
	      }
	  }
      }
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
