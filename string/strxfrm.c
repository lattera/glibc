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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIDE_VERSION
# define STRING_TYPE char
# define USTRING_TYPE unsigned char
# define L_(Ch) Ch
# ifdef USE_IN_EXTENDED_LOCALE_MODEL
#  define STRXFRM __strxfrm_l
# else
#  define STRXFRM strxfrm
# endif
# define STRLEN strlen
# define STPNCPY __stpncpy
#endif

/* Include the shared helper functions.  `strxfrm'/`wcsxfrm' also use
   these functions.  */
#include "../locale/weight.h"


#ifndef WIDE_VERSION
/* Write 32 bit value UTF-8 encoded but only if enough space is left.  */
static __inline size_t
print_val (u_int32_t value, char *dest, size_t max, size_t act)
{
  char tmp[6];
  int idx = 0;

  if (value < 0x80)
    tmp[idx++] = (char) value;
  else
    {
      tmp[idx++] = '\x80' + (char) (value & 0x3f);
      value >>= 6;

      if (value < 0x20)
	tmp[idx++] = '\xc0' + (char) value;
      else
	{
	  tmp[idx++] = '\x80' + (char) (value & 0x3f);
	  value >>= 6;

	  if (value < 0x10)
	    tmp[idx++] = '\xe0' + (char) value;
	  else
	    {
	      tmp[idx++] = '\x80' + (char) (value & 0x3f);
	      value >>= 6;

	      if (value < 0x08)
		tmp[idx++] = '\xf0' + (char) value;
	      else
		{
		  tmp[idx++] = '\x80' + (char) (value & 0x3f);
		  value >>= 6;

		  if (value < 0x04)
		    tmp[idx++] = '\xf8' + (char) value;
		  else
		    {
		      tmp[idx++] = '\x80' + (char) (value & 0x3f);
		      tmp[idx++] = '\xfc' + (char) (value >> 6);
		    }
		}
	    }
	}
    }

  while (idx-- > 0)
    {
      if (act < max)
	dest[act] = tmp[idx];
      ++act;
    }

  return act;
}
#else
static __inline size_t
print_val (u_int32_t value, wchar_t *dest, size_t max, size_t act)
{
  /* We cannot really assume wchar_t is 32 bits wide.  But it is for
     GCC and so we don't do much optimization for the other case.  */
  if (sizeof (wchar_t) == 4)
    {
      if (act < max)
	dest[act] = (wchar_t) value;
      ++act;
    }
  else
    {
      wchar_t tmp[3];
      size_t idx = 0;

      if (value < 0x8000)
	tmp[idx++] = (wchar_t) act;
      else
	{
	  tmp[idx++] = (wchar_t) (0x8000 + (value & 0x3fff));
	  value >>= 14;
	  if (value < 0x2000)
	    tmp[idx++] = (wchar_t) (0xc000 + value);
	  else
	    {
	      tmp[idx++] = (wchar_t) (0x8000 + (value & 0x3fff));
	      value >>= 14;
	      tmp[idx++] = (wchar_t) (0xe000 + value);
	    }
	}
      while (idx-- > 0)
	{
	  if (act < max)
	    dest[act] = tmp[idx];
	  ++act;
	}
    }
  return act;
}
#endif


/* Transform SRC into a form such that the result of strcmp
   on two strings that have been transformed by strxfrm is
   the same as the result of strcoll on the two strings before
   their transformation.  The transformed string is put in at
   most N characters of DEST and its length is returned.  */
#ifndef USE_IN_EXTENDED_LOCALE_MODEL
size_t
STRXFRM (STRING_TYPE *dest, const STRING_TYPE *src, size_t n)
#else
size_t
STRXFRM (STRING_TYPE *dest, const STRING_TYPE *src, size_t n, __locale_t l)
#endif
{
#ifdef USE_IN_EXTENDED_LOCALE_MODEL
  struct locale_data *current = l->__locales[LC_COLLATE];
# if BYTE_ORDER == BIG_ENDIAN
  const u_int32_t *collate_table = (const u_int32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_TABLE_EB)].string;
  const u_int32_t *collate_extra = (const u_int32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EB)].string;
# elif BYTE_ORDER == LITTLE_ENDIAN
  const u_int32_t *collate_table = (const u_int32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_TABLE_EL)].string;
  const u_int32_t *collate_extra = (const u_int32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EL)].string;
# else
#  error bizarre byte order
# endif
#endif
  weight_t *forw = NULL;
  weight_t *backw = NULL;
  size_t pass;
  size_t written;

  /* If the current locale does not specify locale data we use normal
     8-bit string comparison.  */
  if (collate_nrules == 0)
    {
      if (n != 0)
	STPNCPY (dest, src, n);

      return STRLEN (src);
    }

  /* Handle an empty string as a special case.  */
  if (*src == '\0')
    {
      if (n != 0)
	*dest = '\0';
      return 1;
    }

  /* Get full information about the string.  This means we get
     information for all passes in a special data structure.  */
  get_string (src, forw, backw);

  /* Now we have all the information.  In at most the given number of
     passes we can finally decide about the order.  */
  written = 0;
  for (pass = 0; pass < collate_nrules; ++pass)
    {
      int forward = (collate_rules[pass] & sort_forward) != 0;
      const weight_t *run = forward ? forw : backw;
      int idx = forward ? 0 : run->data[pass].number - 1;

      while (1)
	{
	  int ignore = 0;
	  u_int32_t w = 0;

	  /* Here we have to check for IGNORE entries.  If these are
	     found we count them and go on with he next value.  */
	  while (run != NULL
		 && ((w = run->data[pass].value[idx])
		     == (u_int32_t) IGNORE_CHAR))
	    {
	      ++ignore;
	      if ((forward && ++idx >= run->data[pass].number)
		  || (!forward && --idx < 0))
		{
		  weight_t *nextp = forward ? run->next : run->prev;
		  if (nextp == NULL)
		    {
		      w = 0;
		      /* No more non-INGOREd elements means lowest
			 possible value.  */
		      ignore = -1;
		    }
		  else
		    idx = forward ? 0 : nextp->data[pass].number - 1;
		  run = nextp;
		}
	    }

	  /* Stop if all characters are processed.  */
	  if (run == NULL)
	    break;

	  /* Now we have information of the number of ignored weights
	     and the value of the next weight.  We have to add 2
	     because 0 means EOS and 1 is the intermediate string end.  */
	  if ((collate_rules[pass] & sort_position) != 0)
	    written = print_val (ignore + 2, dest, n, written);

	  if (w != 0)
	    written = print_val (w, dest, n, written);

	  /* We have to increment the index counters.  */
	  if ((forward && ++idx >= run->data[pass].number)
	      || (!forward && --idx < 0))
	    {
	      if (forward)
		{
		  run = run->next;
		  idx = 0;
		}
	      else
		{
		  run = run->prev;
		  if (run != NULL)
		    idx = run->data[pass].number - 1;
		}
	    }
	}

      /* Write marker for end of word.  */
      if (pass + 1 < collate_nrules)
	written = print_val (1, dest, n, written);
    }

  /* Terminate string.  */
  if (written < n)
    dest[written] = L_('\0');

  /* Return length without counting the terminating '\0'.  */
  return written;
}
