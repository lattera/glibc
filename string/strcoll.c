/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRING_TYPE
# define STRING_TYPE char
# define USTRING_TYPE unsigned char
# define STRCOLL strcoll
# define STRCMP strcmp
#endif

/* Include the shared helper functions.  `strxfrm'/`wcsxfrm' also use
   these functions.  */
#include "../locale/weight.h"


/* Compare S1 and S2, returning less than, equal to or
   greater than zero if the collated form of S1 is lexicographically
   less than, equal to or greater than the collated form of S2.  */
int
STRCOLL (s1, s2)
     const STRING_TYPE *s1;
     const STRING_TYPE *s2;
{
  weight_t *s1forw = NULL;
  weight_t *s1backw = NULL;
  weight_t *s2forw = NULL;
  weight_t *s2backw = NULL;
  size_t pass;

  /* If the current locale does not specify locale data we use normal
     8-bit string comparison.  */
  if (collate_nrules == 0)
    return STRCMP (s1, s2);

  /* Get full information about the strings.  This means we get
     information for all passes in a special data structure.  */
  get_string (s1, s1forw, s1backw);
  get_string (s2, s2forw, s2backw);

  /* Now we have all the information.  In at most the given number of
     passes we can finally decide about the order.  */
  for (pass = 0; pass < collate_nrules; ++pass)
    {
      int forward = (collate_rules[pass] & sort_forward) != 0;
      const weight_t *s1run = forward ? s1forw : s1backw;
      const weight_t *s2run = forward ? s2forw : s2backw;
      int s1idx = forward ? 0 : s1run->data[pass].number - 1;
      int s2idx = forward ? 0 : s2run->data[pass].number - 1;

      do
	{
	  int s1ignore = 0;
	  int s2ignore = 0;
	  u_int32_t w1, w2;

	  /* Here we have to check for IGNORE entries.  If these are
	     found we count them and go on witht he next value.  */
	  while ((w1 = s1run->data[pass].value[s1idx])
		 == (u_int32_t) IGNORE_CHAR)
	    {
	      ++s1ignore;
	      if ((forward && ++s1idx >= s1run->data[pass].number)
		  || (!forward && --s1idx < 0))
		{
		  weight_t *nextp = forward ? s1run->next : s1run->prev;
		  if (nextp == NULL)
		    {
		      w1 = 0;
		      break;
		    }
		  s1run = nextp;
		  s1idx = forward ? 0 : s1run->data[pass].number - 1;
		}
	    }

	  while ((w2 = s2run->data[pass].value[s2idx])
		 == (u_int32_t) IGNORE_CHAR)
	    {
	      ++s2ignore;
	      if ((forward && ++s2idx >= s2run->data[pass].number)
		  || (!forward && --s2idx < 0))
		{
		  weight_t *nextp = forward ? s2run->next : s2run->prev;
		  if (nextp == NULL)
		    {
		      w2 = 0;
		      break;
		    }
		  s2run = nextp;
		  s2idx = forward ? 0 : s2run->data[pass].number - 1;
		}
	    }

	  /* Now we have information of the number of ignored
	     weights and the value of the next weight.  */
	  if ((collate_rules[pass] & sort_position) != 0
	      && s1ignore != s2ignore && (w1 != 0 || w2 != 0))
	    return s1ignore < s2ignore ? -1 : 1;

	  if (w1 != w2)
	    return w1 < w2 ? -1 : 1;

	  /* We have to increment the index counters.  */
	  if ((forward && ++s1idx >= s1run->data[pass].number)
	      || (!forward && --s1idx < 0))
	    if (forward)
	      {
		s1run = s1run->next;
		s1idx = 0;
	      }
	    else
	      {
		s1run = s1run->prev;
		if (s1run != NULL)
		  s1idx = s1run->data[pass].number - 1;
	      }

	  if ((forward && ++s2idx >= s2run->data[pass].number)
	      || (!forward && --s2idx < 0))
	    if (forward)
	      {
		s2run = s2run->next;
		s2idx = 0;
	      }
	    else
	      {
		s2run = s2run->prev;
		if (s2run != NULL)
		  s2idx = s2run->data[pass].number - 1;
	      }

	}
      while (s1run != NULL && s2run != NULL);

      if (s1run != s2run)
	return s1run != NULL ? 1 : -1;
    }

  return 0;
}
