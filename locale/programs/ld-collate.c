/* Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 1995.

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

#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <wchar.h>

#include "charmap.h"
#include "localeinfo.h"
#include "linereader.h"
#include "locfile.h"
#include "localedef.h"

/* Uncomment the following line in the production version.  */
/* #define NDEBUG 1 */
#include <assert.h>

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

/* Forward declaration.  */
struct element_t;

/* Data type for list of strings.  */
struct section_list
{
  struct section_list *next;
  /* Name of the section.  */
  const char *name;
  /* First element of this section.  */
  struct element_t *first;
  /* Last element of this section.  */
  struct element_t *last;
  /* These are the rules for this section.  */
  enum coll_sort_rule *rules;
};

struct element_t;

struct element_list_t
{
  /* Number of elements.  */
  int cnt;

  struct element_t **w;
};

/* Data type for collating element.  */
struct element_t
{
  const char *name;

  const char *mbs;
  const uint32_t *wcs;
  int mborder;
  int wcorder;

  struct element_list_t *weights;

  /* Where does the definition come from.  */
  const char *file;
  size_t line;

  /* Which section does this belong to.  */
  struct section_list *section;

  /* Predecessor and successor in the order list.  */
  struct element_t *last;
  struct element_t *next;

  /* Next element in multibyte output list.  */
  struct element_t *mbnext;
};

/* Special element value.  */
#define ELEMENT_ELLIPSIS2	((struct element_t *) 1)
#define ELEMENT_ELLIPSIS3	((struct element_t *) 2)
#define ELEMENT_ELLIPSIS4	((struct element_t *) 3)

/* Data type for collating symbol.  */
struct symbol_t
{
  /* Point to place in the order list.  */
  struct element_t *order;

  /* Where does the definition come from.  */
  const char *file;
  size_t line;
};


/* The real definition of the struct for the LC_COLLATE locale.  */
struct locale_collate_t
{
  int col_weight_max;
  int cur_weight_max;

  /* List of known scripts.  */
  struct section_list *sections;
  /* Current section using definition.  */
  struct section_list *current_section;
  /* There always can be an unnamed section.  */
  struct section_list unnamed_section;
  /* To make handling of errors easier we have another section.  */
  struct section_list error_section;

  /* Number of sorting rules given in order_start line.  */
  uint32_t nrules;

  /* Start of the order list.  */
  struct element_t *start;

  /* The undefined element.  */
  struct element_t undefined;

  /* This is the cursor for `reorder_after' insertions.  */
  struct element_t *cursor;

  /* This value is used when handling ellipsis.  */
  struct element_t ellipsis_weight;

  /* Known collating elements.  */
  hash_table elem_table;

  /* Known collating symbols.  */
  hash_table sym_table;

  /* Known collation sequences.  */
  hash_table seq_table;

  struct obstack mempool;

  /* The LC_COLLATE category is a bit special as it is sometimes possible
     that the definitions from more than one input file contains information.
     Therefore we keep all relevant input in a list.  */
  struct locale_collate_t *next;

  /* Arrays with heads of the list for each of the leading bytes in
     the multibyte sequences.  */
  struct element_t *mbheads[256];
};


/* We have a few global variables which are used for reading all
   LC_COLLATE category descriptions in all files.  */
static int nrules;


static struct section_list *
make_seclist_elem (struct locale_collate_t *collate, const char *string,
		   struct section_list *next)
{
  struct section_list *newp;

  newp = (struct section_list *) obstack_alloc (&collate->mempool,
						sizeof (*newp));
  newp->next = next;
  newp->name = string;
  newp->first = NULL;

  return newp;
}


static struct element_t *
new_element (struct locale_collate_t *collate, const char *mbs, size_t mbslen,
	     const uint32_t *wcs, const char *name, size_t namelen)
{
  struct element_t *newp;

  newp = (struct element_t *) obstack_alloc (&collate->mempool,
					     sizeof (*newp));
  newp->name = name == NULL ? NULL : obstack_copy (&collate->mempool,
						   name, namelen);
  if (mbs != NULL)
    newp->mbs = obstack_copy0 (&collate->mempool, mbs, mbslen);
  else
    newp->mbs = NULL;
  if (wcs != NULL)
    {
      size_t nwcs = wcslen ((wchar_t *) wcs) + 1;
      uint32_t zero = 0;
      obstack_grow (&collate->mempool, wcs, nwcs * sizeof (uint32_t));
      obstack_grow (&collate->mempool, &zero, sizeof (uint32_t));
      newp->wcs = (uint32_t *) obstack_finish (&collate->mempool);
    }
  else
    newp->wcs = NULL;
  newp->mborder = 0;
  newp->wcorder = 0;

  /* Will be allocated later.  */
  newp->weights = NULL;

  newp->file = NULL;
  newp->line = 0;

  newp->section = NULL;

  newp->last = NULL;
  newp->next = NULL;

  newp->mbnext = NULL;

  return newp;
}


static struct symbol_t *
new_symbol (struct locale_collate_t *collate)
{
  struct symbol_t *newp;

  newp = (struct symbol_t *) obstack_alloc (&collate->mempool, sizeof (*newp));

  newp->order = NULL;

  newp->file = NULL;
  newp->line = 0;

  return newp;
}


/* Test whether this name is already defined somewhere.  */
static int
check_duplicate (struct linereader *ldfile, struct locale_collate_t *collate,
		 struct charmap_t *charmap, struct repertoire_t *repertoire,
		 const char *symbol, size_t symbol_len)
{
  void *ignore = NULL;

  if (find_entry (&charmap->char_table, symbol, symbol_len, &ignore) == 0)
    {
      lr_error (ldfile, _("`%s' already defined in charmap"), symbol);
      return 1;
    }

  if (find_entry (&repertoire->char_table, symbol, symbol_len, &ignore) == 0)
    {
      lr_error (ldfile, _("`%s' already defined in repertoire"), symbol);
      return 1;
    }

  if (find_entry (&collate->sym_table, symbol, symbol_len, &ignore) == 0)
    {
      lr_error (ldfile, _("`%s' already defined as collating symbol"), symbol);
      return 1;
    }

  if (find_entry (&collate->elem_table, symbol, symbol_len, &ignore) == 0)
    {
      lr_error (ldfile, _("`%s' already defined as collating element"),
		symbol);
      return 1;
    }

  return 0;
}


/* Read the direction specification.  */
static void
read_directions (struct linereader *ldfile, struct token *arg,
		 struct charmap_t *charmap, struct repertoire_t *repertoire,
		 struct locale_collate_t *collate)
{
  int cnt = 0;
  int max = nrules ?: 10;
  enum coll_sort_rule *rules = calloc (max, sizeof (*rules));
  int warned = 0;

  while (1)
    {
      int valid = 0;

      if (arg->tok == tok_forward)
	{
	  if (rules[cnt] & sort_backward)
	    {
	      if (! warned)
		{
		  lr_error (ldfile, _("\
%s: `forward' and `backward' are mutually excluding each other"),
			    "LC_COLLATE");
		  warned = 1;
		}
	    }
	  else if (rules[cnt] & sort_forward)
	    {
	      if (! warned)
		{
		  lr_error (ldfile, _("\
%s: `%s' mentioned twice in definition of weight %d"),
			    "LC_COLLATE", "forward", cnt + 1);
		}
	    }
	  else
	    rules[cnt] |= sort_forward;

	  valid = 1;
	}
      else if (arg->tok == tok_backward)
	{
	  if (rules[cnt] & sort_forward)
	    {
	      if (! warned)
		{
		  lr_error (ldfile, _("\
%s: `forward' and `backward' are mutually excluding each other"),
			    "LC_COLLATE");
		  warned = 1;
		}
	    }
	  else if (rules[cnt] & sort_backward)
	    {
	      if (! warned)
		{
		  lr_error (ldfile, _("\
%s: `%s' mentioned twice in definition of weight %d"),
			    "LC_COLLATE", "backward", cnt + 1);
		}
	    }
	  else
	    rules[cnt] |= sort_backward;

	  valid = 1;
	}
      else if (arg->tok == tok_position)
	{
	  if (rules[cnt] & sort_position)
	    {
	      if (! warned)
		{
		  lr_error (ldfile, _("\
%s: `%s' mentioned twice in definition of weight %d in category `%s'"),
			    "LC_COLLATE", "position", cnt + 1);
		}
	    }
	  else
	    rules[cnt] |= sort_position;

	  valid = 1;
	}

      if (valid)
	arg = lr_token (ldfile, charmap, repertoire);

      if (arg->tok == tok_eof || arg->tok == tok_eol || arg->tok == tok_comma
	  || arg->tok == tok_semicolon)
	{
	  if (! valid && ! warned)
	    {
	      lr_error (ldfile, _("%s: syntax error"), "LC_COLLATE");
	      warned = 1;
	    }

	  /* See whether we have to increment the counter.  */
	  if (arg->tok != tok_comma && rules[cnt] != 0)
	    ++cnt;

	  if (arg->tok == tok_eof || arg->tok == tok_eol)
	    /* End of line or file, so we exit the loop.  */
	    break;

	  if (nrules == 0)
	    {
	      /* See whether we have enough room in the array.  */
	      if (cnt == max)
		{
		  max += 10;
		  rules = (enum coll_sort_rule *) xrealloc (rules,
							    max
							    * sizeof (*rules));
		  memset (&rules[cnt], '\0', (max - cnt) * sizeof (*rules));
		}
	    }
	  else
	    {
	      if (cnt == nrules)
		{
		  /* There must not be any more rule.  */
		  if (! warned)
		    {
		      lr_error (ldfile, _("\
%s: too many rules; first entry only had %d"),
				"LC_COLLATE", nrules);
		      warned = 1;
		    }

		  lr_ignore_rest (ldfile, 0);
		  break;
		}
	    }
	}
      else
	{
	  if (! warned)
	    {
	      lr_error (ldfile, _("%s: syntax error"), "LC_COLLATE");
	      warned = 1;
	    }
	}

      arg = lr_token (ldfile, charmap, repertoire);
    }

  if (nrules == 0)
    {
      /* Now we know how many rules we have.  */
      nrules = cnt;
      rules = (enum coll_sort_rule *) xrealloc (rules,
						nrules * sizeof (*rules));
    }
  else
    {
      if (cnt < nrules)
	{
	  /* Not enough rules in this specification.  */
	  if (! warned)
	    lr_error (ldfile, _("%s: not enough sorting rules"), "LC_COLLATE");

	  do
	    rules[cnt] = sort_forward;
	  while (++cnt < nrules);
	}
    }

  collate->current_section->rules = rules;
}


static struct element_t *
find_element (struct linereader *ldfile, struct locale_collate_t *collate,
	      const char *str, size_t len, uint32_t *wcstr)
{
  struct element_t *result = NULL;

  /* Search for the entries among the collation sequences already define.  */
  if (find_entry (&collate->seq_table, str, len, (void **) &result) != 0)
    {
      /* Nope, not define yet.  So we see whether it is a
         collation symbol.  */
      void *ptr;

      if (find_entry (&collate->sym_table, str, len, &ptr) == 0)
	{
	  /* It's a collation symbol.  */
	  struct symbol_t *sym = (struct symbol_t *) ptr;
	  result = sym->order;

	  if (result == NULL)
	    result = sym->order = new_element (collate, NULL, 0, NULL,
					       NULL, 0);
	}
      else if (find_entry (&collate->elem_table, str, len,
			   (void **) &result) != 0)
	{
	  /* It's also no collation element.  So it is an character
	     element defined later.  */
	  result = new_element (collate, NULL, 0, NULL, str, len);
	  if (result != NULL)
	    /* Insert it into the sequence table.  */
	    insert_entry (&collate->seq_table, str, len, result);
	}
    }

  return result;
}


static void
unlink_element (struct locale_collate_t *collate)
{
  if (collate->cursor->next != NULL)
    collate->cursor->next->last = collate->cursor->last;
  if (collate->cursor->last != NULL)
    collate->cursor->last->next = collate->cursor->next;
  collate->cursor = collate->cursor->last;
}


static void
insert_weights (struct linereader *ldfile, struct element_t *elem,
		struct charmap_t *charmap, struct repertoire_t *repertoire,
		struct locale_collate_t *collate, enum token_t ellipsis)
{
  int weight_cnt;
  struct token *arg;

  /* Initialize all the fields.  */
  elem->file = ldfile->fname;
  elem->line = ldfile->lineno;
  elem->last = collate->cursor;
  elem->next = collate->cursor ? collate->cursor->next : NULL;
  if (collate->cursor != NULL)
    collate->cursor->next = elem;
  elem->weights = (struct element_list_t *)
    obstack_alloc (&collate->mempool, nrules * sizeof (struct element_list_t));
  memset (elem->weights, '\0', nrules * sizeof (struct element_list_t));

  if (collate->current_section->first == NULL)
    collate->current_section->first = elem;
  if (collate->current_section->last == collate->cursor)
    collate->current_section->last = elem;

  collate->cursor = elem;

  weight_cnt = 0;

  arg = lr_token (ldfile, charmap, repertoire);
  do
    {
      if (arg->tok == tok_eof || arg->tok == tok_eol)
	break;

      if (arg->tok == tok_ignore)
	{
	  /* The weight for this level has to be ignored.  We use the
	     null pointer to indicate this.  */
	  elem->weights[weight_cnt].w = (struct element_t **)
	    obstack_alloc (&collate->mempool, sizeof (struct element_t *));
	  elem->weights[weight_cnt].w[0] = NULL;
	  elem->weights[weight_cnt].cnt = 1;
	}
      else if (arg->tok == tok_bsymbol)
	{
	  struct element_t *val = find_element (ldfile, collate,
						arg->val.str.startmb,
						arg->val.str.lenmb,
						arg->val.str.startwc);

	  if (val == NULL)
	    break;

	  elem->weights[weight_cnt].w = (struct element_t **)
	    obstack_alloc (&collate->mempool, sizeof (struct element_t *));
	  elem->weights[weight_cnt].w[0] = val;
	  elem->weights[weight_cnt].cnt = 1;
	}
      else if (arg->tok == tok_string)
	{
	  /* Split the string up in the individual characters and put
	     the element definitions in the list.  */
	  const char *cp = arg->val.str.startmb;
	  int cnt = 0;
	  struct element_t *charelem;
	  void *base = obstack_base (&collate->mempool);

	  if (*cp == '\0')
	    {
	      lr_error (ldfile, _("%s: empty weight string not allowed"),
			"LC_COLLATE");
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  do
	    {
	      if (*cp == '<')
		{
		  /* Ahh, it's a bsymbol.  That's what we want.  */
		  const char *startp = cp;

		  while (*++cp != '>')
		    {
		      if (*cp == ldfile->escape_char)
			++cp;
		      if (*cp == '\0')
			{
			  /* It's a syntax error.  */
			  obstack_free (&collate->mempool, base);
			  goto syntax;
			}
		    }

		    charelem = find_element (ldfile, collate, startp,
					     cp - startp, NULL);
		    ++cp;
		}
	      else
		{
		  /* People really shouldn't use characters directly in
		     the string.  Especially since it's not really clear
		     what this means.  We interpret all characters in the
		     string as if that would be bsymbols.  Otherwise we
		     would have to match back to bsymbols somehow and this
		     is also not what people normally expect.  */
		  charelem = find_element (ldfile, collate, cp++, 1, NULL);
		}

	      if (charelem == NULL)
		{
		  /* We ignore the rest of the line.  */
		  lr_ignore_rest (ldfile, 0);
		  break;
		}

	      /* Add the pointer.  */
	      obstack_ptr_grow (&collate->mempool, charelem);
	      ++cnt;
	    }
	  while (*cp != '\0');

	  /* Now store the information.  */
	  elem->weights[weight_cnt].w = (struct element_t **)
	    obstack_finish (&collate->mempool);
	  elem->weights[weight_cnt].cnt = cnt;

	  /* We don't need the string anymore.  */
	  free (arg->val.str.startmb);
	}
      else if (ellipsis != tok_none
	       && (arg->tok == tok_ellipsis2
		   || arg->tok == tok_ellipsis3
		   || arg->tok == tok_ellipsis4))
	{
	  /* It must be the same ellipsis as used in the initial column.  */
	  if (arg->tok != ellipsis)
	    lr_error (ldfile, _("\
%s: weights must use the same ellipsis symbol as the name"),
		      "LC_COLLATE");

	  /* The weight for this level has to be ignored.  We use the
	     null pointer to indicate this.  */
	  elem->weights[weight_cnt].w = (struct element_t **)
	    obstack_alloc (&collate->mempool, sizeof (struct element_t *));
	  elem->weights[weight_cnt].w[0] = ELEMENT_ELLIPSIS2;
	  elem->weights[weight_cnt].cnt = 1;
	}
      else
	{
	syntax:
	  /* It's a syntax error.  */
	  lr_error (ldfile, _("%s: syntax error"), "LC_COLLATE");
	  lr_ignore_rest (ldfile, 0);
	  break;
	}

      arg = lr_token (ldfile, charmap, repertoire);
      /* This better should be the end of the line or a semicolon.  */
      if (arg->tok == tok_semicolon)
	/* OK, ignore this and read the next token.  */
	arg = lr_token (ldfile, charmap, repertoire);
      else if (arg->tok != tok_eof && arg->tok != tok_eol)
	{
	  /* It's a syntax error.  */
	  lr_error (ldfile, _("%s: syntax error"), "LC_COLLATE");
	  lr_ignore_rest (ldfile, 0);
	  break;
	}
    }
  while (++weight_cnt < nrules);

  if (weight_cnt < nrules)
    {
      /* This means the rest of the line uses the current element as
	 the weight.  */
      do
	{
	  elem->weights[weight_cnt].w = (struct element_t **)
	    obstack_alloc (&collate->mempool, sizeof (struct element_t *));
	  elem->weights[weight_cnt].w[0] = elem;
	  elem->weights[weight_cnt].cnt = 1;
	}
      while (++weight_cnt < nrules);
    }
  else
    {
      if (arg->tok == tok_ignore || arg->tok == tok_bsymbol)
	{
	  /* Too many rule values.  */
	  lr_error (ldfile, _("%s: too many values"), "LC_COLLATE");
	  lr_ignore_rest (ldfile, 0);
	}
      else
	lr_ignore_rest (ldfile, arg->tok != tok_eol && arg->tok != tok_eof);
    }
}


static int
insert_value (struct linereader *ldfile, struct token *arg,
	      struct charmap_t *charmap, struct repertoire_t *repertoire,
	      struct locale_collate_t *collate)
{
  /* First find out what kind of symbol this is.  */
  struct charseq *seq;
  uint32_t wc;
  struct element_t *elem = NULL;

  /* Try to find the character in the charmap.  */
  seq = charmap_find_value (charmap, arg->val.str.startmb, arg->val.str.lenmb);

  /* Determine the wide character.  */
  if (seq == NULL || seq->ucs4 == UNINITIALIZED_CHAR_VALUE)
    {
      wc = repertoire_find_value (repertoire, arg->val.str.startmb,
				  arg->val.str.lenmb);
      if (seq != NULL)
	seq->ucs4 = wc;
    }
  else
    wc = seq->ucs4;

  if (wc == ILLEGAL_CHAR_VALUE && seq == NULL)
    {
      /* It's no character, so look through the collation elements and
	 symbol list.  */
      void *result;

      if (find_entry (&collate->sym_table, arg->val.str.startmb,
		      arg->val.str.lenmb, &result) == 0)
	{
	  /* It's a collation symbol.  */
	  struct symbol_t *sym = (struct symbol_t *) result;
	  elem = sym->order;

	  if (elem == NULL)
	    elem = sym->order = new_element (collate, NULL, 0, NULL, NULL, 0);
	}
      else if (find_entry (&collate->elem_table, arg->val.str.startmb,
			   arg->val.str.lenmb, (void **) &elem) != 0)
	{
	  /* It's also no collation element.  Therefore ignore it.  */
	  lr_ignore_rest (ldfile, 0);
	  return 1;
	}
    }
  else
    {
      /* Otherwise the symbols stands for a character.  */
      if (find_entry (&collate->seq_table, arg->val.str.startmb,
		      arg->val.str.lenmb, (void **) &elem) != 0)
	{
	  uint32_t wcs[2] = { wc, 0 };

	  /* We have to allocate an entry.  */
	  elem = new_element (collate, seq != NULL ? seq->bytes : NULL,
			      seq != NULL ? seq->nbytes : 0,
			      wcs, arg->val.str.startmb, arg->val.str.lenmb);

	  /* And add it to the table.  */
	  if (insert_entry (&collate->seq_table, arg->val.str.startmb,
			    arg->val.str.lenmb, elem) != 0)
	    /* This cannot happen.  */
	    assert (! "Internal error");
	}
    }

  /* Test whether this element is not already in the list.  */
  if (elem->next != NULL || (collate->cursor != NULL
			     && elem->next == collate->cursor))
    {
      lr_error (ldfile, _("order for `%.*s' already defined at %s:%zu"),
		arg->val.str.lenmb, arg->val.str.startmb,
		elem->file, elem->line);
      lr_ignore_rest (ldfile, 0);
      return 1;
    }

  insert_weights (ldfile, elem, charmap, repertoire, collate, tok_none);

  return 0;
}


static void
handle_ellipsis (struct linereader *ldfile, struct token *arg,
		 enum token_t ellipsis, struct charmap_t *charmap,
		 struct repertoire_t *repertoire,
		 struct locale_collate_t *collate)
{
  struct element_t *startp;
  struct element_t *endp;

  /* Unlink the entry added for the ellipsis.  */
  unlink_element (collate);
  startp = collate->cursor;

  /* Process and add the end-entry.  */
  if (arg != NULL
      && insert_value (ldfile, arg, charmap, repertoire, collate))
    /* Something went wrong with inserting the to-value.  This means
       we cannot process the ellipsis.  */
    return;

  /* Reset the cursor.  */
  collate->cursor = startp;

  /* Now we have to handle many different situations:
     - we have to distinguish between the three different ellipsis forms
     - the is the ellipsis at the beginning, in the middle, or at the end.
  */
  endp = collate->cursor->next;
  assert (arg == NULL || endp != NULL);

  /* Both, the start and the end symbol, must stand for characters.  */
  if ((startp == NULL || startp->name == NULL)
      || (endp == NULL || endp->name == NULL))
    {
      lr_error (ldfile, _("\
%s: the start end the end symbol of a range must stand for characters"),
		"LC_COLLATE");
      return;
    }

  if (ellipsis == tok_ellipsis3)
    {
      /* One requirement we make here: the length of the byte
	 sequences for the first and end character must be the same.
	 This is mainly to prevent unwanted effects and this is often
	 not what is wanted.  */
      size_t len = (startp->mbs != NULL ? strlen (startp->mbs)
		    : (endp->mbs != NULL ? strlen (endp->mbs) : 0));
      char mbcnt[len + 1];
      char mbend[len + 1];

      /* Well, this should be caught somewhere else already.  Just to
	 make sure.  */
      assert (startp == NULL || startp->wcs == NULL || startp->wcs[1] == 0);
      assert (endp == NULL || endp->wcs == NULL || endp->wcs[1] == 0);

      if (startp != NULL && endp != NULL
	  && startp->mbs != NULL && endp->mbs != NULL
	  && strlen (startp->mbs) != strlen (endp->mbs))
	{
	  lr_error (ldfile, _("\
%s: byte sequences of first and last character must have the same length"),
		    "LC_COLLATE");
	  return;
	}

      /* Determine whether we have to generate multibyte sequences.  */
      if ((startp == NULL || startp->mbs != NULL)
	  && (endp == NULL || endp->mbs != NULL))
	{
	  int cnt;
	  int ret;

	  /* Prepare the beginning byte sequence.  This is either from the
	     beginning byte sequence or it is all nulls if it was an
	     initial ellipsis.  */
	  if (startp == NULL || startp->mbs == NULL)
	    memset (mbcnt, '\0', len);
	  else
	    {
	      memcpy (mbcnt, startp->mbs, len);

	      /* And increment it so that the value is the first one we will
		 try to insert.  */
	      for (cnt = len - 1; cnt >= 0; --cnt)
		if (++mbcnt[cnt] != '\0')
		  break;
	    }
	  mbcnt[len] = '\0';

	  /* And the end sequence.  */
	  if (endp == NULL || endp->mbs == NULL)
	    memset (mbend, '\0', len);
	  else
	    memcpy (mbend, endp->mbs, len);
	  mbend[len] = '\0';

	  /* Test whether we have a correct range.  */
	  ret = memcmp (mbcnt, mbend, len);
	  if (ret >= 0)
	    {
	      if (ret > 0)
		lr_error (ldfile, _("%s: byte sequence of first character of \
sequence is not lower than that of the last character"), "LC_COLLATE");
	      return;
	    }

	  /* Generate the byte sequences data.  */
	  while (1)
	    {
	      struct charseq *seq;

	      /* Quite a bit of work ahead.  We have to find the character
		 definition for the byte sequence and then determine the
		 wide character belonging to it.  */
	      seq = charmap_find_symbol (charmap, mbcnt, len);
	      if (seq != NULL)
		{
		  struct element_t *elem;
		  size_t namelen;

		  if (seq->ucs4 == UNINITIALIZED_CHAR_VALUE)
		    seq->ucs4 = repertoire_find_value (repertoire, seq->name,
						       strlen (seq->name));

		  /* I don't this this can ever happen.  */
		  assert (seq->name != NULL);
		  namelen = strlen (seq->name);

		  /* Now we are ready to insert the new value in the
		     sequence.  Find out whether the element is
		     already known.  */
		  if (find_entry (&collate->seq_table, seq->name, namelen,
				  (void **) &elem) != 0)
		    {
		      uint32_t wcs[2] = { seq->ucs4, 0 };

		      /* We have to allocate an entry.  */
		      elem = new_element (collate, mbcnt, len, wcs, seq->name,
					  namelen);

		      /* And add it to the table.  */
		      if (insert_entry (&collate->seq_table, seq->name,
					namelen, elem) != 0)
			/* This cannot happen.  */
			assert (! "Internal error");
		    }

		  /* Test whether this element is not already in the list.  */
		  if (elem->next != NULL || (collate->cursor != NULL
					     && elem->next == collate->cursor))
		    {
		      lr_error (ldfile, _("\
order for `%.*s' already defined at %s:%zu"),
				namelen, seq->name, elem->file, elem->line);
		      goto increment;
		    }

		  /* Enqueue the new element.  */
		  elem->last = collate->cursor;
		  elem->next = collate->cursor->next;
		  elem->last->next = elem;
		  if (elem->next != NULL)
		    elem->next->last = elem;
		  collate->cursor = elem;

		 /* Add the weight value.  We take them from the
		    `ellipsis_weights' member of `collate'.  */
		  elem->weights = (struct element_list_t *)
		    obstack_alloc (&collate->mempool,
				   nrules * sizeof (struct element_list_t));
		  for (cnt = 0; cnt < nrules; ++cnt)
		    if (collate->ellipsis_weight.weights[cnt].cnt == 1
			&& (collate->ellipsis_weight.weights[cnt].w[0]
			    == ELEMENT_ELLIPSIS2))
		      {
			elem->weights[cnt].w = (struct element_t **)
			  obstack_alloc (&collate->mempool,
					 sizeof (struct element_t *));
			elem->weights[cnt].w[0] = elem;
			elem->weights[cnt].cnt = 1;
		      }
		    else
		      {
			/* Simly use the weight from `ellipsis_weight'.  */
			elem->weights[cnt].w =
			  collate->ellipsis_weight.weights[cnt].w;
			elem->weights[cnt].cnt =
			  collate->ellipsis_weight.weights[cnt].cnt;
		      }
		}

	      /* Increment for the next round.  */
	    increment:
	      for (cnt = len - 1; cnt >= 0; --cnt)
		if (++mbcnt[cnt] != '\0')
		  break;

	      /* Find out whether this was all.  */
	      if (cnt < 0 || memcmp (mbcnt, mbend, len) >= 0)
		/* Yep, that's all.  */
		break;
	    }
	}
    }
  else
    {
      /* For symbolic range we naturally must have a beginning and an
	 end specified by the user.  */
      if (startp == NULL)
	lr_error (ldfile, _("\
%s: symbolic range ellipsis must not directly follow `order_start'"),
		  "LC_COLLATE");
      else if (endp == NULL)
	lr_error (ldfile, _("\
%s: symbolic range ellipsis must not be direct followed by `order_end'"),
		  "LC_COLLATE");
      else
	{
	  /* Determine the range.  To do so we have to determine the
             common prefix of the both names and then the numeric
             values of both ends.  */
	  size_t lenfrom = strlen (startp->name);
	  size_t lento = strlen (endp->name);
	  char buf[lento + 1];
	  int preflen = 0;
	  long int from;
	  long int to;
	  char *cp;
	  int base = ellipsis == tok_ellipsis2 ? 16 : 10;

	  if (lenfrom != lento)
	    {
	    invalid_range:
	      lr_error (ldfile, _("\
`%s' and `%.*s' are no valid names for symbolic range"),
			startp->name, lento, endp->name);
	      return;
	    }

	  while (startp->name[preflen] == endp->name[preflen])
	    if (startp->name[preflen] == '\0')
	      /* Nothing to be done.  The start and end point are identical
		 and while inserting the end point we have already given
		 the user an error message.  */
	      return;
	    else
	      ++preflen;

	  errno = 0;
	  from = strtol (startp->name + preflen, &cp, base);
	  if ((from == UINT_MAX && errno == ERANGE) || *cp != '\0')
	    goto invalid_range;

	  errno = 0;
	  to = strtol (endp->name + preflen, &cp, base);
	  if ((to == UINT_MAX && errno == ERANGE) || *cp != '\0')
	    goto invalid_range;

	  /* Copy the prefix.  */
	  memcpy (buf, startp->name, preflen);

	  /* Loop over all values.  */
	  for (++from; from < to; ++from)
	    {
	      struct element_t *elem = NULL;
	      struct charseq *seq;
	      uint32_t wc;
	      int cnt;

	      /* Generate the the name.  */
	      sprintf (buf + preflen, base == 10 ? "%d" : "%x", from);

	      /* Look whether this name is already defined.  */
	      if (find_entry (&collate->seq_table, arg->val.str.startmb,
			      arg->val.str.lenmb, (void **) &elem) == 0)
		{
		  if (elem->next != NULL || (collate->cursor != NULL
					     && elem->next == collate->cursor))
		    {
		      lr_error (ldfile, _("\
%s: order for `%.*s' already defined at %s:%zu"),
				"LC_COLLATE", lenfrom, buf,
				elem->file, elem->line);
		      continue;
		    }

		  if (elem->name == NULL)
		    {
		      lr_error (ldfile, _("%s: `%s' must be a charater"),
				"LC_COLLATE", buf);
		      continue;
		    }
		}

	      if (elem == NULL || (elem->mbs == NULL && elem->wcs == NULL))
		{
		  /* Search for a character of this name.  */
		  seq = charmap_find_value (charmap, buf, lenfrom);
		  if (seq == NULL || seq->ucs4 == UNINITIALIZED_CHAR_VALUE)
		    {
		      wc = repertoire_find_value (repertoire, buf, lenfrom);

		      if (seq != NULL)
			seq->ucs4 = wc;
		    }
		  else
		    wc = seq->ucs4;

		  if (wc == ILLEGAL_CHAR_VALUE && seq == NULL)
		    /* We don't know anything about a character with this
		       name.  XXX Should we warn?  */
		    continue;

		  if (elem == NULL)
		    {
		      uint32_t wcs[2] = { wc, 0 };

		      /* We have to allocate an entry.  */
		      elem = new_element (collate,
					  seq != NULL ? seq->bytes : NULL,
					  seq != NULL ? seq->nbytes : 0,
					  wc == ILLEGAL_CHAR_VALUE
					  ? NULL : wcs,
					  buf, lenfrom);
		    }
		  else
		    {
		      /* Update the element.  */
		      if (seq != NULL)
			elem->mbs = obstack_copy0 (&collate->mempool,
						   seq->bytes, seq->nbytes);

		      if (wc != ILLEGAL_CHAR_VALUE)
			{
			  uint32_t zero = 0;

			  obstack_grow (&collate->mempool,
					&wc, sizeof (uint32_t));
			  obstack_grow (&collate->mempool,
					&zero, sizeof (uint32_t));
			  elem->wcs = obstack_finish (&collate->mempool);
			}
		    }

		  elem->file = ldfile->fname;
		  elem->line = ldfile->lineno;
		}

	      /* Enqueue the new element.  */
	      elem->last = collate->cursor;
	      elem->next = collate->cursor->next;
	      elem->last->next = elem;
	      if (elem->next != NULL)
		elem->next->last = elem;
	      collate->cursor = elem;

	      /* Now add the weights.  They come from the `ellipsis_weights'
		 member of `collate'.  */
	      elem->weights = (struct element_list_t *)
		obstack_alloc (&collate->mempool,
			       nrules * sizeof (struct element_list_t));
	      for (cnt = 0; cnt < nrules; ++cnt)
		if (collate->ellipsis_weight.weights[cnt].cnt == 1
		    && (collate->ellipsis_weight.weights[cnt].w[0]
			== ELEMENT_ELLIPSIS2))
		  {
		    elem->weights[cnt].w = (struct element_t **)
		      obstack_alloc (&collate->mempool,
				     sizeof (struct element_t *));
		    elem->weights[cnt].w[0] = elem;
		    elem->weights[cnt].cnt = 1;
		  }
		else
		  {
		    /* Simly use the weight from `ellipsis_weight'.  */
		    elem->weights[cnt].w =
		      collate->ellipsis_weight.weights[cnt].w;
		    elem->weights[cnt].cnt =
		      collate->ellipsis_weight.weights[cnt].cnt;
		  }
	    }
	}
    }
}


static void
collate_startup (struct linereader *ldfile, struct localedef_t *locale,
		 struct localedef_t *copy_locale, int ignore_content)
{
  if (!ignore_content)
    {
      struct locale_collate_t *collate;

      if (copy_locale == NULL)
	{
	  collate = locale->categories[LC_COLLATE].collate =
	    (struct locale_collate_t *)
	    xcalloc (1, sizeof (struct locale_collate_t));

	  /* Init the various data structures.  */
	  init_hash (&collate->elem_table, 100);
	  init_hash (&collate->sym_table, 100);
	  init_hash (&collate->seq_table, 500);
	  obstack_init (&collate->mempool);

	  collate->col_weight_max = -1;
	}
      else
	collate = locale->categories[LC_COLLATE].collate =
	  copy_locale->categories[LC_COLLATE].collate;
    }

  ldfile->translate_strings = 0;
  ldfile->return_widestr = 0;
}


void
collate_finish (struct localedef_t *locale, struct charmap_t *charmap)
{
  /* Now is the time when we can assign the individual collation
     values for all the symbols.  We have possibly different values
     for the wide- and the multibyte-character symbols.  This is done
     since it might make a difference in the encoding if there is in
     some cases no multibyte-character but there are wide-characters.
     (The other way around it is not important since theencoded
     collation value in the wide-character case is 32 bits wide and
     therefore requires no encoding).

     The lowest collation value assigned is 2.  Zero is reserved for
     the NUL byte terminating the strings in the `strxfrm'/`wcsxfrm'
     functions and 1 is used to separate the individual passes for the
     different rules.

     We also have to construct is list with all the bytes/words which
     can come first in a sequence, followed by all the elements which
     also start with this byte/word.  The order is reverse which has
     among others the important effect that longer strings are located
     first in the list.  This is required for the output data since
     the algorithm used in `strcoll' etc depends on this.

     The multibyte case is easy.  We simply sort into an array with
     256 elements.  */
  struct locale_collate_t *collate = locale->categories[LC_COLLATE].collate;
  int mbact = 2;
  int wcact = 2;
  struct element_t *runp = collate->start;

  while (runp != NULL)
    {
      if (runp->mbs != NULL)
	{
	  struct element_t **eptr;

	  /* Determine the order.  */
	  runp->mborder = mbact++;

	  /* Find the point where to insert in the list.  */
	  eptr = &collate->mbheads[(unsigned int) runp->mbs[0]];
	  while (*eptr != NULL)
	    {
	      /* Check which string is larger, the one we want to insert
		 or the current element of the list we are looking at.  */
	      assert (runp->mbs[0] == (*eptr)->mbs[0]);
	      if (strcmp (runp->mbs, (*eptr)->mbs) > 0)
		break;

	      eptr = &(*eptr)->mbnext;
	    }

	  /* Set the pointers.  */
	  runp->mbnext = *eptr;
	  *eptr = runp;
	}

      if (runp->wcs != NULL)
	runp->wcorder = wcact++;

      /* Up to the next entry.  */
      runp = runp->next;
    }
}


void
collate_output (struct localedef_t *locale, struct charmap_t *charmap,
		const char *output_path)
{
}


void
collate_read (struct linereader *ldfile, struct localedef_t *result,
	      struct charmap_t *charmap, const char *repertoire_name,
	      int ignore_content)
{
  struct repertoire_t *repertoire = NULL;
  struct locale_collate_t *collate;
  struct token *now;
  struct token *arg = NULL;
  enum token_t nowtok;
  int state = 0;
  enum token_t was_ellipsis = tok_none;
  struct localedef_t *copy_locale = NULL;

  /* Get the repertoire we have to use.  */
  if (repertoire_name != NULL)
    repertoire = repertoire_read (repertoire_name);

  /* The rest of the line containing `LC_COLLATE' must be free.  */
  lr_ignore_rest (ldfile, 1);

  do
    {
      now = lr_token (ldfile, charmap, NULL);
      nowtok = now->tok;
    }
  while (nowtok == tok_eol);

  if (nowtok == tok_copy)
    {
      state = 2;
      now = lr_token (ldfile, charmap, NULL);
      if (now->tok != tok_string)
	{
	  SYNTAX_ERROR (_("%s: syntax error"), "LC_COLLATE");

	skip_category:
	  do
	    now = lr_token (ldfile, charmap, NULL);
	  while (now->tok != tok_eof && now->tok != tok_end);

	  if (now->tok != tok_eof
	      || (now = lr_token (ldfile, charmap, NULL), now->tok == tok_eof))
	    lr_error (ldfile, _("%s: premature end of file"), "LC_COLLATE");
	  else if (now->tok != tok_lc_collate)
	    {
	      lr_error (ldfile, _("\
%1$s: definition does not end with `END %1$s'"), "LC_COLLATE");
	      lr_ignore_rest (ldfile, 0);
	    }
	  else
	    lr_ignore_rest (ldfile, 1);

	  return;
	}

      /* Get the locale definition.  */
      copy_locale = find_locale (LC_COLLATE, now->val.str.startmb,
				 repertoire_name, charmap);
      if ((copy_locale->avail & COLLATE_LOCALE) == 0)
	{
	  /* Not yet loaded.  So do it now.  */
	  if (locfile_read (copy_locale, charmap) != 0)
	    goto skip_category;
	}

      lr_ignore_rest (ldfile, 1);

      now = lr_token (ldfile, charmap, NULL);
      nowtok = now->tok;
    }

  /* Prepare the data structures.  */
  collate_startup (ldfile, result, copy_locale, ignore_content);
  collate = result->categories[LC_COLLATE].collate;

  while (1)
    {
      /* Of course we don't proceed beyond the end of file.  */
      if (nowtok == tok_eof)
	break;

      /* Ingore empty lines.  */
      if (nowtok == tok_eol)
	{
	  now = lr_token (ldfile, charmap, NULL);
	  nowtok = now->tok;
	  continue;
	}

      switch (nowtok)
	{
	case tok_coll_weight_max:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 0)
	    goto err_label;

	  arg = lr_token (ldfile, charmap, NULL);
	  if (arg->tok != tok_number)
	    goto err_label;
	  if (collate->col_weight_max != -1)
	    lr_error (ldfile, _("%s: duplicate definition of `%s'"),
		      "LC_COLLATE", "col_weight_max");
	  else
	    collate->col_weight_max = arg->val.num;
	  lr_ignore_rest (ldfile, 1);
	  break;

	case tok_section_symbol:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 0)
	    goto err_label;

	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok != tok_bsymbol)
	    goto err_label;
	  else if (!ignore_content)
	    {
	      /* Check whether this section is already known.  */
	      struct section_list *known = collate->sections;
	      while (known != NULL)
		if (strcmp (known->name, arg->val.str.startmb) == 0)
		  break;

	      if (known != NULL)
		{
		  lr_error (ldfile,
			    _("%s: duplicate declaration of section `%s'"),
			    "LC_COLLATE", arg->val.str.startmb);
		  free (arg->val.str.startmb);
		}
	      else
		collate->sections = make_seclist_elem (collate,
						       arg->val.str.startmb,
						       collate->sections);

	      lr_ignore_rest (ldfile, known == NULL);
	    }
	  else
	    {
	      free (arg->val.str.startmb);
	      lr_ignore_rest (ldfile, 0);
	    }
	  break;

	case tok_collating_element:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 0)
	    goto err_label;

	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok != tok_bsymbol)
	    goto err_label;
	  else
	    {
	      const char *symbol = arg->val.str.startmb;
	      size_t symbol_len = arg->val.str.lenmb;

	      /* Next the `from' keyword.  */
	      arg = lr_token (ldfile, charmap, repertoire);
	      if (arg->tok != tok_from)
		{
		  free ((char *) symbol);
		  goto err_label;
		}

	      ldfile->return_widestr = 1;

	      /* Finally the string with the replacement.  */
	      arg = lr_token (ldfile, charmap, repertoire);
	      ldfile->return_widestr = 0;
	      if (arg->tok != tok_string)
		goto err_label;

	      if (!ignore_content)
		{
		  if (symbol == NULL)
		    lr_error (ldfile, _("\
%s: unknown character in collating element name"),
			      "LC_COLLATE");
		  if (arg->val.str.startmb == NULL)
		    lr_error (ldfile, _("\
%s: unknown character in collating element definition"),
			      "LC_COLLATE");
		  if (arg->val.str.startwc == NULL)
		    lr_error (ldfile, _("\
%s: unknown wide character in collating element definition"),
			      "LC_COLLATE");
		  else if (arg->val.str.lenwc < 2)
		    lr_error (ldfile, _("\
%s: substitution string in collating element definition must have at least two characters"),
			      "LC_COLLATE");

		  if (symbol != NULL)
		    {
		      /* The name is already defined.  */
		      if (check_duplicate (ldfile, collate, charmap,
					   repertoire, symbol, symbol_len))
			goto col_elem_free;

		      if (insert_entry (&collate->elem_table,
					symbol, symbol_len,
					new_element (collate,
						     NULL, 0, NULL, symbol,
						     symbol_len)) < 0)
			lr_error (ldfile, _("\
error while adding collating element"));
		    }
		  else
		    goto col_elem_free;
		}
	      else
		{
		col_elem_free:
		  if (symbol != NULL)
		    free ((char *) symbol);
		  if (arg->val.str.startmb != NULL)
		    free (arg->val.str.startmb);
		  if (arg->val.str.startwc != NULL)
		    free (arg->val.str.startwc);
		}
	      lr_ignore_rest (ldfile, 1);
	    }
	  break;

	case tok_collating_symbol:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 0)
	    goto err_label;

	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok != tok_bsymbol)
	    goto err_label;
	  else
	    {
	      const char *symbol = arg->val.str.startmb;
	      size_t symbol_len = arg->val.str.lenmb;

	      if (!ignore_content)
		{
		  if (symbol == NULL)
		    lr_error (ldfile, _("\
%s: unknown character in collating symbol name"),
			      "LC_COLLATE");
		  else
		    {
		      /* The name is already defined.  */
		      if (check_duplicate (ldfile, collate, charmap,
					   repertoire, symbol, symbol_len))
			goto col_sym_free;

		      if (insert_entry (&collate->sym_table,
					symbol, symbol_len,
					new_symbol (collate)) < 0)
			lr_error (ldfile, _("\
error while adding collating symbol"));
		    }
		}
	      else
		{
		col_sym_free:
		  if (symbol != NULL)
		    free ((char *) symbol);
		}
	      lr_ignore_rest (ldfile, 1);
	    }
	  break;

	case tok_symbol_equivalence:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 0)
	    goto err_label;

	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok != tok_bsymbol)
	    goto err_label;
	  else
	    {
	      const char *newname = arg->val.str.startmb;
	      size_t newname_len = arg->val.str.lenmb;
	      const char *symname;
	      size_t symname_len;
	      struct symbol_t *symval;

	      arg = lr_token (ldfile, charmap, repertoire);
	      if (arg->tok != tok_bsymbol)
		{
		  if (newname != NULL)
		    free ((char *) newname);
		  goto err_label;
		}

	      symname = arg->val.str.startmb;
	      symname_len = arg->val.str.lenmb;

	      if (!ignore_content)
		{
		  if (newname == NULL)
		    {
		      lr_error (ldfile, _("\
%s: unknown character in equivalent definition name"),
				"LC_COLLATE");
		      goto sym_equiv_free;
		    }
		  if (symname == NULL)
		    {
		      lr_error (ldfile, _("\
%s: unknown character in equivalent definition value"),
				"LC_COLLATE");
		      goto sym_equiv_free;
		    }
		  /* The name is already defined.  */
		  if (check_duplicate (ldfile, collate, charmap,
				       repertoire, symname, symname_len))
		    goto col_sym_free;

		  /* See whether the symbol name is already defined.  */
		  if (find_entry (&collate->sym_table, symname, symname_len,
				  (void **) &symval) != 0)
		    {
		      lr_error (ldfile, _("\
%s: unknown symbol `%s' in equivalent definition"),
				"LC_COLLATE", symname);
		      goto col_sym_free;
		    }

		  if (insert_entry (&collate->sym_table,
				    newname, newname_len, symval) < 0)
		    {
		      lr_error (ldfile, _("\
error while adding equivalent collating symbol"));
		      goto sym_equiv_free;
		    }

		  free ((char *) symname);
		}
	      else
		{
		sym_equiv_free:
		  if (newname != NULL)
		    free ((char *) newname);
		  if (symname != NULL)
		    free ((char *) symname);
		}
	      lr_ignore_rest (ldfile, 1);
	    }
	  break;

	case tok_order_start:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 0 && state != 1)
	    goto err_label;
	  state = 1;

	  /* The 14652 draft does not specify whether all `order_start' lines
	     must contain the same number of sort-rules, but 14651 does.  So
	     we require this here as well.  */
	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok == tok_bsymbol)
	    {
	      /* This better should be a section name.  */
	      struct section_list *sp = collate->sections;
	      while (sp != NULL
		     && strcmp (sp->name, arg->val.str.startmb) != 0)
		sp = sp->next;

	      if (sp == NULL)
		{
		  lr_error (ldfile, _("\
%s: unknown section name `%s'"),
			    "LC_COLLATE", arg->val.str.startmb);
		  /* We use the error section.  */
		  collate->current_section = &collate->error_section;
		}
	      else
		{
		  /* Remember this section.  */
		  collate->current_section = sp;

		  /* One should not be allowed to open the same
                     section twice.  */
		  if (sp->first != NULL)
		    lr_error (ldfile, _("\
%s: multiple order definitions for section `%s'"),
			      "LC_COLLATE", sp->name);

		  /* Next should come the end of the line or a semicolon.  */
		  arg = lr_token (ldfile, charmap, repertoire);
		  if (arg->tok == tok_eol)
		    {
		      uint32_t cnt;

		      /* This means we have exactly one rule: `forward'.  */
		      if (collate->nrules > 1)
			lr_error (ldfile, _("\
%s: invalid number of sorting rules"),
				  "LC_COLLATE");
		      else
			collate->nrules = 1;
		      sp->rules = obstack_alloc (&collate->mempool,
						 (sizeof (enum coll_sort_rule)
						  * collate->nrules));
		      for (cnt = 0; cnt < collate->nrules; ++cnt)
			sp->rules[cnt] = sort_forward;

		      /* Next line.  */
		      break;
		    }

		  /* Get the next token.  */
		  arg = lr_token (ldfile, charmap, repertoire);
		}
	    }
	  else
	    {
	      /* There is no section symbol.  Therefore we use the unnamed
		 section.  */
	      collate->current_section = &collate->unnamed_section;

	      if (collate->unnamed_section.first != NULL)
		lr_error (ldfile, _("\
%s: multiple order definitions for unnamed section"),
			  "LC_COLLATE");
	    }

	  /* Now read the direction names.  */
	  read_directions (ldfile, arg, charmap, repertoire, collate);

	  /* From now be need the strings untranslated.  */
	  ldfile->translate_strings = 0;
	  break;

	case tok_order_end:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 1)
	    goto err_label;

	  /* Handle ellipsis at end of list.  */
	  if (was_ellipsis != tok_none)
	    {
	      handle_ellipsis (ldfile, NULL, was_ellipsis, charmap, repertoire,
			       collate);
	      was_ellipsis = tok_none;
	    }

	  state = 2;
	  lr_ignore_rest (ldfile, 1);
	  break;

	case tok_reorder_after:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state == 1)
	    {
	      lr_error (ldfile, _("%s: missing `order_end' keyword"),
			"LC_COLLATE");
	      state = 2;

	      /* Handle ellipsis at end of list.  */
	      if (was_ellipsis != tok_none)
		{
		  handle_ellipsis (ldfile, arg, was_ellipsis, charmap,
				   repertoire, collate);
		  was_ellipsis = tok_none;
		}
	    }
	  else if (state != 2 && state != 3)
	    goto err_label;
	  state = 3;

	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok == tok_bsymbol)
	    {
	      /* Find this symbol in the sequence table.  */
	      struct element_t *insp;
	      int no_error = 1;

	      if (find_entry (&collate->seq_table, arg->val.str.startmb,
			      arg->val.str.lenmb, (void **) &insp) == 0)
		/* Yes, the symbol exists.  Simply point the cursor
		   to it.  */
		  collate->cursor = insp;
	      else
		{
		  /* This is bad.  The symbol after which we have to
                     insert does not exist.  */
		  lr_error (ldfile, _("\
%s: cannot reorder after %.*s: symbol not known"),
			    "LC_COLLATE", arg->val.str.lenmb,
			    arg->val.str.startmb);
		  collate->cursor = NULL;
		  no_error = 0;
		}

	      lr_ignore_rest (ldfile, no_error);
	    }
	  else
	    /* This must not happen.  */
	    goto err_label;
	  break;

	case tok_reorder_end:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    break;

	  if (state != 3)
	    goto err_label;
	  state = 4;
	  lr_ignore_rest (ldfile, 1);
	  break;

	case tok_reorder_sections_after:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state == 1)
	    {
	      lr_error (ldfile, _("%s: missing `order_end' keyword"),
			"LC_COLLATE");
	      state = 2;

	      /* Handle ellipsis at end of list.  */
	      if (was_ellipsis != tok_none)
		{
		  handle_ellipsis (ldfile, NULL, was_ellipsis, charmap,
				   repertoire, collate);
		  was_ellipsis = tok_none;
		}
	    }
	  else if (state == 3)
	    {
	      error (0, 0, _("%s: missing `reorder-end' keyword"),
		     "LC_COLLATE");
	      state = 4;
	    }
	  else if (state != 2 && state != 4)
	    goto err_label;
	  state = 5;

	  /* Get the name of the sections we are adding after.  */
	  arg = lr_token (ldfile, charmap, repertoire);
	  if (arg->tok == tok_bsymbol)
	    {
	      /* Now find a section with this name.  */
	      struct section_list *runp = collate->sections;

	      while (runp != NULL)
		{
		  if (runp->name != NULL
		      && strlen (runp->name) == arg->val.str.lenmb
		      && memcmp (runp->name, arg->val.str.startmb,
				 arg->val.str.lenmb) == 0)
		    break;

		  runp = runp->next;
		}

	      if (runp != NULL)
		collate->current_section = runp;
	      else
		{
		  /* This is bad.  The section after which we have to
                     reorder does not exist.  Therefore we cannot
                     process the whole rest of this reorder
                     specification.  */
		  lr_error (ldfile, _("%s: section `%.*s' not known"),
			    "LC_COLLATE", arg->val.str.lenmb,
			    arg->val.str.startmb);

		  do
		    {
		      lr_ignore_rest (ldfile, 0);

		      now = lr_token (ldfile, charmap, NULL);
		    }
		  while (now->tok == tok_reorder_sections_after
			 || now->tok == tok_reorder_sections_end
			 || now->tok == tok_end);

		  /* Process the token we just saw.  */
		  nowtok = now->tok;
		  continue;
		}
	    }
	  else
	    /* This must not happen.  */
	    goto err_label;
	  break;

	case tok_reorder_sections_end:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    break;

	  if (state != 5)
	    goto err_label;
	  state = 6;
	  lr_ignore_rest (ldfile, 1);
	  break;

	case tok_bsymbol:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 1 && state != 3)
	    goto err_label;

	  if (state == 3)
	    {
	      /* It is possible that we already have this collation sequence.
		 In this case we move the entry.  */
	      struct element_t *seqp;

	      /* If the symbol after which we have to insert was not found
		 ignore all entries.  */
	      if (collate->cursor == NULL)
		{
		  lr_ignore_rest (ldfile, 0);
		  break;
		}

	      if (find_entry (&collate->seq_table, arg->val.str.startmb,
			      arg->val.str.lenmb, (void **) &seqp) == 0)
		{
		  /* Remove the entry from the old position.  */
		  if (seqp->last == NULL)
		    collate->start = seqp->next;
		  else
		    seqp->last->next = seqp->next;
		  if (seqp->next != NULL)
		    seqp->next->last = seqp->last;

		  /* We also have to check whether this entry is the
                     first or last of a section.  */
		  if (seqp->section->first == seqp)
		    {
		      if (seqp->section->first == seqp->section->last)
			/* This setion has no content anymore.  */
			seqp->section->first = seqp->section->last = NULL;
		      else
			seqp->section->first = seqp->next;
		    }
		  else if (seqp->section->last == seqp)
		    seqp->section->last = seqp->last;

		  /* Now insert it in the new place.  */
		  seqp->next = collate->cursor->next;
		  seqp->last = collate->cursor;
		  collate->cursor->next = seqp;
		  if (seqp->next != NULL)
		    seqp->next->last = seqp;

		  seqp->section = collate->cursor->section;
		  if (seqp->section->last == collate->cursor)
		    seqp->section->last = seqp;

		  break;
		}

	      /* Otherwise we just add a new entry.  */
	    }
	  else if (state == 5)
	    {
	      /* We are reordering sections.  Find the named section.  */
	      struct section_list *runp = collate->sections;
	      struct section_list *prevp = NULL;

	      while (runp != NULL)
		{
		  if (runp->name != NULL
		      && strlen (runp->name) == arg->val.str.lenmb
		      && memcmp (runp->name, arg->val.str.startmb,
				 arg->val.str.lenmb) == 0)
		    break;

		  prevp = runp;
		  runp = runp->next;
		}

	      if (runp == NULL)
		{
		  lr_error (ldfile, _("%s: section `%.*s' not known"),
			    "LC_COLLATE", arg->val.str.lenmb,
			    arg->val.str.startmb);
		  lr_ignore_rest (ldfile, 0);
		}
	      else
		{
		  if (runp != collate->current_section)
		    {
		      /* Remove the named section from the old place and
			 insert it in the new one.  */
		      prevp->next = runp->next;

		      runp->next = collate->current_section->next;
		      collate->current_section->next = runp;
		      collate->current_section = runp;
		    }

		  /* Process the rest of the line which might change
                     the collation rules.  */
		  arg = lr_token (ldfile, charmap, repertoire);
		  if (arg->tok != tok_eof && arg->tok != tok_eol)
		    read_directions (ldfile, arg, charmap, repertoire,
				     collate);
		}
	      break;
	    }
	  else if (was_ellipsis != tok_none)
	    {
	      /* Using the information in the `ellipsis_weight'
                 element and this and the last value we have to handle
                 the ellipsis now.  */
	      assert (state == 1);

	      handle_ellipsis (ldfile, arg, was_ellipsis, charmap, repertoire,
			       collate);

	      /* Remember that we processed the ellipsis.  */
	      was_ellipsis = tok_none;

	      /* And don't add the value a second time.  */
	      break;
	    }

	  /* Now insert in the new place.  */
	  insert_value (ldfile, arg, charmap, repertoire, collate);
	  break;

	case tok_undefined:
	  /* Ignore the rest of the line if we don't need the input of
	     this line.  */
	  if (ignore_content)
	    {
	      lr_ignore_rest (ldfile, 0);
	      break;
	    }

	  if (state != 1)
	    goto err_label;

	  if (was_ellipsis != tok_none)
	    {
	      lr_error (ldfile,
			_("%s: cannot have `%s' as end of ellipsis range"),
			"LC_COLLATE", "UNDEFINED");

	      unlink_element (collate);
	      was_ellipsis = tok_none;
	    }

	  /* See whether UNDEFINED already appeared somewhere.  */
	  if (collate->undefined.next != NULL
	      || (collate->cursor != NULL
		  && collate->undefined.next == collate->cursor))
	    {
	      lr_error (ldfile,
			_("%s: order for `%.*s' already defined at %s:%zu"),
			"LC_COLLATE", 9, "UNDEFINED", collate->undefined.file,
			collate->undefined.line);
	      lr_ignore_rest (ldfile, 0);
	    }
	  else
	    /* Parse the weights.  */
	     insert_weights (ldfile, &collate->undefined, charmap,
			     repertoire, collate, tok_none);
	  break;

	case tok_ellipsis2:
	case tok_ellipsis3:
	case tok_ellipsis4:
	  /* This is the symbolic (decimal or hexadecimal) or absolute
             ellipsis.  */
	  if (was_ellipsis != tok_none)
	    goto err_label;

	  if (state != 1 && state != 3)
	    goto err_label;

	  was_ellipsis = nowtok;

	  insert_weights (ldfile, &collate->ellipsis_weight, charmap,
			  repertoire, collate, nowtok);
	  break;

	case tok_end:
	  /* Next we assume `LC_COLLATE'.  */
	  if (!ignore_content)
	    {
	      if (state == 0)
		/* We must either see a copy statement or have
		   ordering values.  */
		lr_error (ldfile,
			  _("%s: empty category description not allowed"),
			  "LC_COLLATE");
	      else if (state == 1)
		{
		  lr_error (ldfile, _("%s: missing `order_end' keyword"),
			    "LC_COLLATE");

		  /* Handle ellipsis at end of list.  */
		  if (was_ellipsis != tok_none)
		    {
		      handle_ellipsis (ldfile, NULL, was_ellipsis, charmap,
				       repertoire, collate);
		      was_ellipsis = tok_none;
		    }
		}
	      else if (state == 3)
		error (0, 0, _("%s: missing `reorder-end' keyword"),
		       "LC_COLLATE");
	      else if (state == 5)
		error (0, 0, _("%s: missing `reorder-sections-end' keyword"),
		       "LC_COLLATE");
	    }
	  arg = lr_token (ldfile, charmap, NULL);
	  if (arg->tok == tok_eof)
	    break;
	  if (arg->tok == tok_eol)
	    lr_error (ldfile, _("%s: incomplete `END' line"), "LC_COLLATE");
	  else if (arg->tok != tok_lc_collate)
	    lr_error (ldfile, _("\
%1$s: definition does not end with `END %1$s'"), "LC_COLLATE");
	  lr_ignore_rest (ldfile, arg->tok == tok_lc_collate);
	  return;

	default:
	err_label:
	  SYNTAX_ERROR (_("%s: syntax error"), "LC_COLLATE");
	}

      /* Prepare for the next round.  */
      now = lr_token (ldfile, charmap, NULL);
      nowtok = now->tok;
    }

  /* When we come here we reached the end of the file.  */
  lr_error (ldfile, _("%s: premature end of file"), "LC_COLLATE");
}
