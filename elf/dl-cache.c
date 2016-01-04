/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <unistd.h>
#include <ldsodefs.h>
#include <sys/mman.h>
#include <dl-cache.h>
#include <dl-procinfo.h>
#include <stdint.h>
#include <_itoa.h>

#ifndef _DL_PLATFORMS_COUNT
# define _DL_PLATFORMS_COUNT 0
#endif

/* This is the starting address and the size of the mmap()ed file.  */
static struct cache_file *cache;
static struct cache_file_new *cache_new;
static size_t cachesize;

/* 1 if cache_data + PTR points into the cache.  */
#define _dl_cache_verify_ptr(ptr) (ptr < cache_data_size)

#define SEARCH_CACHE(cache) \
/* We use binary search since the table is sorted in the cache file.	      \
   The first matching entry in the table is returned.			      \
   It is important to use the same algorithm as used while generating	      \
   the cache file.  */							      \
do									      \
  {									      \
    left = 0;								      \
    right = cache->nlibs - 1;						      \
									      \
    while (left <= right)						      \
      {									      \
	__typeof__ (cache->libs[0].key) key;				      \
									      \
	middle = (left + right) / 2;					      \
									      \
	key = cache->libs[middle].key;					      \
									      \
	/* Make sure string table indices are not bogus before using	      \
	   them.  */							      \
	if (! _dl_cache_verify_ptr (key))				      \
	  {								      \
	    cmpres = 1;							      \
	    break;							      \
	  }								      \
									      \
	/* Actually compare the entry with the key.  */			      \
	cmpres = _dl_cache_libcmp (name, cache_data + key);		      \
	if (__glibc_unlikely (cmpres == 0))				      \
	  {								      \
	    /* Found it.  LEFT now marks the last entry for which we	      \
	       know the name is correct.  */				      \
	    left = middle;						      \
									      \
	    /* There might be entries with this name before the one we	      \
	       found.  So we have to find the beginning.  */		      \
	    while (middle > 0)						      \
	      {								      \
		__typeof__ (cache->libs[0].key) key;			      \
									      \
		key = cache->libs[middle - 1].key;			      \
		/* Make sure string table indices are not bogus before	      \
		   using them.  */					      \
		if (! _dl_cache_verify_ptr (key)			      \
		    /* Actually compare the entry.  */			      \
		    || _dl_cache_libcmp (name, cache_data + key) != 0)	      \
		  break;						      \
		--middle;						      \
	      }								      \
									      \
	    do								      \
	      {								      \
		int flags;						      \
		__typeof__ (cache->libs[0]) *lib = &cache->libs[middle];      \
									      \
		/* Only perform the name test if necessary.  */		      \
		if (middle > left					      \
		    /* We haven't seen this string so far.  Test whether the  \
		       index is ok and whether the name matches.  Otherwise   \
		       we are done.  */					      \
		    && (! _dl_cache_verify_ptr (lib->key)		      \
			|| (_dl_cache_libcmp (name, cache_data + lib->key)    \
			    != 0)))					      \
		  break;						      \
									      \
		flags = lib->flags;					      \
		if (_dl_cache_check_flags (flags)			      \
		    && _dl_cache_verify_ptr (lib->value))		      \
		  {							      \
		    if (best == NULL || flags == GLRO(dl_correct_cache_id))   \
		      {							      \
			HWCAP_CHECK;					      \
			best = cache_data + lib->value;			      \
									      \
			if (flags == GLRO(dl_correct_cache_id))		      \
			  /* We've found an exact match for the shared	      \
			     object and no general `ELF' release.  Stop	      \
			     searching.  */				      \
			  break;					      \
		      }							      \
		  }							      \
	      }								      \
	    while (++middle <= right);					      \
	    break;							      \
	}								      \
									      \
	if (cmpres < 0)							      \
	  left = middle + 1;						      \
	else								      \
	  right = middle - 1;						      \
      }									      \
  }									      \
while (0)


int
internal_function
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


/* Look up NAME in ld.so.cache and return the file name stored there, or null
   if none is found.  The cache is loaded if it was not already.  If loading
   the cache previously failed there will be no more attempts to load it.
   The caller is responsible for freeing the returned string.  The ld.so.cache
   may be unmapped at any time by a completing recursive dlopen and
   this function must take care that it does not return references to
   any data in the mapping.  */
char *
internal_function
_dl_load_cache_lookup (const char *name)
{
  int left, right, middle;
  int cmpres;
  const char *cache_data;
  uint32_t cache_data_size;
  const char *best;

  /* Print a message if the loading of libs is traced.  */
  if (__glibc_unlikely (GLRO(dl_debug_mask) & DL_DEBUG_LIBS))
    _dl_debug_printf (" search cache=%s\n", LD_SO_CACHE);

  if (cache == NULL)
    {
      /* Read the contents of the file.  */
      void *file = _dl_sysdep_read_whole_file (LD_SO_CACHE, &cachesize,
					       PROT_READ);

      /* We can handle three different cache file formats here:
	 - the old libc5/glibc2.0/2.1 format
	 - the old format with the new format in it
	 - only the new format
	 The following checks if the cache contains any of these formats.  */
      if (file != MAP_FAILED && cachesize > sizeof *cache
	  && memcmp (file, CACHEMAGIC, sizeof CACHEMAGIC - 1) == 0)
	{
	  size_t offset;
	  /* Looks ok.  */
	  cache = file;

	  /* Check for new version.  */
	  offset = ALIGN_CACHE (sizeof (struct cache_file)
				+ cache->nlibs * sizeof (struct file_entry));

	  cache_new = (struct cache_file_new *) ((void *) cache + offset);
	  if (cachesize < (offset + sizeof (struct cache_file_new))
	      || memcmp (cache_new->magic, CACHEMAGIC_VERSION_NEW,
			 sizeof CACHEMAGIC_VERSION_NEW - 1) != 0)
	    cache_new = (void *) -1;
	}
      else if (file != MAP_FAILED && cachesize > sizeof *cache_new
	       && memcmp (file, CACHEMAGIC_VERSION_NEW,
			  sizeof CACHEMAGIC_VERSION_NEW - 1) == 0)
	{
	  cache_new = file;
	  cache = file;
	}
      else
	{
	  if (file != MAP_FAILED)
	    __munmap (file, cachesize);
	  cache = (void *) -1;
	}

      assert (cache != NULL);
    }

  if (cache == (void *) -1)
    /* Previously looked for the cache file and didn't find it.  */
    return NULL;

  best = NULL;

  if (cache_new != (void *) -1)
    {
      uint64_t platform;

      /* This is where the strings start.  */
      cache_data = (const char *) cache_new;

      /* Now we can compute how large the string table is.  */
      cache_data_size = (const char *) cache + cachesize - cache_data;

      platform = _dl_string_platform (GLRO(dl_platform));
      if (platform != (uint64_t) -1)
	platform = 1ULL << platform;

#define _DL_HWCAP_TLS_MASK (1LL << 63)
      uint64_t hwcap_exclude = ~((GLRO(dl_hwcap) & GLRO(dl_hwcap_mask))
				 | _DL_HWCAP_PLATFORM | _DL_HWCAP_TLS_MASK);

      /* Only accept hwcap if it's for the right platform.  */
#define HWCAP_CHECK \
      if (lib->hwcap & hwcap_exclude)					      \
	continue;							      \
      if (GLRO(dl_osversion) && lib->osversion > GLRO(dl_osversion))	      \
	continue;							      \
      if (_DL_PLATFORMS_COUNT						      \
	  && (lib->hwcap & _DL_HWCAP_PLATFORM) != 0			      \
	  && (lib->hwcap & _DL_HWCAP_PLATFORM) != platform)		      \
	continue
      SEARCH_CACHE (cache_new);
    }
  else
    {
      /* This is where the strings start.  */
      cache_data = (const char *) &cache->libs[cache->nlibs];

      /* Now we can compute how large the string table is.  */
      cache_data_size = (const char *) cache + cachesize - cache_data;

#undef HWCAP_CHECK
#define HWCAP_CHECK do {} while (0)
      SEARCH_CACHE (cache);
    }

  /* Print our result if wanted.  */
  if (__builtin_expect (GLRO(dl_debug_mask) & DL_DEBUG_LIBS, 0)
      && best != NULL)
    _dl_debug_printf ("  trying file=%s\n", best);

  if (best == NULL)
    return NULL;

  /* The double copy is *required* since malloc may be interposed
     and call dlopen itself whose completion would unmap the data
     we are accessing. Therefore we must make the copy of the
     mapping data without using malloc.  */
  char *temp;
  temp = alloca (strlen (best) + 1);
  strcpy (temp, best);
  return strdup (temp);
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
