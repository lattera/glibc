/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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

#include <alloca.h>
#include <langinfo.h>
#include "localeinfo.h"

#ifndef STRING_TYPE
# error STRING_TYPE not defined
#endif

#ifndef USTRING_TYPE
# error USTRING_TYPE not defined
#endif

typedef struct weight_t
{
  struct weight_t *prev;
  struct weight_t *next;
  struct data_pair
    {
      int number;
      const u_int32_t *value;
    } data[0];
} weight_t;


/* The following five macros grant access to the values in the
   collate locale file that do not depend on byte order.  */
#ifndef USE_IN_EXTENDED_LOCALE_MODEL
# define collate_nrules \
  (_NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES))
# define collate_hash_size \
  (_NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_HASH_SIZE))
# define collate_hash_layers \
  (_NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_HASH_LAYERS))
# define collate_undefined \
  (_NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_UNDEFINED))
# define collate_rules \
  ((u_int32_t *) _NL_CURRENT (LC_COLLATE, _NL_COLLATE_RULES))

static __inline void get_weight (const STRING_TYPE **str, weight_t *result);
static __inline void
get_weight (const STRING_TYPE **str, weight_t *result)
#else
# define collate_nrules \
  current->values[_NL_ITEM_INDEX (_NL_COLLATE_NRULES)].word
# define collate_hash_size \
  current->values[_NL_ITEM_INDEX (_NL_COLLATE_HASH_SIZE)].word
# define collate_hash_layers \
  current->values[_NL_ITEM_INDEX (_NL_COLLATE_HASH_LAYERS)].word
# define collate_undefined \
  current->values[_NL_ITEM_INDEX (_NL_COLLATE_UNDEFINED)].word
# define collate_rules \
  ((u_int32_t *) current->values[_NL_ITEM_INDEX (_NL_COLLATE_RULES)].string)

static __inline void get_weight (const STRING_TYPE **str, weight_t *result,
				 struct locale_data *current,
				 const u_int32_t *__collate_table,
				 const u_int32_t *__collate_extra);
static __inline void
get_weight (const STRING_TYPE **str, weight_t *result,
	    struct locale_data *current, const u_int32_t *__collate_table,
	    const u_int32_t *__collate_extra)
#endif
{
  unsigned int ch = *((USTRING_TYPE *) (*str))++;
  size_t slot;

  if (sizeof (STRING_TYPE) == 1)
    slot = ch * (collate_nrules + 1);
  else
    {
      const size_t level_size = collate_hash_size * (collate_nrules + 1);
      size_t level;

      slot = (ch % collate_hash_size) * (collate_nrules + 1);

      level = 0;
      while (__collate_table[slot] != (u_int32_t) ch)
	{
	  if (__collate_table[slot + 1] == 0
	      || ++level >= collate_hash_layers)
	    {
	      size_t idx = collate_undefined;
	      size_t cnt;

	      for (cnt = 0; cnt < collate_nrules; ++cnt)
		{
		  result->data[cnt].number = __collate_extra[idx++];
		  result->data[cnt].value = &__collate_extra[idx];
		  idx += result->data[cnt].number;
		}
	      return;
	    }
	  slot += level_size;
	}
    }

  if (__collate_table[slot + 1] != (u_int32_t) FORWARD_CHAR)
    {
      /* We have a simple form.  One value for each weight.  */
      size_t cnt;

      for (cnt = 0; cnt < collate_nrules; ++cnt)
	{
	  result->data[cnt].number = 1;
	  result->data[cnt].value = &__collate_table[slot + 1 + cnt];
	}
      return;
    }

  /* We now look for any collation element which starts with CH.
     There might none, but the last list member is a catch-all case
     because it is simple the character CH.  The value of this entry
     might be the same as UNDEFINED.  */
  slot = __collate_table[slot + 2];

  while (1)
    {
      size_t idx;

      /* This is a comparison between a u_int32_t array (aka wchar_t) and
	 an 8-bit string.  */
      for (idx = 0; __collate_extra[slot + 2 + idx] != 0; ++idx)
	if (__collate_extra[slot + 2 + idx] != ((USTRING_TYPE *) *str)[idx])
	  break;

      /* When the loop finished with all characters of the collation
	 element used, we found the longest prefix.  */
      if (__collate_extra[slot + 2 + idx] == 0)
	{
	  size_t cnt;

	  *str += idx;
	  idx += slot + 3;
	  for (cnt = 0; cnt < collate_nrules; ++cnt)
	    {
	      result->data[cnt].number = __collate_extra[idx++];
	      result->data[cnt].value = &__collate_extra[idx];
	      idx += result->data[cnt].number;
	    }
	  return;
	}

      /* To next entry in list.  */
      slot += __collate_extra[slot];
    }
}


/* To process a string efficiently we retrieve all information about
   the string at once.  The following macro constructs a double linked
   list of this information.  It is a macro because we use `alloca'
   and we use a double linked list because of the backward collation
   order.

   We have this strange extra macro since the functions which use the
   given locale (not the global one) cannot use the global tables.  */
#ifndef USE_IN_EXTENDED_LOCALE_MODEL
# define call_get_weight(strp, newp) get_weight (strp, newp)
#else
# define call_get_weight(strp, newp) \
  get_weight (strp, newp, current, collate_table, collate_extra)
#endif

#define get_string(str, forw, backw) \
  do									      \
    {									      \
      weight_t *newp;							      \
      while (*str != '\0')						      \
	{								      \
	  newp = (weight_t *) alloca (sizeof (weight_t)			      \
				      + (collate_nrules			      \
					 * sizeof (struct data_pair)));	      \
									      \
	  newp->prev = backw;						      \
	  if (backw == NULL)						      \
	    forw = newp;						      \
	  else								      \
	    backw->next = newp;						      \
	  newp->next = NULL;						      \
	  backw = newp;							      \
	  call_get_weight (&str, newp);					      \
	}								      \
    }									      \
  while (0)
