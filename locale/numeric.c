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

#include "localedef.h"

/* These are defined in locfile-parse.c.   */
extern struct cat_item LC_NUMERIC_desc[];
extern char *LC_NUMERIC_values[];

void
numeric_check(void)
{
  int item_no;

  /* First general check for existence.  */
  for (item_no = 0; item_no < category[LC_NUMERIC].number; ++item_no)
    if (LC_NUMERIC_values[item_no] == NULL)
      {
	int errcode;

	errcode = LC_NUMERIC_desc[item_no].status = std ? 5 : 0;

	error (errcode, 0, gettext ("item `%s' of category `%s' undefined"),
	       LC_NUMERIC_desc[item_no].name, "LC_NUMERIC");
      }
    else
      {
	if (LC_NUMERIC_desc[item_no].item_id == DECIMAL_POINT
	    && LC_NUMERIC_values[item_no][0] == '\0')
	  /* The decimal point must not be empty.  This is not said
	     explicitly in POSIX but ANSI C (ISO/IEC 9899) says in
	     4.4.2.1 it has to be != "".  */
	  error (0, 0,
		 gettext ("item `%s' in category `%s' must not be empty"),
		 LC_NUMERIC_desc[item_no].name, "LC_NUMERIC");
      }
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
