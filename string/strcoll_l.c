/* Copyright (C) 1995-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Ulrich Drepper <drepper@gnu.org>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


#include <assert.h>
#include <langinfo.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef STRING_TYPE
# define STRING_TYPE char
# define USTRING_TYPE unsigned char
# define STRCOLL __strcoll_l
# define STRCMP strcmp
# define STRLEN strlen
# define WEIGHT_H "../locale/weight.h"
# define SUFFIX	MB
# define L(arg) arg
#endif

#define CONCAT(a,b) CONCAT1(a,b)
#define CONCAT1(a,b) a##b

#include "../locale/localeinfo.h"

/* Track status while looking for sequences in a string.  */
typedef struct
{
  int len;			/* Length of the current sequence.  */
  size_t val;			/* Position of the sequence relative to the
				   previous non-ignored sequence.  */
  size_t idxnow;		/* Current index in sequences.  */
  size_t idxmax;		/* Maximum index in sequences.  */
  size_t idxcnt;		/* Current count of indices.  */
  size_t backw;			/* Current Backward sequence index.  */
  size_t backw_stop;		/* Index where the backward sequences stop.  */
  const USTRING_TYPE *us;	/* The string.  */
  int32_t *idxarr;		/* Array to cache weight indices.  */
  unsigned char *rulearr;	/* Array to cache rules.  */
  unsigned char rule;		/* Saved rule for the first sequence.  */
  int32_t idx;			/* Index to weight of the current sequence.  */
  int32_t save_idx;		/* Save looked up index of a forward
				   sequence after the last backward
				   sequence.  */
  const USTRING_TYPE *back_us;	/* Beginning of the backward sequence.  */
} coll_seq;

/* Get next sequence.  The weight indices are cached, so we don't need to
   traverse the string.  */
static void
get_next_seq_cached (coll_seq *seq, int nrules, int pass,
		     const unsigned char *rulesets,
		     const USTRING_TYPE *weights)
{
  size_t val = seq->val = 0;
  int len = seq->len;
  size_t backw_stop = seq->backw_stop;
  size_t backw = seq->backw;
  size_t idxcnt = seq->idxcnt;
  size_t idxmax = seq->idxmax;
  size_t idxnow = seq->idxnow;
  unsigned char *rulearr = seq->rulearr;
  int32_t *idxarr = seq->idxarr;

  while (len == 0)
    {
      ++val;
      if (backw_stop != ~0ul)
	{
	  /* There is something pushed.  */
	  if (backw == backw_stop)
	    {
	      /* The last pushed character was handled.  Continue
		 with forward characters.  */
	      if (idxcnt < idxmax)
		{
		  idxnow = idxcnt;
		  backw_stop = ~0ul;
		}
	      else
		{
		  /* Nothing any more.  The backward sequence
		     ended with the last sequence in the string.  */
		  idxnow = ~0ul;
		  break;
		}
	    }
	  else
	    idxnow = --backw;
	}
      else
	{
	  backw_stop = idxcnt;

	  while (idxcnt < idxmax)
	    {
	      if ((rulesets[rulearr[idxcnt] * nrules + pass]
		   & sort_backward) == 0)
		/* No more backward characters to push.  */
		break;
	      ++idxcnt;
	    }

	  if (backw_stop == idxcnt)
	    {
	      /* No sequence at all or just one.  */
	      if (idxcnt == idxmax)
		/* Note that LEN is still zero.  */
		break;

	      backw_stop = ~0ul;
	      idxnow = idxcnt++;
	    }
	  else
	    /* We pushed backward sequences.  */
	    idxnow = backw = idxcnt - 1;
	}
      len = weights[idxarr[idxnow]++];
    }

  /* Update the structure.  */
  seq->val = val;
  seq->len = len;
  seq->backw_stop = backw_stop;
  seq->backw = backw;
  seq->idxcnt = idxcnt;
  seq->idxnow = idxnow;
}

/* Get next sequence.  Traverse the string as required.  */
static void
get_next_seq (coll_seq *seq, int nrules, const unsigned char *rulesets,
	      const USTRING_TYPE *weights, const int32_t *table,
	      const USTRING_TYPE *extra, const int32_t *indirect)
{
#include WEIGHT_H
  size_t val = seq->val = 0;
  int len = seq->len;
  size_t backw_stop = seq->backw_stop;
  size_t backw = seq->backw;
  size_t idxcnt = seq->idxcnt;
  size_t idxmax = seq->idxmax;
  size_t idxnow = seq->idxnow;
  unsigned char *rulearr = seq->rulearr;
  int32_t *idxarr = seq->idxarr;
  const USTRING_TYPE *us = seq->us;

  while (len == 0)
    {
      ++val;
      if (backw_stop != ~0ul)
	{
	  /* There is something pushed.  */
	  if (backw == backw_stop)
	    {
	      /* The last pushed character was handled.  Continue
		 with forward characters.  */
	      if (idxcnt < idxmax)
		{
		  idxnow = idxcnt;
		  backw_stop = ~0ul;
		}
	      else
		/* Nothing any more.  The backward sequence ended with
		   the last sequence in the string.  Note that LEN
		   is still zero.  */
		break;
	    }
	  else
	    idxnow = --backw;
	}
      else
	{
	  backw_stop = idxmax;

	  while (*us != L('\0'))
	    {
	      int32_t tmp = findidx (&us, -1);
	      rulearr[idxmax] = tmp >> 24;
	      idxarr[idxmax] = tmp & 0xffffff;
	      idxcnt = idxmax++;

	      if ((rulesets[rulearr[idxcnt] * nrules]
		   & sort_backward) == 0)
		/* No more backward characters to push.  */
		break;
	      ++idxcnt;
	    }

	  if (backw_stop >= idxcnt)
	    {
	      /* No sequence at all or just one.  */
	      if (idxcnt == idxmax || backw_stop > idxcnt)
		/* Note that LEN is still zero.  */
		break;

	      backw_stop = ~0ul;
	      idxnow = idxcnt;
	    }
	  else
	    /* We pushed backward sequences.  */
	    idxnow = backw = idxcnt - 1;
	}
      len = weights[idxarr[idxnow]++];
    }

  /* Update the structure.  */
  seq->val = val;
  seq->len = len;
  seq->backw_stop = backw_stop;
  seq->backw = backw;
  seq->idxcnt = idxcnt;
  seq->idxmax = idxmax;
  seq->idxnow = idxnow;
  seq->us = us;
}

/* Get next sequence.  Traverse the string as required.  This function does not
   set or use any index or rule cache.  */
static void
get_next_seq_nocache (coll_seq *seq, int nrules, const unsigned char *rulesets,
		      const USTRING_TYPE *weights, const int32_t *table,
		      const USTRING_TYPE *extra, const int32_t *indirect,
		      int pass)
{
#include WEIGHT_H
  size_t val = seq->val = 0;
  int len = seq->len;
  size_t backw_stop = seq->backw_stop;
  size_t backw = seq->backw;
  size_t idxcnt = seq->idxcnt;
  size_t idxmax = seq->idxmax;
  int32_t idx = seq->idx;
  const USTRING_TYPE *us = seq->us;

  while (len == 0)
    {
      ++val;
      if (backw_stop != ~0ul)
	{
	  /* There is something pushed.  */
	  if (backw == backw_stop)
	    {
	      /* The last pushed character was handled.  Continue
		 with forward characters.  */
	      if (idxcnt < idxmax)
		{
		  idx = seq->save_idx;
		  backw_stop = ~0ul;
		}
	      else
		{
		  /* Nothing anymore.  The backward sequence ended with
		     the last sequence in the string.  Note that len is
		     still zero.  */
		  idx = 0;
		  break;
	        }
	    }
	  else
	    {
	      /* XXX Traverse BACKW sequences from the beginning of
		 BACKW_STOP to get the next sequence.  Is ther a quicker way
	         to do this?  */
	      size_t i = backw_stop;
	      us = seq->back_us;
	      while (i < backw)
		{
		  int32_t tmp = findidx (&us, -1);
		  idx = tmp & 0xffffff;
		  i++;
		}
	      --backw;
	      us = seq->us;
	    }
	}
      else
	{
	  backw_stop = idxmax;
	  int32_t prev_idx = idx;

	  while (*us != L('\0'))
	    {
	      int32_t tmp = findidx (&us, -1);
	      unsigned char rule = tmp >> 24;
	      prev_idx = idx;
	      idx = tmp & 0xffffff;
	      idxcnt = idxmax++;

	      /* Save the rule for the first sequence.  */
	      if (__glibc_unlikely (idxcnt == 0))
	        seq->rule = rule;

	      if ((rulesets[rule * nrules + pass]
		   & sort_backward) == 0)
		/* No more backward characters to push.  */
		break;
	      ++idxcnt;
	    }

	  if (backw_stop >= idxcnt)
	    {
	      /* No sequence at all or just one.  */
	      if (idxcnt == idxmax || backw_stop > idxcnt)
		/* Note that len is still zero.  */
		break;

	      backw_stop = ~0ul;
	    }
	  else
	    {
	      /* We pushed backward sequences.  If the stream ended with the
		 backward sequence, then we process the last sequence we
		 found.  Otherwise we process the sequence before the last
		 one since the last one was a forward sequence.  */
	      seq->back_us = seq->us;
	      seq->us = us;
	      backw = idxcnt;
	      if (idxmax > idxcnt)
		{
		  backw--;
		  seq->save_idx = idx;
		  idx = prev_idx;
		}
	      if (backw > backw_stop)
		backw--;
	    }
	}

      len = weights[idx++];
      /* Skip over indices of previous levels.  */
      for (int i = 0; i < pass; i++)
	{
	  idx += len;
	  len = weights[idx];
	  idx++;
	}
    }

  /* Update the structure.  */
  seq->val = val;
  seq->len = len;
  seq->backw_stop = backw_stop;
  seq->backw = backw;
  seq->idxcnt = idxcnt;
  seq->idxmax = idxmax;
  seq->us = us;
  seq->idx = idx;
}

/* Compare two sequences.  This version does not use the index and rules
   cache.  */
static int
do_compare_nocache (coll_seq *seq1, coll_seq *seq2, int position,
		    const USTRING_TYPE *weights)
{
  int seq1len = seq1->len;
  int seq2len = seq2->len;
  size_t val1 = seq1->val;
  size_t val2 = seq2->val;
  int idx1 = seq1->idx;
  int idx2 = seq2->idx;
  int result = 0;

  /* Test for position if necessary.  */
  if (position && val1 != val2)
    {
      result = val1 > val2 ? 1 : -1;
      goto out;
    }

  /* Compare the two sequences.  */
  do
    {
      if (weights[idx1] != weights[idx2])
	{
	  /* The sequences differ.  */
	  result = weights[idx1] - weights[idx2];
	  goto out;
	}

      /* Increment the offsets.  */
      ++idx1;
      ++idx2;

      --seq1len;
      --seq2len;
    }
  while (seq1len > 0 && seq2len > 0);

  if (position && seq1len != seq2len)
    result = seq1len - seq2len;

out:
  seq1->len = seq1len;
  seq2->len = seq2len;
  seq1->idx = idx1;
  seq2->idx = idx2;
  return result;
}

/* Compare two sequences using the index cache.  */
static int
do_compare (coll_seq *seq1, coll_seq *seq2, int position,
	    const USTRING_TYPE *weights)
{
  int seq1len = seq1->len;
  int seq2len = seq2->len;
  size_t val1 = seq1->val;
  size_t val2 = seq2->val;
  int32_t *idx1arr = seq1->idxarr;
  int32_t *idx2arr = seq2->idxarr;
  int idx1now = seq1->idxnow;
  int idx2now = seq2->idxnow;
  int result = 0;

  /* Test for position if necessary.  */
  if (position && val1 != val2)
    {
      result = val1 > val2 ? 1 : -1;
      goto out;
    }

  /* Compare the two sequences.  */
  do
    {
      if (weights[idx1arr[idx1now]] != weights[idx2arr[idx2now]])
	{
	  /* The sequences differ.  */
	  result = weights[idx1arr[idx1now]] - weights[idx2arr[idx2now]];
	  goto out;
	}

      /* Increment the offsets.  */
      ++idx1arr[idx1now];
      ++idx2arr[idx2now];

      --seq1len;
      --seq2len;
    }
  while (seq1len > 0 && seq2len > 0);

  if (position && seq1len != seq2len)
    result = seq1len - seq2len;

out:
  seq1->len = seq1len;
  seq2->len = seq2len;
  return result;
}

int
STRCOLL (const STRING_TYPE *s1, const STRING_TYPE *s2, __locale_t l)
{
  struct __locale_data *current = l->__locales[LC_COLLATE];
  uint_fast32_t nrules = current->values[_NL_ITEM_INDEX (_NL_COLLATE_NRULES)].word;
  /* We don't assign the following values right away since it might be
     unnecessary in case there are no rules.  */
  const unsigned char *rulesets;
  const int32_t *table;
  const USTRING_TYPE *weights;
  const USTRING_TYPE *extra;
  const int32_t *indirect;

  if (nrules == 0)
    return STRCMP (s1, s2);

  rulesets = (const unsigned char *)
    current->values[_NL_ITEM_INDEX (_NL_COLLATE_RULESETS)].string;
  table = (const int32_t *)
    current->values[_NL_ITEM_INDEX (CONCAT(_NL_COLLATE_TABLE,SUFFIX))].string;
  weights = (const USTRING_TYPE *)
    current->values[_NL_ITEM_INDEX (CONCAT(_NL_COLLATE_WEIGHT,SUFFIX))].string;
  extra = (const USTRING_TYPE *)
    current->values[_NL_ITEM_INDEX (CONCAT(_NL_COLLATE_EXTRA,SUFFIX))].string;
  indirect = (const int32_t *)
    current->values[_NL_ITEM_INDEX (CONCAT(_NL_COLLATE_INDIRECT,SUFFIX))].string;

  assert (((uintptr_t) table) % __alignof__ (table[0]) == 0);
  assert (((uintptr_t) weights) % __alignof__ (weights[0]) == 0);
  assert (((uintptr_t) extra) % __alignof__ (extra[0]) == 0);
  assert (((uintptr_t) indirect) % __alignof__ (indirect[0]) == 0);

  /* We need this a few times.  */
  size_t s1len = STRLEN (s1);
  size_t s2len = STRLEN (s2);

  /* Catch empty strings.  */
  if (__glibc_unlikely (s1len == 0) || __glibc_unlikely (s2len == 0))
    return (s1len != 0) - (s2len != 0);

  /* Perform the first pass over the string and while doing this find
     and store the weights for each character.  Since we want this to
     be as fast as possible we are using `alloca' to store the temporary
     values.  But since there is no limit on the length of the string
     we have to use `malloc' if the string is too long.  We should be
     very conservative here.

     Please note that the localedef programs makes sure that `position'
     is not used at the first level.  */

  coll_seq seq1, seq2;
  bool use_malloc = false;
  int result = 0;

  memset (&seq1, 0, sizeof (seq1));
  seq2 = seq1;

  size_t size_max = SIZE_MAX / (sizeof (int32_t) + 1);

  if (MIN (s1len, s2len) > size_max
      || MAX (s1len, s2len) > size_max - MIN (s1len, s2len))
    {
      /* If the strings are long enough to cause overflow in the size request,
         then skip the allocation and proceed with the non-cached routines.  */
    }
  else if (! __libc_use_alloca ((s1len + s2len) * (sizeof (int32_t) + 1)))
    {
      seq1.idxarr = (int32_t *) malloc ((s1len + s2len) * (sizeof (int32_t) + 1));

      /* If we failed to allocate memory, we leave everything as NULL so that
	 we use the nocache version of traversal and comparison functions.  */
      if (seq1.idxarr != NULL)
	{
	  seq2.idxarr = &seq1.idxarr[s1len];
	  seq1.rulearr = (unsigned char *) &seq2.idxarr[s2len];
	  seq2.rulearr = &seq1.rulearr[s1len];
	  use_malloc = true;
	}
    }
  else
    {
      seq1.idxarr = (int32_t *) alloca (s1len * sizeof (int32_t));
      seq2.idxarr = (int32_t *) alloca (s2len * sizeof (int32_t));
      seq1.rulearr = (unsigned char *) alloca (s1len);
      seq2.rulearr = (unsigned char *) alloca (s2len);
    }

  int rule = 0;

  /* Cache values in the first pass and if needed, use them in subsequent
     passes.  */
  for (int pass = 0; pass < nrules; ++pass)
    {
      seq1.idxcnt = 0;
      seq1.idx = 0;
      seq2.idx = 0;
      seq1.backw_stop = ~0ul;
      seq1.backw = ~0ul;
      seq2.idxcnt = 0;
      seq2.backw_stop = ~0ul;
      seq2.backw = ~0ul;

      /* We need the elements of the strings as unsigned values since they
	 are used as indices.  */
      seq1.us = (const USTRING_TYPE *) s1;
      seq2.us = (const USTRING_TYPE *) s2;

      /* We assume that if a rule has defined `position' in one section
	 this is true for all of them.  */
      int position = rulesets[rule * nrules + pass] & sort_position;

      while (1)
	{
	  if (__glibc_unlikely (seq1.idxarr == NULL))
	    {
	      get_next_seq_nocache (&seq1, nrules, rulesets, weights, table,
				    extra, indirect, pass);
	      get_next_seq_nocache (&seq2, nrules, rulesets, weights, table,
				    extra, indirect, pass);
	    }
	  else if (pass == 0)
	    {
	      get_next_seq (&seq1, nrules, rulesets, weights, table, extra,
			    indirect);
	      get_next_seq (&seq2, nrules, rulesets, weights, table, extra,
			    indirect);
	    }
	  else
	    {
	      get_next_seq_cached (&seq1, nrules, pass, rulesets, weights);
	      get_next_seq_cached (&seq2, nrules, pass, rulesets, weights);
	    }

	  /* See whether any or both strings are empty.  */
	  if (seq1.len == 0 || seq2.len == 0)
	    {
	      if (seq1.len == seq2.len)
		/* Both ended.  So far so good, both strings are equal
		   at this level.  */
		break;

	      /* This means one string is shorter than the other.  Find out
		 which one and return an appropriate value.  */
	      result = seq1.len == 0 ? -1 : 1;
	      goto free_and_return;
	    }

	  if (__glibc_unlikely (seq1.idxarr == NULL))
	    result = do_compare_nocache (&seq1, &seq2, position, weights);
	  else
	    result = do_compare (&seq1, &seq2, position, weights);
	  if (result != 0)
	    goto free_and_return;
	}

      if (__glibc_likely (seq1.rulearr != NULL))
	rule = seq1.rulearr[0];
      else
	rule = seq1.rule;
    }

  /* Free the memory if needed.  */
 free_and_return:
  if (use_malloc)
    free (seq1.idxarr);

  return result;
}
libc_hidden_def (STRCOLL)

#ifndef WIDE_CHAR_VERSION
weak_alias (__strcoll_l, strcoll_l)
#endif
