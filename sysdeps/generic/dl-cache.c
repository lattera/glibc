/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <unistd.h>
#include <elf/ldsodefs.h>
#include <sys/mman.h>

/* System-dependent function to read a file's whole contents
   in the most convenient manner available.  */
extern void *_dl_sysdep_read_whole_file (const char *filename,
					 size_t *filesize_ptr,
					 int mmap_prot);

#ifndef LD_SO_CACHE
# define LD_SO_CACHE "/etc/ld.so.cache"
#endif

#define CACHEMAGIC "ld.so-1.7.0"

struct cache_file
  {
    char magic[sizeof CACHEMAGIC - 1];
    unsigned int nlibs;
    struct
      {
	int flags;		/* This is 1 for an ELF library.  */
	unsigned int key, value; /* String table indices.  */
      } libs[0];
  };

/* This is the starting address and the size of the mmap()ed file.  */
static struct cache_file *cache;
static size_t cachesize;

/* This is the cache ID we expect.  Normally it is 3 for glibc linked
   binaries.  */
int _dl_correct_cache_id = 3;

/* Helper function which must match the one in ldconfig, so that
   we rely on the same sort order.  */
static int
_dl_cache_libcmp (const char *p1, const char *p2)
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


/* Look up NAME in ld.so.cache and return the file name stored there,
   or null if none is found.  */

const char *
_dl_load_cache_lookup (const char *name)
{
  int left, right, middle;
  int cmpres;
  const char *cache_data;
  const char *best;

  /* Print a message if the loading of libs is traced.  */
  if (_dl_debug_libs)
    _dl_debug_message (1, " search cache=", LD_SO_CACHE, "\n", NULL);

  if (cache == NULL)
    {
      /* Read the contents of the file.  */
      void *file = _dl_sysdep_read_whole_file (LD_SO_CACHE, &cachesize,
					       PROT_READ);
      if (file && cachesize > sizeof *cache &&
	  !memcmp (file, CACHEMAGIC, sizeof CACHEMAGIC - 1))
	/* Looks ok.  */
	cache = file;
      else
	{
	  if (file)
	    __munmap (file, cachesize);
	  cache = (void *) -1;
	  return NULL;
	}
    }

  if (cache == (void *) -1)
    /* Previously looked for the cache file and didn't find it.  */
    return NULL;

  /* This is where the strings start.  */
  cache_data = (const char *) &cache->libs[cache->nlibs];

  best = NULL;

  /* We use binary search since the table is sorted in the cache file.
     It is important to use the same algorithm as used while generating
     the cache file.  */
  left = 0;
  right = cache->nlibs - 1;
  middle = (left + right) / 2;
  cmpres = 1;

  while (left <= right)
    {
      /* Make sure string table indices are not bogus before using them.  */
      if (cache->libs[middle].key >= cachesize - sizeof *cache)
	{
	  cmpres = 1;
	  break;
	}

      /* Actually compare the entry with the key.  */
      cmpres = _dl_cache_libcmp (name, cache_data + cache->libs[middle].key);
      if (cmpres == 0)
	/* Found it.  */
	break;

      if (cmpres < 0)
	left = middle + 1;
      else
	right = middle - 1;

      middle = (left + right) / 2;
    }

  if (cmpres == 0)
    {
      /* LEFT now marks the last entry for which we know the name is
	 correct.  */
      left = middle;

      /* There might be entries with this name before the one we
         found.  So we have to find the beginning.  */
      while (middle > 0
	     /* Make sure string table indices are not bogus before
                using them.  */
	     && cache->libs[middle - 1].key < cachesize - sizeof *cache
	     /* Actually compare the entry.  */
	     && (_dl_cache_libcmp (name,
				   cache_data + cache->libs[middle - 1].key)
		 == 0))
	--middle;

      do
	{
	  int flags;

	  /* Only perform the name test if necessary.  */
	  if (middle > left
	      /* We haven't seen this string so far.  Test whether the
		 index is ok and whether the name matches.  Otherwise
		 we are done.  */
	      && (cache->libs[middle].key >= cachesize - sizeof *cache
		  || (_dl_cache_libcmp (name,
					cache_data + cache->libs[middle].key)
		      != 0)))
	    break;

	  flags = cache->libs[middle].flags;
	  if ((flags == 1 || flags == 3)
	      && cache->libs[middle].value < cachesize - sizeof *cache)
	    {
	      if (best == NULL || flags == _dl_correct_cache_id)
		{
		  best = cache_data + cache->libs[middle].value;

		  if (flags == _dl_correct_cache_id)
		    /* We've found an exact match for the shared
		       object and no general `ELF' release.  Stop
		       searching.  */
		    break;
		}
	    }
	}
      while (++middle <= right);
    }

  /* Print our result if wanted.  */
  if (_dl_debug_libs && best != NULL)
    _dl_debug_message (1, "  trying file=", best, "\n", NULL);

  return best;
}

#ifndef MAP_COPY
/* If the system does not support MAP_COPY we cannot leave the file open
   all the time since this would create problems when the file is replaced.
   Therefore we provide this function to close the file and open it again
   once needed.  */
void
_dl_unload_cache (void)
{
  if (cache != NULL && cache != (struct cache_file *) -1)
    {
      __munmap (cache, cachesize);
      cache = NULL;
    }
}
#endif
