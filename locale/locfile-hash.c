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

#include <obstack.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include "hash.h"

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free

void *xmalloc (size_t n);

typedef struct hash_entry
  {
    int used;
    char *key;
    void *data;
    struct hash_entry *next;
  }
hash_entry;

/* Prototypes for local functions.  */
static size_t lookup (hash_table *htab, const char *key, size_t keylen,
		      unsigned long hval);
static unsigned long compute_hashval(const char *key, size_t keylen);
static unsigned long next_prime(unsigned long seed);
static int is_prime(unsigned long candidate);


int
init_hash(hash_table *htab, unsigned long init_size)
{
  /* We need the size to be a prime.  */
  init_size = next_prime (init_size);

  /* Initialize the data structure.  */
  htab->size = init_size;
  htab->filled = 0;
  htab->first = NULL;
  htab->table = calloc (init_size + 1, sizeof (hash_entry));
  obstack_init (&htab->mem_pool);

  return htab->table == NULL;
}


int
delete_hash(hash_table *htab)
{
  free (htab->table);
  obstack_free (&htab->mem_pool, NULL);
  return 0;
}


int
insert_entry (hash_table *htab, const char *key, size_t keylen, void *data)
{
  unsigned long hval = compute_hashval (key, keylen);
  hash_entry *table = (hash_entry *) htab->table;
  size_t idx = lookup (htab, key, keylen, hval);

  if (table[idx].used)
    /* We don't want to overwrite the old value.  */
    return 1;
  else
    {
      hash_entry **p;

      /* An empty bucket has been found.  */
      table[idx].used = hval;
      table[idx].key = obstack_copy0 (&htab->mem_pool, key, keylen);
      table[idx].data = data;

      /* List the new value in the ordered list.  */
      for (p = (hash_entry **) &htab->first; *p != NULL && (*p)->data < data;
	   p = &(*p)->next);
      if (*p == NULL || (*p)->data > data)
	/* Insert new value in the list.  */
	{
	  table[idx].next = *p;
	  *p = &table[idx];
	}

      ++htab->filled;
      if (100 * htab->filled > 90 * htab->size)
	{
	  /* Resize the table.  */
	  unsigned long old_size = htab->size;

	  htab->size = next_prime (htab->size * 2);
	  htab->filled = 0;
	  htab->first = NULL;
	  htab->table = calloc (htab->size, sizeof (hash_entry));

	  for (idx = 1; idx <= old_size; ++idx)
	    if (table[idx].used)
	      insert_entry (htab, table[idx].key, strlen(table[idx].key),
			    table[idx].data);

	  free (table);
	}
      return 0;
    }
  /* NOTREACHED */
}


int
find_entry (hash_table *htab, const char *key, size_t keylen, void **result)
{
  hash_entry *table = (hash_entry *) htab->table;
  size_t idx = lookup (htab, key, keylen, compute_hashval (key, keylen));
  int retval;

  retval = table[idx].used;
  *result = retval ? table[idx].data : NULL;

  return retval;
}


int
iterate_table (hash_table *htab, void **ptr, void **result)
{
  if (*ptr == NULL)
    *ptr = (void *) htab->first;
  else
    {
      *ptr = (void *) (((hash_entry *) *ptr)->next);
      if (*ptr == NULL)
	return 0;
    }

  *result = ((hash_entry *) *ptr)->data;
  return 1;
}


static size_t
lookup (hash_table *htab, const char *key, size_t keylen, unsigned long hval)
{
  unsigned long hash;
  size_t idx;
  hash_entry *table = (hash_entry *) htab->table;

  /* First hash function: simply take the modul but prevent zero.  */
  hash = 1 + hval % htab->size;

  idx = hash;
  
  if (table[idx].used)
    {
      if (table[idx].used == hval && table[idx].key[keylen] == '\0'
	  && strncmp (key, table[idx].key, keylen) == 0)
	return idx;

      /* Second hash function as suggested in [Knuth].  */
      hash = 1 + hash % (htab->size - 2);

      do
	{
	  if (idx <= hash)
	    idx = htab->size + idx - hash;
	  else
	    idx -= hash;

	  /* If entry is found use it.  */
	  if (table[idx].used == hval && table[idx].key[keylen] == '\0'
	      && strncmp (key, table[idx].key, keylen) == 0)
	    return idx;
	}
      while (table[idx].used);
    }
  return idx;
}


static unsigned long
compute_hashval(const char *key, size_t keylen)
{
  size_t cnt;
  unsigned long hval, g;
  /* Compute the hash value for the given string.  */
  cnt = 0;
  hval = keylen;
  while (cnt < keylen)
    {
      hval <<= 4;
      hval += key[cnt++];
      g = hval & (0xf << (LONGBITS - 4));
      if (g != 0)
	{
	  hval ^= g >> (LONGBITS - 8);
	  hval ^= g;
	}
    }
  return hval;
}


static unsigned long
next_prime(unsigned long seed)
{
  /* Make it definitely odd.  */
  seed |= 1;

  while (!is_prime (seed))
    seed += 2;

  return seed;
}


static int
is_prime(unsigned long candidate)
{
  /* No even number and none less than 10 will be passwd here.  */
  unsigned long div = 3;
  unsigned long sq = div * div;

  while (sq < candidate && candidate % div != 0)
    {
      ++div;
      sq += 4 * div;
      ++div;
    }

  return candidate % div != 0;
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
