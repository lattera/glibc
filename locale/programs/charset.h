/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1996.

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

#ifndef _CHARSET_H
#define _CHARSET_H

#include <obstack.h>

#include "repertoire.h"
#include "simple-hash.h"
#include "linereader.h"


struct width_rule
{
  unsigned int from;
  unsigned int to;
  unsigned int width;
};


struct charset_t
{
  const char *code_set_name;
  int mb_cur_min;
  int mb_cur_max;

  struct width_rule *width_rules;
  size_t nwidth_rules;
  size_t nwidth_rules_max;
  unsigned int width_default;

  struct obstack mem_pool;
  hash_table char_table;
};


/* We need one value to mark the error case.  Let's use 0xffffffff.
   I.e., it is placed in the last page of ISO 10646.  For now only the
   first is used and we have plenty of room.  */
#define ILLEGAL_CHAR_VALUE ((wchar_t) 0xffffffffu)


/* Declared in localedef.c.  */
extern int be_quiet;

/* Prototypes for charmap handling functions.  */
struct charset_t *charmap_read (const char *filename);

/* Prototypes for function to insert new character.  */
void charset_new_char (struct linereader *lr, hash_table *ht, int bytes,
		       unsigned int value, const char *from, const char *to);

/* Return the value stored under the given key in the hashing table.  */
unsigned int charset_find_value (const hash_table *ht,
				 const char *name, size_t len);

#endif /* charset.h */
