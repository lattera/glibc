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
#include <stdlib.h>
#include <string.h>

#include "localedef.h"
#include "token.h"


/* The content iof the field int_curr_symbol has to be taken from
   ISO-4217.  We test for correct values.  */
#define DEFINE_INT_CURR(str) str,
static const char *const valid_int_curr[] =
  {
#   include "iso-4217.def"
  };
#define NVALID_INT_CURR ((sizeof (valid_int_curr) \
			 / sizeof (valid_int_curr[0])))
#undef DEFINE_INT_CURR


/* These are defined in locfile-parse.c.  */
extern struct cat_item LC_MONETARY_desc[];
extern char *LC_MONETARY_values[];

static int _curr_strcmp(const char *s1, const char **s2);



void
monetary_check(void)
{
  int item_no, val;

  for (item_no = 0; LC_MONETARY_desc[item_no].item_id != 0; ++item_no)
    /* Test whether the entry has been defined.  Byte values are simply
       stored.  */
    if (LC_MONETARY_values[item_no] == NULL)
      {
	int errcode;

	errcode = LC_MONETARY_desc[item_no].status = std ? 5 : 0;

	error (errcode, 0, gettext ("item `%s' of category `%s' undefined"),
	       LC_MONETARY_desc[item_no].name, "LC_MONETARY");
      }
    else
      switch (LC_MONETARY_desc[item_no].item_id)
	{
	case INT_CURR_SYMBOL:
	  if (strlen (LC_MONETARY_values[item_no]) != 4)
	    error (0, 0,
		   gettext ("item `%s' of category `%s' has wrong length"),
		   LC_MONETARY_desc[item_no].name, "LC_MONETARY");
	  else if (bsearch (LC_MONETARY_values[item_no], valid_int_curr,
		       NVALID_INT_CURR, sizeof (char *),
		       (comparison_fn_t) _curr_strcmp) == NULL)
	    error (0, 0, gettext ("item `%s' does not correspond to any "
				  "valid name in ISO-4217"),
		   LC_MONETARY_desc[item_no].name);
	  break;
	case P_CS_PRECEDES:
	case P_SEP_BY_SPACE:
	case N_CS_PRECEDES:
	case N_SEP_BY_SPACE:
	case  P_SIGN_POSN:
	case N_SIGN_POSN:
	  val = (int) *(char *) LC_MONETARY_values[item_no];
	  if (val < LC_MONETARY_desc[item_no].min
	      || val > LC_MONETARY_desc[item_no].max)
	    error (0, 0, gettext ("value for item `%s' in category `%s' "
				  "must be in range %d...%d"),
		   LC_MONETARY_desc[item_no].name, "LC_MONETARY",
		   LC_MONETARY_desc[item_no].min,
		   LC_MONETARY_desc[item_no].max);
	  break;
	case MON_DECIMAL_POINT:
	  /* The decimal point must not be empty.  This is not said
	     explicitly in POSIX but ANSI C (ISO/IEC 9899) says in
	     4.4.2.1 it has to be != "".  */
	  if (LC_MONETARY_values[item_no][0] == '\0')
	    error (0, 0,
		   gettext ("item `%s' in category `%s' must not be empty"),
		   LC_MONETARY_desc[item_no].name, "LC_MONETARY");
	  break;
	case CURRENCY_SYMBOL:
	case MON_THOUSANDS_SEP:
	case MON_GROUPING:
	case POSITIVE_SIGN:
	case NEGATIVE_SIGN:
	case INT_FRAC_DIGITS:
	case FRAC_DIGITS:
	  /* Everything is ok for these values.  */
	  break;
	default:
	  error (5, 0, gettext ("Internal error in %s, line %u"),
		 __FUNCTION__, __LINE__);
	  /* NOTREACHED */
	}

}


static int
_curr_strcmp(const char *s1, const char **s2)
{
  return strcmp (s1, *s2);
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
