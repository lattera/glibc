/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <endian.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRING_TYPE
# define STRING_TYPE char
# define USTRING_TYPE unsigned char
# ifdef USE_IN_EXTENDED_LOCALE_MODEL
#  define STRCOLL __strcoll_l
# else
#  define STRCOLL strcoll
# endif
# define STRCMP strcmp
#endif

/* Include the shared helper functions.  `strxfrm'/`wcsxfrm' also use
   these functions.  */
#include "../locale/weight.h"


/* Compare S1 and S2, returning less than, equal to or
   greater than zero if the collated form of S1 is lexicographically
   less than, equal to or greater than the collated form of S2.  */
#ifndef USE_IN_EXTENDED_LOCALE_MODEL
int
STRCOLL (s1, s2)
     const STRING_TYPE *s1;
     const STRING_TYPE *s2;
#else
int
STRCOLL (s1, s2, l)
     const STRING_TYPE *s1;
     const STRING_TYPE *s2;
     __locale_t l;
#endif
{
#ifdef USE_IN_EXTENDED_LOCALE_MODEL
  struct locale_data *current = l->__locales[LC_COLLATE];
# if BYTE_ORDER == BIG_ENDIAN
  const uint32_t *collate_table = (const uint32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_TABLE_EB)].string;
  const uint32_t *collate_extra = (const uint32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EB)].string;
# elif BYTE_ORDER == LITTLE_ENDIAN
  const uint32_t *collate_table = (const uint32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_TABLE_EL)].string;
  const uint32_t *collate_extra = (const uint32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EL)].string;
# else
#  error bizarre byte order
# endif
#endif
  weight_t *s1forw = NULL;
  weight_t *s1backw = NULL;
  weight_t *s2forw = NULL;
  weight_t *s2backw = NULL;
  size_t pass;

  /* If the current locale does not specify locale data we use normal
     8-bit string comparison.  */
  if (collate_nrules == 0)
    return STRCMP (s1, s2);

  /* Handle empty strings as a special case.  */
  if (*s1 == '\0')
    return *s2 == '\0' ? 0 : -1;
  else if (*s2 == '\0')
    return 1;

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

      while (1)
	{
	  int s1ignore = 0;
	  int s2ignore = 0;
	  uint32_t w1 = 0;
	  uint32_t w2 = 0;

	  /* Here we have to check for IGNORE entries.  If these are
	     found we count them and go on with the next value.  */
	  while (s1run != NULL
		 && ((w1 = s1run->data[pass].value[s1idx])
		     == (uint32_t) IGNORE_CHAR))
	    {
	      ++s1ignore;
	      if ((forward && ++s1idx >= s1run->data[pass].number)
		  || (!forward && --s1idx < 0))
		{
		  weight_t *nextp = forward ? s1run->next : s1run->prev;
		  if (nextp == NULL)
		    {
		      w1 = 0;
		      /* No more non-INGOREd elements means lowest
			 possible value.  */
		      s1ignore = -1;
		    }
		  else
		    s1idx = forward ? 0 : nextp->data[pass].number - 1;
		  s1run = nextp;
		}
	    }

	  while (s2run != NULL
		 && ((w2 = s2run->data[pass].value[s2idx])
		     == (uint32_t) IGNORE_CHAR))
	    {
	      ++s2ignore;
	      if ((forward && ++s2idx >= s2run->data[pass].number)
		  || (!forward && --s2idx < 0))
		{
		  weight_t *nextp = forward ? s2run->next : s2run->prev;
		  if (nextp == NULL)
		    {
		      w2 = 0;
		      /* No more non-INGOREd elements means lowest
			 possible value.  */
		      s2ignore = -1;
		    }
		  else
		    s2idx = forward ? 0 : nextp->data[pass].number - 1;
		  s2run = nextp;
		}
	    }

	  /* If one string is completely processed stop.  */
	  if (s1run == NULL || s2run == NULL)
	    break;

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

      if (s1run != s2run)
	return s1run != NULL ? 1 : -1;
    }

  return 0;
}
