/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef _SIMPLE_HASH_H
#define _SIMPLE_HASH_H

#include <obstack.h>

typedef struct hash_table
{
  unsigned long size;
  unsigned long filled;
  void *first;
  void *table;
  struct obstack mem_pool;
}
hash_table;


int init_hash __P ((hash_table *htab, unsigned long int init_size));
int delete_hash __P ((hash_table *htab));
int insert_entry __P ((hash_table *htab, const void *key, size_t keylen,
		       void *data));
int find_entry __P ((hash_table *htab, const void *key, size_t keylen,
		     void **result));
int set_entry __P ((hash_table *htab, const void *key, size_t keylen,
		    void *newval));

int iterate_table __P ((hash_table *htab, void **ptr,
			const void **key, size_t *keylen, void **data));

unsigned long next_prime __P ((unsigned long int seed));

#endif /* simple-hash.h */
