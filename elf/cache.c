/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1999.

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

#define _GNU_SOURCE 1

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <dirent.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ldconfig.h"

#define CACHEMAGIC "ld.so-1.7.0"

struct cache_entry
{
  char *lib;
  char *path;
  int flags;
  struct cache_entry *next;
};

struct file_entry
{
  int flags;		/* This is 1 for an ELF library.  */
  unsigned int key, value; /* String table indices.  */
};


struct cache_file
{
  char magic[sizeof CACHEMAGIC - 1];
  unsigned int nlibs;
  struct file_entry libs[0];
};


/* List of all cache entries.  */
static struct cache_entry *entries;

static const char *flag_descr[] =
{ "libc4", "ELF", "libc5", "libc6"};


/* Print a single entry.  */
static void
print_entry (const char *lib, int flag, const char *key)
{
  printf ("\t%s (", lib);
  switch (flag)
    {
    case FLAG_LIBC4:
    case FLAG_ELF:
    case FLAG_ELF_LIBC5:
    case FLAG_ELF_LIBC6:
      fputs (flag_descr [flag & FLAG_TYPE_MASK], stdout);
      break;
    default:
      fputs ("unknown", stdout);
      break;
    }
  switch (flag & FLAG_REQUIRED_MASK)
    {
#ifdef __sparc__
    case FLAG_SPARC_LIB64:
      fputs (",64bit", stdout);
#endif
    case 0:
      break;
    default:
      fprintf (stdout, ",%d", flag & FLAG_REQUIRED_MASK);
      break;
    }
  printf (") => %s\n", key);
}


/* Print the whole cache file.  */
void
print_cache (const char *cache_name)
{
  size_t cache_size;
  struct stat st;
  int fd;
  unsigned int i;
  struct cache_file *cache;
  const char *cache_data;
  
  fd = open (cache_name, O_RDONLY);
  if (fd < 0)
    error (EXIT_FAILURE, errno, _("Can't open cache file %s\n"), cache_name);

  if (fstat (fd, &st) < 0
      /* No need to map the file if it is empty.  */
      || st.st_size == 0)
    {
      close (fd);
      return;
    }
  
  cache = mmap (0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (cache == MAP_FAILED)
    error (EXIT_FAILURE, errno, _("mmap of cache file failed.\n"));
  cache_size = st.st_size;

  if (cache_size < sizeof (struct cache_file)
      || memcmp (cache->magic, CACHEMAGIC, sizeof CACHEMAGIC - 1))
    return;
  /* This is where the strings start.  */
  cache_data = (const char *) &cache->libs[cache->nlibs];

  printf (_("%d libs found in cache `%s'\n"), cache->nlibs, cache_name);
  
  /* Print everything.  */
  for (i = 0; i < cache->nlibs; i++)
    print_entry (cache_data + cache->libs[i].key,
		 cache->libs[i].flags,
		 cache_data + cache->libs[i].value);

  /* Cleanup.  */
  munmap (cache, cache_size);
  close (fd);
}

/* Initialize cache data structures.  */
void
init_cache (void)
{
  entries = NULL;
}


/* Helper function which must match the one in the dynamic linker, so that
   we rely on the same sort order.  */
int
cache_libcmp (const char *p1, const char *p2)
{
  while (*p1 != '\0')
    {
      if (*p1 >= '0' && *p1 <= '9')
        {
          if (*p2 >= '0' && *p2 <= '9')
            {
	      /* Must compare this numerically.  */
	      int val1;
	      int val2;

	      val1 = *p1++ - '0';
	      val2 = *p2++ - '0';
	      while (*p1 >= '0' && *p1 <= '9')
	        val1 = val1 * 10 + *p1++ - '0';
	      while (*p2 >= '0' && *p2 <= '9')
	        val2 = val2 * 10 + *p2++ - '0';
	      if (val1 != val2)
		return val1 - val2;
	    }
	  else
            return 1;
        }
      else if (*p2 >= '0' && *p2 <= '9')
        return -1;
      else if (*p1 != *p2)
        return *p1 - *p2;
      else
	{
	  ++p1;
	  ++p2;
	}
    }
  return *p1 - *p2;
}

static
int compare (const struct cache_entry *e1, const struct cache_entry *e2)
{
  int res;
  
  /* We need to swap entries here to get the correct sort order.  */
  res = cache_libcmp (e2->lib, e1->lib);
  if (res == 0)
    {
      if (e1->flags < e2->flags)
	return 1;
      else if (e1->flags > e2->flags)
	return -1;
    }
  return res;
}


/* Save the contents of the cache.  */
void
save_cache (const char *cache_name)
{
  struct cache_entry *entry;
  int i, fd;
  size_t total_strlen, len;
  char *strings, *str, *temp_name;
  struct cache_file *file_entries;
  size_t file_entries_size;
  unsigned int str_offset;
  /* Number of cache entries.  */
  int cache_entry_count = 0;

  /* The cache entries are sorted already, save them in this order. */

  /* Count the length of all strings.  */
  total_strlen = 0;
  for (entry = entries; entry != NULL; entry = entry->next)
    {
      /* Account the final NULs.  */
      total_strlen += strlen (entry->lib) + strlen (entry->path) + 2;
      ++cache_entry_count;
    }
  
  /* Create the on disk cache structure.  */
  /* First an array for all strings.  */
  strings = (char *)xmalloc (total_strlen + 1);

  /* And the list of all entries.  */
  file_entries_size = sizeof (struct cache_file)
    + cache_entry_count * sizeof (struct file_entry);
  file_entries = (struct cache_file *) xmalloc (file_entries_size);

  /* Fill in the header.  */
  memset (file_entries, 0, sizeof (struct cache_file));
  memcpy (file_entries->magic, CACHEMAGIC, sizeof CACHEMAGIC - 1);
  
  file_entries->nlibs = cache_entry_count;

  str_offset = 0;
  str = strings;
  for (i = 0, entry = entries; entry != NULL; entry = entry->next, ++i)
    {
      file_entries->libs[i].flags = entry->flags;
      /* First the library.  */
      /* XXX: Actually we can optimize here and remove duplicates.  */
      file_entries->libs[i].key = str_offset;
      len = strlen (entry->lib);
      str = stpcpy (str, entry->lib);
      /* Account the final NUL.  */
      ++str;
      str_offset += len + 1;
      /* Then the path.  */
      file_entries->libs[i].value = str_offset;
      len = strlen (entry->path);
      str = stpcpy (str, entry->path);
      /* Account the final NUL.  */
      ++str;
      str_offset += len + 1;
    }
  assert (str_offset == total_strlen);

  /* Write out the cache.  */

  /* Write cache first to a temporary file and rename it later.  */
  temp_name = xmalloc (strlen (cache_name) + 2);
  sprintf (temp_name, "%s~", cache_name);
  /* First remove an old copy if it exists.  */
  if (unlink (temp_name) && errno != ENOENT)
    error (EXIT_FAILURE, errno, _("Can't remove old temporary cache file %s"),
	   temp_name);

  /* Create file.  */
  fd = open (temp_name, O_CREAT|O_WRONLY|O_TRUNC|O_NOFOLLOW, 0644);
  if (fd < 0)
    error (EXIT_FAILURE, errno, _("Can't create temporary cache file %s"),
	   temp_name);

  /* Write contents.  */
  if (write (fd, file_entries, file_entries_size) != (ssize_t)file_entries_size)
    error (EXIT_FAILURE, errno, _("Writing of cache data failed"));

  if (write (fd, strings, total_strlen) != (ssize_t)total_strlen)
    error (EXIT_FAILURE, errno, _("Writing of cache data failed."));

  close (fd);

  /* Move temporary to its final location.  */
  if (rename (temp_name, cache_name))
    error (EXIT_FAILURE, errno, _("Renaming of %s to %s failed"), temp_name,
	   cache_name);
  
  /* Free all allocated memory.  */
  free (file_entries);
  free (strings);

  while (entries)
    {
      entry = entries;
      free (entry->path);
      free (entry->lib);
      entries = entries->next;
      free (entry);
    }
}

/* Add one library to the cache.  */
void
add_to_cache (const char *path, const char *lib, int flags)
{
  struct cache_entry *new_entry, *ptr, *prev;
  char *full_path;
  int len;

  new_entry = (struct cache_entry *) xmalloc (sizeof (struct cache_entry));

  len = strlen (lib) + strlen (path) + 2;

  full_path = (char *) xmalloc (len);
  snprintf (full_path, len, "%s/%s", path, lib);
  
  new_entry->lib = xstrdup (lib);
  new_entry->path = full_path;
  new_entry->flags = flags;

  /* Keep the list sorted - search for right place to insert.  */
  ptr = entries;
  prev = entries;
  while (ptr != NULL)
    {
      if (compare (ptr, new_entry) > 0)
	break;
      prev = ptr;
      ptr = ptr->next;
    }
  /* Is this the first entry?  */
  if (ptr == entries)
    {
      new_entry->next = entries;
      entries = new_entry;
    }
  else
    {
      new_entry->next = prev->next;
      prev->next = new_entry;
    }
}
