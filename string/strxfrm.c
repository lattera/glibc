/* Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

#include <langinfo.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../locale/localeinfo.h"

#ifdef USE_IN_EXTENDED_LOCALE_MODEL
# define STRXFRM __strxfrm_l
#else
# define STRXFRM strxfrm
#endif


/* These are definitions used by some of the functions for handling
   UTF-8 encoding below.  */
static const uint32_t encoding_mask[] =
{
  ~0x7ff, ~0xffff, ~0x1fffff, ~0x3ffffff
};

static const unsigned char encoding_byte[] =
{
  0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};


/* We need UTF-8 encoding of numbers.  */
static inline int
utf8_encode (char *buf, int val)
{
  char *startp = buf;
  int retval;

  if (val < 0x80)
    {
      *buf++ = (char) val;
      retval = 1;
    }
  else
    {
      int step;

      for (step = 2; step < 6; ++step)
	if ((val & encoding_mask[step - 2]) == 0)
	  break;
      retval = step;

      *buf = encoding_byte[step - 2];
      --step;
      do
	{
	  buf[step] = 0x80 | (val & 0x3f);
	  val >>= 6;
	}
      while (--step > 0);
      *buf |= val;
    }

  return buf - startp;
}


#ifndef USE_IN_EXTENDED_LOCALE_MODEL
size_t
STRXFRM (char *dest, const char *src, size_t n)
#else
size_t
STRXFRM (char *dest, const char *src, size_t n, __locale_t l)
#endif
{
#ifdef USE_IN_EXTENDED_LOCALE_MODEL
  struct locale_data *current = l->__locales[LC_COLLATE];
  uint_fast32_t nrules = *((uint32_t *) current->values[_NL_ITEM_INDEX (_NL_COLLATE_NRULES)].string);
#else
  uint32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
#endif
  /* We don't assign the following values right away since it might be
     unnecessary in case there are no rules.  */
  const unsigned char *rulesets;
  const int32_t *table;
  const unsigned char *weights;
  const unsigned char *extra;
  const int32_t *indirect;
  uint_fast32_t pass;
  size_t needed;
  const unsigned char *usrc;
  size_t srclen = strlen (src);
  int32_t *idxarr;
  unsigned char *rulearr;
  size_t idxmax;
  size_t idxcnt;
  int use_malloc = 0;

#include "../locale/weight.h"

  if (nrules == 0)
    {
      if (n != 0)
	__stpncpy (dest, src, n);

      return srclen;
    }

#ifdef USE_IN_EXTENDED_LOCALE_MODEL
  rulesets = (const unsigned char *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_RULESETS)].string;
  table = (const int32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_TABLEMB)].string;
  weights = (const unsigned char *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_WEIGHTMB)].string;
  extra = (const unsigned char *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_EXTRAMB)].string;
  indirect = (const int32_t *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_INDIRECTMB)].string;
#else
  rulesets = (const unsigned char *)
    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_RULESETS);
  table = (const int32_t *)
    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
  weights = (const unsigned char *)
    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTMB);
  extra = (const unsigned char *)
    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAMB);
  indirect = (const int32_t *)
    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_INDIRECTMB);
#endif

  /* Handle an empty string as a special case.  */
  if (srclen == 0)
    {
      if (n != 0)
        *dest = '\0';
      return 1;
    }

  /* We need the elements of the string as unsigned values since they
     are used as indeces.  */
  usrc = (const unsigned char *) src;

  /* Perform the first pass over the string and while doing this find
     and store the weights for each character.  Since we want this to
     be as fast as possible we are using `alloca' to store the temporary
     values.  But since there is no limit on the length of the string
     we have to use `malloc' if the string is too long.  We should be
     very conservative here.  */
  if (srclen >= 16384)
    {
      idxarr = (int32_t *) malloc (srclen * (sizeof (int32_t) + 1));
      rulearr = (unsigned char *) &idxarr[srclen];

      if (idxarr == NULL)
	/* No memory.  Well, go with the stack then.

	   XXX Once this implementation is stable we will handle this
	   differently.  Instead of precomputing the indeces we will
	   do this in time.  This means, though, that this happens for
	   every pass again.  */
	goto try_stack;
      use_malloc = 1;
    }
  else
    {
    try_stack:
      idxarr = (int32_t *) alloca (srclen * sizeof (int32_t));
      rulearr = (unsigned char *) alloca (srclen);
    }

  idxmax = 0;
  do
    {
      int32_t tmp = findidx (&usrc);
      rulearr[idxmax] = tmp >> 24;
      idxarr[idxmax] = tmp & 0xffffff;

      ++idxmax;
    }
  while (*usrc != '\0');

  /* Now the passes over the weights.  We now use the indeces we found
     before.  */
  needed = 0;
  for (pass = 0; pass < nrules; ++pass)
    {
      size_t backw_stop = ~0ul;
      int rule = rulesets[rulearr[0] * nrules + pass];
      /* We assume that if a rule has defined `position' in one section
	 this is true for all of them.  */
      int position = rule & sort_position;

      if (position == 0)
	{
	  for (idxcnt = 0; idxcnt < idxmax; ++idxcnt)
	    {
	      if ((rule & sort_forward) != 0)
		{
		  size_t len;

		  if (backw_stop != ~0ul)
		    {
		      /* Handle the pushed elements now.  */
		      size_t backw;

		      for (backw = idxcnt - 1; backw >= backw_stop; --backw)
			{
			  len = weights[idxarr[backw]++];

			  if (needed + len < n)
			    while (len-- > 0)
			      dest[needed++] = weights[idxarr[backw]++];
			  else
			    {
				/* No more characters fit into the buffer.  */
			      needed += len;
			      idxarr[backw] += len;
			    }
			}

		      backw_stop = ~0ul;
		    }

		  /* Now handle the forward element.  */
		  len = weights[idxarr[idxcnt]++];
		  if (needed + len < n)
		    while (len-- > 0)
		      dest[needed++] = weights[idxarr[idxcnt]++];
		  else
		    {
		      /* No more characters fit into the buffer.  */
		      needed += len;
		      idxarr[idxcnt] += len;
		    }
		}
	      else
		{
		  /* Remember where the backwards series started.  */
		  if (backw_stop == ~0ul)
		    backw_stop = idxcnt;
		}

	      rule = rulesets[rulearr[idxcnt + 1] * nrules + pass];
	    }


	  if (backw_stop != ~0ul)
	    {
	      /* Handle the pushed elements now.  */
	      size_t backw;

	      backw = idxcnt;
	      while (backw > backw_stop)
		{
		  size_t len = weights[idxarr[--backw]++];

		  if (needed + len < n)
		    while (len-- > 0)
		      dest[needed++] = weights[idxarr[backw]++];
		  else
		    {
		      /* No more characters fit into the buffer.  */
		      needed += len;
		      idxarr[backw] += len;
		    }
		}
	    }
	}
      else
	{
	  int val = 1;
	  char buf[7];
	  size_t buflen;
	  size_t i;

	  for (idxcnt = 0; idxcnt < idxmax; ++idxcnt)
	    {
	      if ((rule & sort_forward) != 0)
		{
		  size_t len;

		  if (backw_stop != ~0ul)
		    {
		     /* Handle the pushed elements now.  */
		      size_t backw;

		      for (backw = idxcnt - 1; backw >= backw_stop; --backw)
			{
			  len = weights[idxarr[backw]++];
			  if (len != 0)
			    {
			      buflen = utf8_encode (buf, val);
			      if (needed + buflen + len < n)
				{
				  for (i = 0; i < buflen; ++i)
				    dest[needed + i] = buf[i];
				  for (i = 0; i < len; ++i)
				    dest[needed + buflen + i] =
				      weights[idxarr[backw] + i];
				}
			      idxarr[backw] += len;
			      needed += buflen + len;
			      val = 1;
			    }
			  else
			    ++val;
			}

		      backw_stop = ~0ul;
		    }

		  /* Now handle the forward element.  */
		  len = weights[idxarr[idxcnt]++];
		  if (len != 0)
		    {
		      buflen = utf8_encode (buf, val);
		      if (needed + buflen + len < n)
			{
			  for (i = 0; i < buflen; ++i)
			    dest[needed + i] = buf[i];
			  for (i = 0; i < len; ++i)
			    dest[needed + buflen + i] =
			      weights[idxarr[idxcnt] + i];
			}
		      idxarr[idxcnt] += len;
		      needed += buflen + len;
		      val = 1;
		    }
		  else
		    /* Note that we don't have to increment `idxarr[idxcnt]'
		       since the length is zero.  */
		    ++val;
		}
	      else
		{
		  /* Remember where the backwards series started.  */
		  if (backw_stop == ~0ul)
		    backw_stop = idxcnt;
		}

	      rule = rulesets[rulearr[idxcnt + 1] * nrules + pass];
	    }

	  if (backw_stop != ~0)
	    {
	      /* Handle the pushed elements now.  */
	      size_t backw;

	      backw = idxmax - 1;
	      while (backw > backw_stop)
		{
		  size_t len = weights[idxarr[--backw]++];
		  if (len != 0)
		    {
		      buflen = utf8_encode (buf, val);
		      if (needed + buflen + len < n)
			{
			  for (i = 0; i < buflen; ++i)
			    dest[needed + i] = buf[i];
			  for (i = 0; i < len; ++i)
			    dest[needed + buflen + i] =
			      weights[idxarr[backw] + i];
			}
		      idxarr[backw] += len;
		      needed += buflen + len;
		      val = 1;
		    }
		  else
		    ++val;
		}
	    }
	}

      /* Finally store the byte to separate the passes or terminate
	 the string.  */
      if (needed < n)
	dest[needed] = pass + 1 < nrules ? '\1' : '\0';
      ++needed;
    }

  /* This is a little optimization: many collation specifications have
     a `position' rule at the end and if no non-ignored character
     is found the last \1 byte is immediately followed by a \0 byte
     signalling this.  We can avoid the \1 byte(s).  */
  if (needed <= n && needed > 2 && dest[needed - 2] == '\1')
    {
      /* Remove the \1 byte.  */
      --needed;
      dest[needed - 1] = '\0';
    }

  /* Free the memory if needed.  */
  if (use_malloc)
    free (idxarr);

  return needed;
}
