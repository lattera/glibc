/* Copyright (C) 1995 Free Software Foundation, Inc.

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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _HASH_H
#define _HASH_H

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


int init_hash (hash_table *htab, unsigned long init_size);
int delete_hash(hash_table *htab);
int insert_entry (hash_table *htab, const char *key, size_t keylen,
		  void *data);
int find_entry (hash_table *htab, const char *key, size_t keylen,
		void **result);

int iterate_table (hash_table *htab, void **ptr, void **result);

#endif /* hash.h */
/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */

