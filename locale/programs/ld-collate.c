/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <endian.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <obstack.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "localeinfo.h"
#include "locales.h"
#include "simple-hash.h"
#include "stringtrans.h"
#include "strlen-hash.h"

/* Uncomment the following line in the production version.  */
/* #define NDEBUG 1 */
#include <assert.h>


#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define SWAPU32(w) \
  (((w) << 24) | (((w) & 0xff00) << 8) | (((w) >> 8) & 0xff00) | ((w) >> 24))


/* What kind of symbols get defined?  */
enum coll_symbol
{
  undefined,
  ellipsis,
  character,
  element,
  symbol
};


typedef struct patch_t
{
  const char *fname;
  size_t lineno;
  const char *token;
  union
  {
    unsigned int *pos;
    size_t idx;
  } where;
  struct patch_t *next;
} patch_t;


typedef struct element_t
{
  const wchar_t *name;
  unsigned int this_weight;

  struct element_t *next;

  unsigned int *ordering;
  size_t ordering_len;
} element_t;


/* The real definition of the struct for the LC_COLLATE locale.  */
struct locale_collate_t
{
  /* Collate symbol table.  Simple mapping to number.  */
  hash_table symbols;

  /* The collation elements.  */
  hash_table elements;
  struct obstack element_mem;

  /* The result table.  */
  hash_table result;

  /* Sorting rules given in order_start line.  */
  u_int32_t nrules;
  u_int32_t nrules_max;
  enum coll_sort_rule *rules;

  /* Used while recognizing symbol composed of multiple tokens
     (collating-element).  */
  const char *combine_token;
  size_t combine_token_len;

  /* How many sorting order specifications so far.  */
  unsigned int order_cnt;

  /* Was lastline ellipsis?  */
  int was_ellipsis;
  /* Value of last entry if was character.  */
  wchar_t last_char;
  /* Current element.  */
  element_t *current_element;
  /* What kind of symbol is current element.  */
  enum coll_symbol kind;

  /* While collecting the weights we need some temporary space.  */
  unsigned int current_order;
  int *weight_cnt;
  unsigned int weight_idx;
  unsigned int *weight;
  size_t nweight;
  size_t nweight_max;

  /* Patch lists.  */
  patch_t *current_patch;
  patch_t *all_patches;

  /* Room for the UNDEFINED information.  */
  element_t undefined;
  unsigned int undefined_len;
};


/* Be verbose?  Defined in localedef.c.  */
extern int verbose;


void *xmalloc (size_t __n);
void *xrealloc (void *__p, size_t __n);


#define obstack_chunk_alloc malloc
#define obstack_chunk_free free


void
collate_startup (struct linereader *lr, struct localedef_t *locale,
		 struct charset_t *charset)
{
  struct locale_collate_t *collate;

  /* It is important that we always use UCS4 encoding for strings now.  */
  encoding_method = ENC_UCS4;

  /* Allocate the needed room.  */
  locale->categories[LC_COLLATE].collate = collate =
    (struct locale_collate_t *) xmalloc (sizeof (struct locale_collate_t));

  /* Allocate hash table for collating elements.  */
  if (init_hash (&collate->elements, 512))
    error (4, 0, _("memory exhausted"));
  collate->combine_token = NULL;
  obstack_init (&collate->element_mem);

  /* Allocate hash table for collating elements.  */
  if (init_hash (&collate->symbols, 64))
    error (4, 0, _("memory exhausted"));

  /* Allocate hash table for result.  */
  if (init_hash (&collate->result, 512))
    error (4, 0, _("memory exhausted"));

  collate->nrules = 0;
  collate->nrules_max = 10;
  collate->rules
    = (enum coll_sort_rule *) xmalloc (collate->nrules_max
				       * sizeof (enum coll_sort_rule));

  collate->order_cnt = 1;	/* The smallest weight is 2.  */

  collate->was_ellipsis = 0;
  collate->last_char = L'\0';	/* 0 because leading ellipsis is allowed.  */

  collate->all_patches = NULL;

  /* This tells us no UNDEFINED entry was found until now.  */
  memset (&collate->undefined, '\0', sizeof (collate->undefined));

  lr->translate_strings = 0;
}


void
collate_finish (struct localedef_t *locale, struct charset_t *charset)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  patch_t *patch;
  size_t cnt;

  /* Patch the constructed table so that forward references are
     correctly filled.  */
  for (patch = collate->all_patches; patch != NULL; patch = patch->next)
    {
      wchar_t wch;
      size_t toklen = strlen (patch->token);
      void *ptmp;
      unsigned int value = 0;

      wch = charset_find_value (&charset->char_table, patch->token, toklen);
      if (wch != ILLEGAL_CHAR_VALUE)
	{
	  element_t *runp;

	  if (find_entry (&collate->result, &wch, sizeof (wchar_t),
			  (void *) &runp) < 0)
	    runp = NULL;
	  for (; runp != NULL; runp = runp->next)
	    if (runp->name[0] == wch && runp->name[1] == L'\0')
	      break;

	  value = runp == NULL ? 0 : runp->this_weight;
	}
      else if (find_entry (&collate->elements, patch->token, toklen, &ptmp)
	       >= 0)
	{
	  value = ((element_t *) ptmp)->this_weight;
	}
      else if (find_entry (&collate->symbols, patch->token, toklen, &ptmp)
	       >= 0)
	{
	  value = (unsigned long int) ptmp;
	}
      else
	value = 0;

      if (value == 0)
	{
	  if (!be_quiet)
	    error_at_line (0, 0, patch->fname, patch->lineno,
			   _("no weight defined for symbol `%s'"),
			   patch->token);
	}
      else
	*patch->where.pos = value;
    }

  /* If no definition for UNDEFINED is given, all characters in the
     given charset must be specified.  */
  if (collate->undefined.ordering == NULL)
    {
      /**************************************************************\
      |* XXX We should test whether really an unspecified character *|
      |* exists before giving the message.			    *|
      \**************************************************************/
      u_int32_t weight;

      if (!be_quiet)
	error (0, 0, _("no definition of `UNDEFINED'"));

      collate->undefined.ordering_len = collate->nrules;
      weight = ++collate->order_cnt;

      for (cnt = 0; cnt < collate->nrules; ++cnt)
	{
	  u_int32_t one = 1;
	  obstack_grow (&collate->element_mem, &one, sizeof (one));
	}

      for (cnt = 0; cnt < collate->nrules; ++cnt)
	obstack_grow (&collate->element_mem, &weight, sizeof (weight));

      collate->undefined.ordering = obstack_finish (&collate->element_mem);
    }

  collate->undefined_len = 2;	/* For the name: 1 x wchar_t + L'\0'.  */
  for (cnt = 0; cnt < collate->nrules; ++cnt)
    collate->undefined_len += 1 + collate->undefined.ordering[cnt];
}



void
collate_output (struct localedef_t *locale, struct charset_t *charset,
		const char *output_path)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  u_int32_t table_size, table_best, level_best, sum_best;
  void *last;
  element_t *pelem;
  wchar_t *name;
  size_t len;
  const size_t nelems = _NL_ITEM_INDEX (_NL_NUM_LC_COLLATE);
  struct iovec iov[2 + nelems];
  struct locale_file data;
  u_int32_t idx[nelems];
  struct obstack non_simple;
  struct obstack string_pool;
  size_t cnt, entry_size;
  u_int32_t undefined_offset = UINT_MAX;
  u_int32_t *table, *extra, *table2, *extra2;
  size_t extra_len;
  u_int32_t element_hash_tab_size;
  u_int32_t *element_hash_tab;
  u_int32_t *element_hash_tab_ob;
  u_int32_t element_string_pool_size;
  char *element_string_pool;
  u_int32_t element_value_size;
  wchar_t *element_value;
  wchar_t *element_value_ob;
  u_int32_t symbols_hash_tab_size;
  u_int32_t *symbols_hash_tab;
  u_int32_t *symbols_hash_tab_ob;
  u_int32_t symbols_string_pool_size;
  char *symbols_string_pool;
  u_int32_t symbols_class_size;
  u_int32_t *symbols_class;
  u_int32_t *symbols_class_ob;
  hash_table *hash_tab;
  unsigned int dummy_weights[collate->nrules + 1];

  sum_best = UINT_MAX;
  table_best = 0xffff;
  level_best = 0xffff;

  /* Compute table size.  */
  if (!be_quiet)
    fputs (_("\
Computing table size for collation information might take a while..."),
	   stderr);
  for (table_size = 256; table_size < sum_best; ++table_size)
    {
      size_t hits[table_size];
      unsigned int worst = 1;
      size_t cnt;

      last = NULL;

      for (cnt = 0; cnt < 256; ++cnt)
	hits[cnt] = 1;
      memset (&hits[256], '\0', sizeof (hits) - 256 * sizeof (size_t));

      while (iterate_table (&collate->result, &last, (const void **) &name,
			    &len, (void **) &pelem) >= 0)
	if (pelem->ordering != NULL && pelem->name[0] > 0xff)
	  if (++hits[(unsigned int) pelem->name[0] % table_size] > worst)
	    {
	      worst = hits[(unsigned int) pelem->name[0] % table_size];
	      if (table_size * worst > sum_best)
		break;
	    }

      if (table_size * worst < sum_best)
	{
	  sum_best = table_size * worst;
	  table_best = table_size;
	  level_best = worst;
	}
    }
  assert (table_best != 0xffff || level_best != 0xffff);
  if (!be_quiet)
    fputs (_(" done\n"), stderr);

  obstack_init (&non_simple);
  obstack_init (&string_pool);

  data.magic = LIMAGIC (LC_COLLATE);
  data.n = nelems;
  iov[0].iov_base = (void *) &data;
  iov[0].iov_len = sizeof (data);

  iov[1].iov_base = (void *) idx;
  iov[1].iov_len = sizeof (idx);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_NRULES)].iov_base = &collate->nrules;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_NRULES)].iov_len = sizeof (u_int32_t);

  table = (u_int32_t *) alloca (collate->nrules * sizeof (u_int32_t));
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_RULES)].iov_base = table;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_RULES)].iov_len
    = collate->nrules * sizeof (u_int32_t);
  /* Another trick here.  Describing the collation method needs only a
     few bits (3, to be exact).  But the binary file should be
     accessible by machines with both endianesses and so we store both
     forms in the same word.  */
  for (cnt = 0; cnt < collate->nrules; ++cnt)
    table[cnt] = collate->rules[cnt] | SWAPU32 (collate->rules[cnt]);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_HASH_SIZE)].iov_base = &table_best;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_HASH_SIZE)].iov_len = sizeof (u_int32_t);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_HASH_LAYERS)].iov_base = &level_best;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_HASH_LAYERS)].iov_len
    = sizeof (u_int32_t);

  entry_size = 1 + MAX (collate->nrules, 2);

  table = (u_int32_t *) alloca (table_best * level_best * entry_size
				* sizeof (table[0]));
  memset (table, '\0', table_best * level_best * entry_size
	  * sizeof (table[0]));


  /* Macros for inserting in output table.  */
#define ADD_VALUE(expr)							      \
  do {									      \
    u_int32_t to_write = (u_int32_t) expr;				      \
    obstack_grow (&non_simple, &to_write, sizeof (to_write));		      \
  } while (0)

#define ADD_ELEMENT(pelem, len)						      \
  do {									      \
    size_t cnt, idx;							      \
									      \
    ADD_VALUE (len);							      \
									      \
    wlen = wcslen (pelem->name);					      \
    obstack_grow (&non_simple, pelem->name, (wlen + 1) * sizeof (u_int32_t)); \
									      \
    idx = collate->nrules;						      \
    for (cnt = 0; cnt < collate->nrules; ++cnt)				      \
      {									      \
	size_t disp;							      \
									      \
	ADD_VALUE (pelem->ordering[cnt]);				      \
	for (disp = 0; disp < pelem->ordering[cnt]; ++disp)		      \
	  ADD_VALUE (pelem->ordering[idx++]);				      \
      }									      \
  } while (0)

#define ADD_FORWARD(pelem)						      \
  do {									      \
    /* We leave a reference in the main table and put all		      \
       information in the table for the extended entries.  */		      \
    element_t *runp;							      \
    element_t *has_simple = NULL;					      \
    size_t wlen;							      \
									      \
    table[(level * table_best + slot) * entry_size + 1]			      \
      = FORWARD_CHAR;							      \
    table[(level * table_best + slot) * entry_size + 2]			      \
      = obstack_object_size (&non_simple) / sizeof (u_int32_t);		      \
									      \
    /* Here we have to construct the non-simple table entry.  First	      \
       compute the total length of this entry.  */			      \
    for (runp = (pelem); runp != NULL; runp = runp->next)		      \
      if (runp->ordering != NULL)					      \
	{								      \
	  u_int32_t value;						      \
	  size_t cnt;							      \
									      \
	  value = 1 + wcslen (runp->name) + 1;				      \
									      \
	  for (cnt = 0; cnt < collate->nrules; ++cnt)			      \
	    /* We have to take care for entries without ordering	      \
	       information.  While reading them they get inserted in the      \
	       table and later not removed when something goes wrong with     \
	       reading its weights.  */					      \
	    value += 1 + runp->ordering[cnt];				      \
									      \
	  if (runp->name[1] == L'\0')					      \
	    has_simple = runp;						      \
									      \
	  ADD_ELEMENT (runp, value);					      \
	}								      \
									      \
    if (has_simple == NULL)						      \
      {									      \
	size_t idx, cnt;						      \
									      \
	ADD_VALUE (collate->undefined_len + 1);				      \
									      \
	/* Add the name.  */						      \
	ADD_VALUE ((pelem)->name[0]);					      \
	ADD_VALUE (0);							      \
									      \
	idx = collate->nrules;						      \
	for (cnt = 0; cnt < collate->nrules; ++cnt)			      \
	  {								      \
	    size_t disp;						      \
									      \
	    ADD_VALUE (collate->undefined.ordering[cnt]);		      \
	    for (disp = 0; disp < collate->undefined.ordering[cnt]; ++disp)   \
	      {								      \
		if ((wchar_t) collate->undefined.ordering[idx]		      \
		    == ELLIPSIS_CHAR)					      \
		  ADD_VALUE ((pelem)->name[0]);				      \
		else							      \
		  ADD_VALUE (collate->undefined.ordering[idx++]);	      \
		++idx;							      \
	      }								      \
	  }								      \
      }									      \
  } while (0)



  /* Fill the table now.  First we look for all the characters which
     fit into one single byte.  This speeds up the 8-bit string
     functions.  */
  last = NULL;
  while (iterate_table (&collate->result, &last, (const void **) &name,
			&len, (void **) &pelem) >= 0)
    if (pelem->name[0] <= 0xff)
      {
	/* We have a single byte name.  Now we must distinguish
	   between entries in simple form (i.e., only one value per
	   weight and no collation element starting with the same
	   character) and those which are not.  */
	size_t slot = ((size_t) pelem->name[0]);
	const size_t level = 0;

	table[slot * entry_size] = pelem->name[0];

	if (pelem->name[1] == L'\0' && pelem->next == NULL
	    && pelem->ordering_len == collate->nrules)
	  {
	    /* Yes, we have a simple one.  Lucky us.  */
	    size_t cnt;

	    for (cnt = 0; cnt < collate->nrules; ++cnt)
	      table[slot * entry_size + 1 + cnt]
		= pelem->ordering[collate->nrules + cnt];
	  }
	else
	  ADD_FORWARD (pelem);
      }

  /* Now check for missing single byte entries.  If one exist we fill
     with the UNDEFINED entry.  */
  for (cnt = 0; cnt < 256; ++cnt)
    /* The first weight is never 0 for existing entries.  */
    if (table[cnt * entry_size + 1] == 0)
      {
	/* We have to fill in the information from the UNDEFINED
	   entry.  */
	table[cnt * entry_size] = (u_int32_t) cnt;

	if (collate->undefined.ordering_len == collate->nrules)
	  {
	    size_t inner;

	    for (inner = 0; inner < collate->nrules; ++inner)
	      if ((wchar_t)collate->undefined.ordering[collate->nrules + inner]
		  == ELLIPSIS_CHAR)
		table[cnt * entry_size + 1 + inner] = cnt;
	      else
		table[cnt * entry_size + 1 + inner]
		  = collate->undefined.ordering[collate->nrules + inner];
	  }
	else
	  {
	    if (undefined_offset != UINT_MAX)
	      {
		table[cnt * entry_size + 1] = FORWARD_CHAR;
		table[cnt * entry_size + 2] = undefined_offset;
	      }
	    else
	      {
		const size_t slot = cnt;
		const size_t level = 0;

		ADD_FORWARD (&collate->undefined);
		undefined_offset = table[cnt * entry_size + 2];
	      }
	  }
      }

  /* Now we are ready for inserting the whole rest.   */
  last = NULL;
  while (iterate_table (&collate->result, &last, (const void **) &name,
			&len, (void **) &pelem) >= 0)
    if (pelem->name[0] > 0xff)
      {
	/* Find the position.  */
	size_t slot = ((size_t) pelem->name[0]) % table_best;
	size_t level = 0;

	while (table[(level * table_best + slot) * entry_size + 1] != 0)
	  ++level;
	assert (level < level_best);

	if (pelem->name[1] == L'\0' && pelem->next == NULL
	    && pelem->ordering_len == collate->nrules)
	  {
	    /* Again a simple entry.  */
	    size_t inner;

	    for (inner = 0; inner < collate->nrules; ++inner)
	      table[(level * table_best + slot) * entry_size + 1 + inner]
		= pelem->ordering[collate->nrules + inner];
	  }
	else
	  ADD_FORWARD (pelem);
      }

  /* Add the UNDEFINED entry.  */
  {
    /* Here we have to construct the non-simple table entry.  */
    size_t idx, cnt;

    undefined_offset = obstack_object_size (&non_simple);

    idx = collate->nrules;
    for (cnt = 0; cnt < collate->nrules; ++cnt)
      {
	size_t disp;

	ADD_VALUE (collate->undefined.ordering[cnt]);
	for (disp = 0; disp < collate->undefined.ordering[cnt]; ++disp)
	  ADD_VALUE (collate->undefined.ordering[idx++]);
      }
  }

  /* Finish the extra block.  */
  extra_len = obstack_object_size (&non_simple);
  extra = (u_int32_t *) obstack_finish (&non_simple);
  assert ((extra_len % sizeof (u_int32_t)) == 0);

  /* Now we have to build the two array for the other byte ordering.  */
  table2 = (u_int32_t *) alloca (table_best * level_best * entry_size
				 * sizeof (table[0]));
  extra2 = (u_int32_t *) alloca (extra_len);

  for (cnt = 0; cnt < table_best * level_best * entry_size; ++cnt)
    table2[cnt] = SWAPU32 (table[cnt]);

  for (cnt = 0; cnt < extra_len / sizeof (u_int32_t); ++cnt)
    extra2[cnt] = SWAPU32 (extra2[cnt]);

  /* We need a simple hashing table to get a collation-element->chars
     mapping.  We again use internal hashing using a secondary hashing
     function.

     Each string has an associate hashing value V, computed by a
     fixed function.  To locate the string we use open addressing with
     double hashing.  The first index will be V % M, where M is the
     size of the hashing table.  If no entry is found, iterating with
     a second, independent hashing function takes place.  This second
     value will be 1 + V % (M - 2).  The approximate number of probes
     will be

	  for unsuccessful search: (1 - N / M) ^ -1
	  for successful search:   - (N / M) ^ -1 * ln (1 - N / M)

     where N is the number of keys.

     If we now choose M to be the next prime bigger than 4 / 3 * N,
     we get the values 4 and 1.85 resp.  Because unsuccessful searches
     are unlikely this is a good value.  Formulas: [Knuth, The Art of
     Computer Programming, Volume 3, Sorting and Searching, 1973,
     Addison Wesley]  */
  if (collate->elements.filled == 0)
    {
      /* We don't need any element table since there are no collating
	 elements.  */
      element_hash_tab_size = 0;
      element_hash_tab = NULL;
      element_hash_tab_ob = NULL;
      element_string_pool_size = 0;
      element_string_pool = NULL;
      element_value_size = 0;
      element_value = NULL;
      element_value_ob = NULL;
    }
  else
    {
      void *ptr;		/* Running pointer.  */
      const char *key;		/* Key for current bucket.  */
      size_t keylen;		/* Length of key data.  */
      const element_t *data;	/* Data, i.e., the character sequence.  */

      element_hash_tab_size = next_prime ((collate->elements.filled * 4) / 3);
      if (element_hash_tab_size < 7)
	/* We need a minimum to make the following code work.  */
	element_hash_tab_size = 7;

      element_hash_tab = obstack_alloc (&non_simple, (2 * element_hash_tab_size
						      * sizeof (u_int32_t)));
      memset (element_hash_tab, '\377', (2 * element_hash_tab_size
					 * sizeof (u_int32_t)));

      ptr = NULL;
      while (iterate_table (&collate->elements, &ptr, (const void **) &key,
			    &keylen, (void **) &data) == 0)
	{
	  size_t hash_val = hash_string (key, keylen);
	  size_t idx = hash_val % element_hash_tab_size;

	  if (element_hash_tab[2 * idx] != (~((u_int32_t) 0)))
	    {
	      /* We need the second hashing function.  */
	      size_t c = 1 + (hash_val % (element_hash_tab_size - 2));

	      do
		if (idx >= element_hash_tab_size - c)
		  idx -= element_hash_tab_size - c;
		else
		  idx += c;
	      while (element_hash_tab[2 * idx] != (~((u_int32_t) 0)));
	    }

	  element_hash_tab[2 * idx] = obstack_object_size (&non_simple);
	  element_hash_tab[2 * idx + 1] = (obstack_object_size (&string_pool)
					   / sizeof (wchar_t));

	  obstack_grow0 (&non_simple, key, keylen);
	  obstack_grow (&string_pool, data->name,
			(wcslen (data->name) + 1) * sizeof (wchar_t));
	}

      if (obstack_object_size (&non_simple) % 4 != 0)
	obstack_blank (&non_simple,
		       4 - (obstack_object_size (&non_simple) % 4));
      element_string_pool_size = obstack_object_size (&non_simple);
      element_string_pool = obstack_finish (&non_simple);

      element_value_size = obstack_object_size (&string_pool);
      element_value = obstack_finish (&string_pool);

      /* Create the tables for the other byte order.  */
      element_hash_tab_ob = obstack_alloc (&non_simple,
					   (2 * element_hash_tab_size
					    * sizeof (u_int32_t)));
      for (cnt = 0; cnt < 2 * element_hash_tab_size; ++cnt)
	element_hash_tab_ob[cnt] = SWAPU32 (element_hash_tab[cnt]);

      element_value_ob = obstack_alloc (&string_pool, element_value_size);
      if (sizeof (wchar_t) != 4)
	{
	  fputs ("sizeof (wchar_t) != 4 currently not handled", stderr);
	  abort ();
	}
      for (cnt = 0; cnt < element_value_size / 4; ++cnt)
	element_value_ob[cnt] = SWAPU32 (element_value[cnt]);
    }

  /* Store collation elements as map to collation class.  There are
     three kinds of symbols:
       - simple characters
       - collation elements
       - collation symbols
     We need to make a table which lets the user to access the primary
     weight based on the symbol string.  */
  symbols_hash_tab_size = next_prime ((4 * (charset->char_table.filled
					    + collate->elements.filled
					    + collate->symbols.filled)) / 3);
  symbols_hash_tab = obstack_alloc (&non_simple, (2 * symbols_hash_tab_size
						  * sizeof (u_int32_t)));
  memset (symbols_hash_tab, '\377', (2 * symbols_hash_tab_size
				     * sizeof (u_int32_t)));

  /* Now fill the array.  First the symbols from the character set,
     then the collation elements and last the collation symbols.  */
  hash_tab = &charset->char_table;
  while (1)
    {
      void *ptr;	/* Running pointer.  */
      const char *key;	/* Key for current bucket.  */
      size_t keylen;	/* Length of key data.  */
      void *data;	/* Data.  */

      ptr = NULL;
      while (iterate_table (hash_tab, &ptr, (const void **) &key,
			    &keylen, (void **) &data) == 0)
	{
	  size_t hash_val;
	  size_t idx;
	  u_int32_t word;
	  unsigned int *weights;

	  if (hash_tab == &charset->char_table
	      || hash_tab == &collate->elements)
	    {
	      element_t *lastp, *firstp;
	      wchar_t dummy_name[2];
	      const wchar_t *name;
	      size_t name_len;

	      if (hash_tab == &charset->char_table)
		{
		  dummy_name[0] = (wchar_t) ((unsigned long int) data);
		  dummy_name[1] = L'\0';
		  name = dummy_name;
		  name_len = sizeof (wchar_t);
		}
	      else
		{
		  element_t *elemp = (element_t *) data;
		  name = elemp->name;
		  name_len = wcslen (name) * sizeof (wchar_t);
		}

	      /* First check whether this character is used at all.  */
	      if (find_entry (&collate->result, name, name_len,
			      (void *) &firstp) < 0)
		/* The symbol is not directly mentioned in the collation.
		   I.e., we use the value for UNDEFINED.  */
		lastp = &collate->undefined;
	      else
		{
		  /* The entry for the simple character is always found at
		     the end.  */
		  lastp = firstp;
		  while (lastp->next != NULL && wcscmp (name, lastp->name))
		    lastp = lastp->next;
		}

	      weights = lastp->ordering;
	    }
	  else
	    {
	      dummy_weights[0] = 1;
	      dummy_weights[collate->nrules]
		= (unsigned int) ((unsigned long int) data);

	      weights = dummy_weights;
	    }

	  /* In LASTP->ordering we now have the collation class.
	     Determine the place in the hashing table next.  */
	  hash_val = hash_string (key, keylen);
	  idx = hash_val % symbols_hash_tab_size;

	  if (symbols_hash_tab[2 * idx] != (~((u_int32_t) 0)))
	    {
	      /* We need the second hashing function.  */
	      size_t c = 1 + (hash_val % (symbols_hash_tab_size - 2));

	      do
		if (idx >= symbols_hash_tab_size - c)
		  idx -= symbols_hash_tab_size - c;
		else
		  idx += c;
	      while (symbols_hash_tab[2 * idx] != (~((u_int32_t) 0)));
	    }

	  symbols_hash_tab[2 * idx] = obstack_object_size (&string_pool);
	  symbols_hash_tab[2 * idx + 1] = (obstack_object_size (&non_simple)
					   / sizeof (u_int32_t));

	  obstack_grow0 (&string_pool, key, keylen);
	  /* Adding the first weight looks complicated.  We have to deal
	     with the kind it is stored and with the fact that original
	     form uses `unsigned int's while we need `u_int32_t' here.  */
	  word = weights[0];
	  obstack_grow (&non_simple, &word, sizeof (u_int32_t));
	  for (cnt = 0; cnt < weights[0]; ++cnt)
	    {
	      word = weights[collate->nrules + cnt];
	      obstack_grow (&non_simple, &word, sizeof (u_int32_t));
	    }
	}

      if (hash_tab == &charset->char_table)
	hash_tab = &collate->elements;
      else if (hash_tab == &collate->elements)
	hash_tab = &collate->symbols;
      else
	break;
    }

  /* Now we have the complete tables.  */
  if (obstack_object_size (&string_pool) % 4 != 0)
    obstack_blank (&non_simple, 4 - (obstack_object_size (&string_pool) % 4));
  symbols_string_pool_size = obstack_object_size (&string_pool);
  symbols_string_pool = obstack_finish (&string_pool);

  symbols_class_size = obstack_object_size (&non_simple);
  symbols_class = obstack_finish (&non_simple);

  /* Generate tables with other byte order.  */
  symbols_hash_tab_ob = obstack_alloc (&non_simple, (2 * symbols_hash_tab_size
						     * sizeof (u_int32_t)));
  for (cnt = 0; cnt < 2 * symbols_hash_tab_size; ++cnt)
    symbols_hash_tab_ob[cnt] = SWAPU32 (symbols_hash_tab[cnt]);

  symbols_class_ob = obstack_alloc (&non_simple, symbols_class_size);
  for (cnt = 0; cnt < symbols_class_size / 4; ++cnt)
    symbols_class_ob[cnt] = SWAPU32 (symbols_class[cnt]);


  /* Store table addresses and lengths.   */
#if __BYTE_ORDER == __BIG_ENDIAN
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EB)].iov_base = table;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EB)].iov_len
    = table_best * level_best * entry_size * sizeof (table[0]);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EL)].iov_base = table2;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EL)].iov_len
    = table_best * level_best * entry_size * sizeof (table[0]);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EB)].iov_base = extra;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EB)].iov_len = extra_len;

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EL)].iov_base = extra2;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EL)].iov_len = extra_len;
#else
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EB)].iov_base = table2;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EB)].iov_len
    = table_best * level_best * entry_size * sizeof (table[0]);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EL)].iov_base = table;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_TABLE_EL)].iov_len
    = table_best * level_best * entry_size * sizeof (table[0]);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EB)].iov_base = extra2;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EB)].iov_len = extra_len;

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EL)].iov_base = extra;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_EXTRA_EL)].iov_len = extra_len;
#endif

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_UNDEFINED)].iov_base = &undefined_offset;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_UNDEFINED)].iov_len = sizeof (u_int32_t);


  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_SIZE)].iov_base
    = &element_hash_tab_size;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_SIZE)].iov_len
    = sizeof (u_int32_t);

#if __BYTE_ORDER == __BIG_ENDIAN
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EB)].iov_base
    = element_hash_tab;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EB)].iov_len
    = 2 * element_hash_tab_size * sizeof (u_int32_t);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EL)].iov_base
    = element_hash_tab_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EL)].iov_len
    = 2 * element_hash_tab_size * sizeof (u_int32_t);
#else
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EL)].iov_base
    = element_hash_tab;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EL)].iov_len
    = 2 * element_hash_tab_size * sizeof (u_int32_t);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EB)].iov_base
    = element_hash_tab_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_HASH_EB)].iov_len
    = 2 * element_hash_tab_size * sizeof (u_int32_t);
#endif

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_STR_POOL)].iov_base
    = element_string_pool;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_STR_POOL)].iov_len
    = element_string_pool_size;

#if __BYTE_ORDER == __BIG_ENDIAN
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EB)].iov_base
    = element_value;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EB)].iov_len
    = element_value_size;

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EL)].iov_base
    = element_value_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EL)].iov_len
    = element_value_size;
#else
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EL)].iov_base
    = element_value;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EL)].iov_len
    = element_value_size;

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EB)].iov_base
    = element_value_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_ELEM_VAL_EB)].iov_len
    = element_value_size;
#endif

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_SIZE)].iov_base
    = &symbols_hash_tab_size;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_SIZE)].iov_len
    = sizeof (u_int32_t);

#if __BYTE_ORDER == __BIG_ENDIAN
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EB)].iov_base
    = symbols_hash_tab;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EB)].iov_len
    = 2 * symbols_hash_tab_size * sizeof (u_int32_t);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EL)].iov_base
    = symbols_hash_tab_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EL)].iov_len
    = 2 * symbols_hash_tab_size * sizeof (u_int32_t);
#else
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EL)].iov_base
    = symbols_hash_tab;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EL)].iov_len
    = 2 * symbols_hash_tab_size * sizeof (u_int32_t);

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EB)].iov_base
    = symbols_hash_tab_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_HASH_EB)].iov_len
    = 2 * symbols_hash_tab_size * sizeof (u_int32_t);
#endif

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_STR_POOL)].iov_base
    = symbols_string_pool;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_STR_POOL)].iov_len
    = symbols_string_pool_size;

#if __BYTE_ORDER == __BIG_ENDIAN
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EB)].iov_base
    = symbols_class;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EB)].iov_len
    = symbols_class_size;

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EL)].iov_base
    = symbols_class_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EL)].iov_len
    = symbols_class_size;
#else
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EL)].iov_base
    = symbols_class;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EL)].iov_len
    = symbols_class_size;

  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EB)].iov_base
    = symbols_class_ob;
  iov[2 + _NL_ITEM_INDEX (_NL_COLLATE_SYMB_CLASS_EB)].iov_len
    = symbols_class_size;
#endif

  /* Update idx array.  */
  idx[0] = iov[0].iov_len + iov[1].iov_len;
  for (cnt = 1; cnt < nelems; ++cnt)
    idx[cnt] = idx[cnt - 1] + iov[1 + cnt].iov_len;

  write_locale_data (output_path, "LC_COLLATE", 2 + nelems, iov);

  obstack_free (&non_simple, NULL);
  obstack_free (&string_pool, NULL);
}


void
collate_element_to (struct linereader *lr, struct localedef_t *locale,
		    struct token *code, struct charset_t *charset)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  unsigned int value;
  void *not_used;

  if (collate->combine_token != NULL)
    {
      free ((void *) collate->combine_token);
      collate->combine_token = NULL;
    }

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);
  if ((wchar_t) value != ILLEGAL_CHAR_VALUE)
    {
      lr_error (lr, _("symbol for multicharacter collating element "
		      "`%.*s' duplicates symbolic name in charset"),
		(int) code->val.str.len, code->val.str.start);
      return;
    }

  if (find_entry (&collate->elements, code->val.str.start, code->val.str.len,
		  &not_used) >= 0)
    {
      lr_error (lr, _("symbol for multicharacter collating element "
		      "`%.*s' duplicates other element definition"),
		(int) code->val.str.len, code->val.str.start);
      return;
    }

  if (find_entry (&collate->elements, code->val.str.start, code->val.str.len,
		  &not_used) >= 0)
    {
      lr_error (lr, _("symbol for multicharacter collating element "
		      "`%.*s' duplicates symbol definition"),
		(int) code->val.str.len, code->val.str.start);
      return;
    }

  collate->combine_token = code->val.str.start;
  collate->combine_token_len = code->val.str.len;
}


void
collate_element_from (struct linereader *lr, struct localedef_t *locale,
		      struct token *code, struct charset_t *charset)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  element_t *elemp, *runp;

  /* CODE is a string.  */
  elemp = (element_t *) obstack_alloc (&collate->element_mem,
				       sizeof (element_t));

  /* We have to translate the string.  It may contain <...> character
     names.  */
  elemp->name = (wchar_t *) translate_string (code->val.str.start, charset);
  elemp->this_weight = 0;
  elemp->ordering = NULL;
  elemp->ordering_len = 0;

  free (code->val.str.start);

  if (elemp->name == NULL)
    {
      /* At least one character in the string is not defined.  We simply
	 do nothing.  */
      if (verbose)
	lr_error (lr, _("\
`from' string in collation element declaration contains unknown character"));
      return;
    }

  if (elemp->name[0] == L'\0' || elemp->name[1] == L'\0')
    {
      lr_error (lr, _("illegal collation element"));
      return;
    }

  /* The entries in the linked lists of RESULT are sorting in
     descending order.  The order is important for the `strcoll' and
     `wcscoll' functions.  */
  if (find_entry (&collate->result, elemp->name, sizeof (wchar_t),
		  (void *) &runp) >= 0)
    {
      /* We already have an entry with this key.  Check whether it is
	 identical.  */
      element_t *prevp = NULL;
      int cmpres;

      do
	{
	  cmpres = wcscmp (elemp->name, runp->name);
	  if (cmpres <= 0)
	    break;
	  prevp = runp;
	}
      while ((runp = runp->next) != NULL);

      if (cmpres == 0)
	lr_error (lr, _("duplicate collating element definition"));
      else
	{
	  elemp->next = runp;
	  if (prevp == NULL)
	    {
	      if (set_entry (&collate->result, elemp->name, sizeof (wchar_t),
			     elemp) < 0)
		error (EXIT_FAILURE, 0, _("\
error while inserting collation element into hash table"));
	    }
	  else
	    prevp->next = elemp;
	}
    }
  else
    {
      elemp->next = NULL;
      if (insert_entry (&collate->result, elemp->name, sizeof (wchar_t), elemp)
	  < 0)
	error (EXIT_FAILURE, errno, _("error while inserting to hash table"));
    }

  if (insert_entry (&collate->elements, collate->combine_token,
		    collate->combine_token_len, (void *) elemp) < 0)
    lr_error (lr, _("cannot insert new collating symbol definition: %s"),
	      strerror (errno));
}


void
collate_symbol (struct linereader *lr, struct localedef_t *locale,
		struct token *code, struct charset_t *charset)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  wchar_t value;
  void *not_used;

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);
  if (value != ILLEGAL_CHAR_VALUE)
    {
      lr_error (lr, _("symbol for multicharacter collating element "
		      "`%.*s' duplicates symbolic name in charset"),
		(int) code->val.str.len, code->val.str.start);
      return;
    }

  if (find_entry (&collate->elements, code->val.str.start, code->val.str.len,
		  &not_used) >= 0)
    {
      lr_error (lr, _("symbol for multicharacter collating element "
		      "`%.*s' duplicates element definition"),
		(int) code->val.str.len, code->val.str.start);
      return;
    }

  if (find_entry (&collate->symbols, code->val.str.start, code->val.str.len,
		  &not_used) >= 0)
    {
      lr_error (lr, _("symbol for multicharacter collating element "
		      "`%.*s' duplicates other symbol definition"),
		(int) code->val.str.len, code->val.str.start);
      return;
    }

  if (insert_entry (&collate->symbols, code->val.str.start, code->val.str.len,
		    (void *) 0) < 0)
    lr_error (lr, _("cannot insert new collating symbol definition: %s"),
	      strerror (errno));
}


void
collate_new_order (struct linereader *lr, struct localedef_t *locale,
		   enum coll_sort_rule sort_rule)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;

  if (collate->nrules >= collate->nrules_max)
    {
      collate->nrules_max *= 2;
      collate->rules
	= (enum coll_sort_rule *) xrealloc (collate->rules,
					    collate->nrules_max
					    * sizeof (enum coll_sort_rule));
    }

  collate->rules[collate->nrules++] = sort_rule;
}


void
collate_build_arrays (struct linereader *lr, struct localedef_t *locale)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;

  collate->rules
    = (enum coll_sort_rule *) xrealloc (collate->rules,
					collate->nrules
					* sizeof (enum coll_sort_rule));

  /* Allocate arrays for temporary weights.  */
  collate->weight_cnt = (int *) xmalloc (collate->nrules * sizeof (int));

  /* Choose arbitrary start value for table size.  */
  collate->nweight_max = 5 * collate->nrules;
  collate->weight = (int *) xmalloc (collate->nweight_max * sizeof (int));
}


int
collate_order_elem (struct linereader *lr, struct localedef_t *locale,
		    struct token *code, struct charset_t *charset)
{
  const wchar_t zero = L'\0';
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  int result = 0;
  wchar_t value;
  void *tmp;
  unsigned int i;

  switch (code->tok)
    {
    case tok_bsymbol:
      /* We have a string to find in one of the three hashing tables.  */
      value = charset_find_value (&charset->char_table, code->val.str.start,
				  code->val.str.len);
      if (value != ILLEGAL_CHAR_VALUE)
	{
	  element_t *lastp, *firstp;

	  collate->kind = character;

	  if (find_entry (&collate->result, &value, sizeof (wchar_t),
			  (void *) &firstp) < 0)
	    firstp = lastp = NULL;
	  else
	    {
	      /* The entry for the simple character is always found at
		 the end.  */
	      lastp = firstp;
	      while (lastp->next != NULL)
		lastp = lastp->next;

	      if (lastp->name[0] == value && lastp->name[1] == L'\0')
		{
		  lr_error (lr, _("duplicate definition for character `%.*s'"),
			    (int) code->val.str.len, code->val.str.start);
		  lr_ignore_rest (lr, 0);
		  result = -1;
		  break;
		}
	    }

	  collate->current_element
	    = (element_t *) obstack_alloc (&collate->element_mem,
					   sizeof (element_t));

	  obstack_grow (&collate->element_mem, &value, sizeof (value));
	  obstack_grow (&collate->element_mem, &zero, sizeof (zero));

	  collate->current_element->name =
	    (const wchar_t *) obstack_finish (&collate->element_mem);

	  collate->current_element->this_weight = ++collate->order_cnt;

	  collate->current_element->next = NULL;

	  if (firstp == NULL)
	    {
	      if (insert_entry (&collate->result, &value, sizeof (wchar_t),
				(void *) collate->current_element) < 0)
		{
		  lr_error (lr, _("cannot insert collation element `%.*s'"),
			    (int) code->val.str.len, code->val.str.start);
		  exit (4);
		}
	    }
	  else
	    lastp->next = collate->current_element;
	}
      else if (find_entry (&collate->elements, code->val.str.start,
			   code->val.str.len, &tmp) >= 0)
	{
	  collate->current_element = (element_t *) tmp;

	  if (collate->current_element->this_weight != 0)
	    {
	      lr_error (lr, _("\
collation element `%.*s' appears more than once: ignore line"),
			(int) code->val.str.len, code->val.str.start);
	      lr_ignore_rest (lr, 0);
	      result = -1;
	      break;
	    }

	  collate->kind = element;
	  collate->current_element->this_weight = ++collate->order_cnt;
	}
      else if (find_entry (&collate->symbols, code->val.str.start,
			   code->val.str.len, &tmp) >= 0)
	{
	  unsigned int order = ++collate->order_cnt;

	  if ((unsigned long int) tmp != 0ul)
	    {
	      lr_error (lr, _("\
collation symbol `%.*s' appears more than once: ignore line"),
			(int) code->val.str.len, code->val.str.start);
	      lr_ignore_rest (lr, 0);
	      result = -1;
	      break;
	    }

	  collate->kind = symbol;

	  if (set_entry (&collate->symbols, code->val.str.start,
			 code->val.str.len, (void *) order) < 0)
	    {
	      lr_error (lr, _("cannot process order specification"));
	      exit (4);
	    }
	}
      else
	{
	  if (verbose)
	    lr_error (lr, _("unknown symbol `%.*s': line ignored"),
		      (int) code->val.str.len, code->val.str.start);
          lr_ignore_rest (lr, 0);

          result = -1;
	}
      break;

    case tok_undefined:
      collate->kind = undefined;
      collate->current_element = &collate->undefined;
      break;

    case tok_ellipsis:
      if (collate->was_ellipsis)
	{
	  lr_error (lr, _("\
two lines in a row containing `...' are not allowed"));
	  result = -1;
	}
      else if (collate->kind != character)
	{
	  /* An ellipsis requires the previous line to be an
	     character definition.  */
	  lr_error (lr, _("\
line before ellipsis does not contain definition for character constant"));
	  lr_ignore_rest (lr, 0);
	  result = -1;
	}
      else
	collate->kind = ellipsis;
      break;

    default:
      assert (! "illegal token in `collate_order_elem'");
    }

  /* Now it's time to handle the ellipsis in the previous line.  We do
     this only when the last line contained an definition for a
     character, the current line also defines an character, the
     character code for the later is bigger than the former.  */
  if (collate->was_ellipsis)
    {
      if (collate->kind != character)
	{
	  lr_error (lr, _("\
line after ellipsis must contain character definition"));
	  lr_ignore_rest (lr, 0);
	  result = -1;
	}
      else if (collate->last_char > value)
	{
	  lr_error (lr, _("end point of ellipsis range is bigger then start"));
	  lr_ignore_rest (lr, 0);
	  result = -1;
	}
      else
	{
	  /* We can fill the arrays with the information we need.  */
	  wchar_t name[2];
	  unsigned int *data;
	  size_t *ptr;
	  size_t cnt;

	  name[0] = collate->last_char + 1;
	  name[1] = L'\0';

	  data = (unsigned int *) alloca ((collate->nrules + collate->nweight)
					  * sizeof (unsigned int));
	  ptr = (size_t *) alloca (collate->nrules * sizeof (size_t));

	  if (data == NULL || ptr == NULL)
	    error (4, 0, _("memory exhausted"));

	  /* Prepare data.  Because the characters covered by an
	     ellipsis all have equal values we prepare the data once
	     and only change the variable number (if there are any).
	     PTR[...] will point to the entries which will have to be
	     fixed during the output loop.  */
	  for (cnt = 0; cnt < collate->nrules; ++cnt)
	    {
	      data[cnt] = collate->weight_cnt[cnt];
	      ptr[cnt] = (cnt == 0
			  ? collate->nweight
			  : ptr[cnt - 1] + collate->weight_cnt[cnt - 1]);
	    }

	  for (cnt = 0; cnt < collate->nweight; ++cnt)
	    data[collate->nrules + cnt] = collate->weight[cnt];

	  for (cnt = 0; cnt < collate->nrules; ++cnt)
	    if ((wchar_t) data[ptr[cnt]] != ELLIPSIS_CHAR)
	      ptr[cnt] = 0;

	  while (name[0] <= value)
	    {
	      element_t *pelem;

	      pelem = (element_t *) obstack_alloc (&collate->element_mem,
						   sizeof (element_t));
	      if (pelem == NULL)
		error (4, 0, _("memory exhausted"));

	      pelem->name
		= (const wchar_t *) obstack_copy (&collate->element_mem,
						  name, 2 * sizeof (wchar_t));
	      pelem->this_weight = ++collate->order_cnt;

	      pelem->ordering_len = collate->nweight;
	      pelem->ordering
		= (unsigned int *) obstack_copy (&collate->element_mem, data,
						 (collate->nrules
						  + pelem->ordering_len)
						 * sizeof (unsigned int));

	      /* `...' weights need to be adjusted.  */
	      for (cnt = 0; cnt < collate->nrules; ++cnt)
		if (ptr[cnt] != 0)
		  pelem->ordering[ptr[cnt]] = pelem->this_weight;

	      /* Insert new entry into result table.  */
	      if (find_entry (&collate->result, name, sizeof (wchar_t),
			      (void *) &pelem->next) >= 0)
		{
		  if (set_entry (&collate->result, name, sizeof (wchar_t),
				 (void *) pelem) < 0)
		    error (4, 0, _("cannot insert into result table"));
		}
	      else
		{
		  pelem->next = NULL;
		  if (insert_entry (&collate->result, name, sizeof (wchar_t),
				    (void *) pelem) < 0)
		    error (4, 0, _("cannot insert into result table"));
		}

	      /* Increment counter.  */
	      ++name[0];
	    }
	}
    }

  /* Reset counters for weights.  */
  collate->weight_idx = 0;
  collate->nweight = 0;
  for (i = 0; i < collate->nrules; ++i)
    collate->weight_cnt[i] = 0;
  collate->current_patch = NULL;

  return result;
}


int
collate_weight_bsymbol (struct linereader *lr, struct localedef_t *locale,
			struct token *code, struct charset_t *charset)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  unsigned int here_weight;
  wchar_t value;
  void *tmp;

  assert (code->tok == tok_bsymbol);

  value = charset_find_value (&charset->char_table, code->val.str.start,
			      code->val.str.len);
  if (value != ILLEGAL_CHAR_VALUE)
    {
      element_t *runp;

      if (find_entry (&collate->result, &value, sizeof (wchar_t),
		      (void *)&runp) < 0)
	runp = NULL;

      while (runp != NULL
	     && (runp->name[0] != value || runp->name[1] != L'\0'))
	runp = runp->next;

      here_weight = runp == NULL ? 0 : runp->this_weight;
    }
  else if (find_entry (&collate->elements, code->val.str.start,
		       code->val.str.len, &tmp) >= 0)
    {
      element_t *runp = (element_t *) tmp;

      here_weight = runp->this_weight;
    }
  else if (find_entry (&collate->symbols, code->val.str.start,
		       code->val.str.len, &tmp) >= 0)
    {
      here_weight = (unsigned int) tmp;
    }
  else
    {
      if (verbose)
	lr_error (lr, _("unknown symbol `%.*s': line ignored"),
		  (int) code->val.str.len, code->val.str.start);
      lr_ignore_rest (lr, 0);
      return -1;
    }

  /* When we currently work on a collation symbol we do not expect any
     weight.  */
  if (collate->kind == symbol)
    {
      lr_error (lr, _("\
specification of sorting weight for collation symbol does not make sense"));
      lr_ignore_rest (lr, 0);
      return -1;
    }

  /* Add to the current collection of weights.  */
  if (collate->nweight >= collate->nweight_max)
    {
      collate->nweight_max *= 2;
      collate->weight = (unsigned int *) xrealloc (collate->weight,
						   collate->nweight_max);
    }

  /* If the weight is currently not known, we remember to patch the
     resulting tables.  */
  if (here_weight == 0)
    {
      patch_t *newp;

      newp = (patch_t *) obstack_alloc (&collate->element_mem,
					sizeof (patch_t));
      newp->fname = lr->fname;
      newp->lineno = lr->lineno;
      newp->token = (const char *) obstack_copy0 (&collate->element_mem,
						  code->val.str.start,
						  code->val.str.len);
      newp->where.idx = collate->nweight++;
      newp->next = collate->current_patch;
      collate->current_patch = newp;
    }
  else
    collate->weight[collate->nweight++] = here_weight;
  ++collate->weight_cnt[collate->weight_idx];

  return 0;
}


int
collate_next_weight (struct linereader *lr, struct localedef_t *locale)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;

  if (collate->kind == symbol)
    {
      lr_error (lr, _("\
specification of sorting weight for collation symbol does not make sense"));
      lr_ignore_rest (lr, 0);
      return -1;
    }

  ++collate->weight_idx;
  if (collate->weight_idx >= collate->nrules)
    {
      lr_error (lr, _("too many weights"));
      lr_ignore_rest (lr, 0);
      return -1;
    }

  return 0;
}


int
collate_simple_weight (struct linereader *lr, struct localedef_t *locale,
		       struct token *code, struct charset_t *charset)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  unsigned int value = 0;

  /* There current tokens can be `IGNORE', `...', or a string.  */
  switch (code->tok)
    {
    case tok_ignore:
      /* This token is allowed in all situations.  */
      value = IGNORE_CHAR;
      break;

    case tok_ellipsis:
      /* The ellipsis is only allowed for the `...' or `UNDEFINED'
	 entry.  */
      if (collate->kind != ellipsis && collate->kind != undefined)
	{
	  lr_error (lr, _("\
`...' must only be used in `...' and `UNDEFINED' entries"));
	  lr_ignore_rest (lr, 0);
	  return -1;
	}
      value = ELLIPSIS_CHAR;
      break;

    case tok_string:
      /* This can become difficult.  We have to get the weights which
	 correspond to the single wide chars in the string.  But some
	 of the `chars' might not be real characters, but collation
	 elements or symbols.  And so the string decoder might have
	 signaled errors.  The string at this point is not translated.
	 I.e., all <...> sequences are still there.  */
      {
	char *runp = code->val.str.start;
	void *tmp;

	while (*runp != '\0')
	  {
	    char *startp = (char *) runp;
	    char *putp = (char *) runp;
	    wchar_t wch;

	    /* Lookup weight for char and store it.  */
	    if (*runp == '<')
	      {
		while (*++runp != '\0' && *runp != '>')
		  {
		    if (*runp == lr->escape_char)
		      if (*++runp == '\0')
			{
			  lr_error (lr, _("unterminated weight name"));
			  lr_ignore_rest (lr, 0);
			  return -1;
			}
		    *putp++ = *runp;
		  }
		if (*runp == '>')
		  ++runp;

		if (putp == startp)
		  {
		    lr_error (lr, _("empty weight name: line ignored"));
		    lr_ignore_rest (lr, 0);
		    return -1;
		  }

		wch = charset_find_value (&charset->char_table, startp,
					  putp - startp);
		if (wch != ILLEGAL_CHAR_VALUE)
		  {
		    element_t *pelem;

		    if (find_entry (&collate->result, &wch, sizeof (wchar_t),
				    (void *)&pelem) < 0)
		      pelem = NULL;

		    while (pelem != NULL
			   && (pelem->name[0] != wch
			       || pelem->name[1] != L'\0'))
		      pelem = pelem->next;

		    value = pelem == NULL ? 0 : pelem->this_weight;
		  }
		else if (find_entry (&collate->elements, startp, putp - startp,
				     &tmp) >= 0)
		  {
		    element_t *pelem = (element_t *) tmp;

		    value = pelem->this_weight;
		  }
		else if (find_entry (&collate->symbols, startp, putp - startp,
				     &tmp) >= 0)
		  {
		    value = (unsigned int) tmp;
		  }
		else
		  {
		    if (verbose)
		      lr_error (lr, _("unknown symbol `%.*s': line ignored"),
				(int) (putp - startp), startp);
		    lr_ignore_rest (lr, 0);
		    return -1;
		  }
	      }
	    else
	      {
		element_t *wp;
		wchar_t wch;

		if (*runp == lr->escape_char)
		  {
		    static const char digits[] = "0123456789abcdef";
		    const char *dp;
		    int base;

		    ++runp;
		    if (tolower (*runp) == 'x')
		      {
			++runp;
			base = 16;
		      }
		    else if (tolower (*runp) == 'd')
		      {
			++runp;
			base = 10;
		      }
		    else
		      base = 8;

		    dp = strchr (digits, tolower (*runp));
		    if (dp == NULL || (dp - digits) >= base)
		      {
		      illegal_char:
			lr_error (lr, _("\
illegal character constant in string"));
			lr_ignore_rest (lr, 0);
			return -1;
		      }
		    wch = dp - digits;
		    ++runp;

		    dp = strchr (digits, tolower (*runp));
		    if (dp == NULL || (dp - digits) >= base)
		      goto illegal_char;
		    wch *= base;
		    wch += dp - digits;
		    ++runp;

		    if (base != 16)
		      {
			dp = strchr (digits, tolower (*runp));
			if (dp != NULL && (dp - digits < base))
			  {
			    wch *= base;
			    wch += dp - digits;
			    ++runp;
			  }
		      }
		  }
		else
		  wch = (wchar_t) *runp++;

		/* Lookup the weight for WCH.  */
		if (find_entry (&collate->result, &wch, sizeof (wch),
				(void *)&wp) < 0)
		  wp = NULL;

		while (wp != NULL
		       && (wp->name[0] != wch || wp->name[1] != L'\0'))
		  wp = wp->next;

		value = wp == NULL ? 0 : wp->this_weight;

		/* To get the correct name for the error message.  */
		putp = runp;

		/**************************************************\
		|* I know here is something wrong.  Characters in *|
		|* the string which are not in the <...> form	  *|
		|* cannot be declared forward for now!!!	  *|
		\**************************************************/
	      }

	    /* Store in weight array.  */
	    if (collate->nweight >= collate->nweight_max)
	      {
		collate->nweight_max *= 2;
		collate->weight
		  = (unsigned int *) xrealloc (collate->weight,
					       collate->nweight_max);
	      }

	    if (value == 0)
	      {
		patch_t *newp;

		newp = (patch_t *) obstack_alloc (&collate->element_mem,
						  sizeof (patch_t));
		newp->fname = lr->fname;
		newp->lineno = lr->lineno;
		newp->token
		  = (const char *) obstack_copy0 (&collate->element_mem,
						  startp, putp - startp);
		newp->where.idx = collate->nweight++;
		newp->next = collate->current_patch;
		collate->current_patch = newp;
	      }
	    else
	      collate->weight[collate->nweight++] = value;
	    ++collate->weight_cnt[collate->weight_idx];
	  }
      }
      return 0;

    default:
      assert (! "should not happen");
    }


  if (collate->nweight >= collate->nweight_max)
    {
      collate->nweight_max *= 2;
      collate->weight = (unsigned int *) xrealloc (collate->weight,
						   collate->nweight_max);
    }

  collate->weight[collate->nweight++] = value;
  ++collate->weight_cnt[collate->weight_idx];

  return 0;
}


void
collate_end_weight (struct linereader *lr, struct localedef_t *locale)
{
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  element_t *pelem = collate->current_element;

  if (collate->kind == symbol)
    {
      /* We don't have to do anything.  */
      collate->was_ellipsis = 0;
      return;
    }

  if (collate->kind == ellipsis)
    {
      /* Before the next line is processed the ellipsis is handled.  */
      collate->was_ellipsis = 1;
      return;
    }

  assert (collate->kind == character || collate->kind == element
	  || collate->kind == undefined);

  /* Fill in the missing weights.  */
  while (++collate->weight_idx < collate->nrules)
    {
      collate->weight[collate->nweight++] = pelem->this_weight;
      ++collate->weight_cnt[collate->weight_idx];
    }

  /* Now we know how many ordering weights the current
     character/element has.  Allocate room in the element structure
     and copy information.  */
  pelem->ordering_len = collate->nweight;

  /* First we write an array with the number of values for each
     weight.  */
  obstack_grow (&collate->element_mem, collate->weight_cnt,
		collate->nrules * sizeof (unsigned int));

  /* Now the weights itselves.  */
  obstack_grow (&collate->element_mem, collate->weight,
		collate->nweight * sizeof (unsigned int));

  /* Get result.  */
  pelem->ordering = obstack_finish (&collate->element_mem);

  /* Now we handle the "patches".  */
  while (collate->current_patch != NULL)
    {
      patch_t *this_patch;

      this_patch = collate->current_patch;

      this_patch->where.pos = &pelem->ordering[collate->nrules
					      + this_patch->where.idx];

      collate->current_patch = this_patch->next;
      this_patch->next = collate->all_patches;
      collate->all_patches = this_patch;
    }

  /* Set information for next round.  */
  collate->was_ellipsis = 0;
  if (collate->kind != undefined)
    collate->last_char = pelem->name[0];
}
