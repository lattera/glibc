/* Cache handling for iconv modules.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <gconv_int.h>
#include <iconvconfig.h>

#include "../intl/hash-string.h"

static void *cache;
static size_t cache_size;
static int cache_malloced;


int
internal_function
__gconv_load_cache (void)
{
  int fd;
  struct stat64 st;
  struct gconvcache_header *header;

  /* We cannot use the cache if the GCONV_PATH environment variable is
     set.  */
  __gconv_path_envvar = getenv ("GCONV_PATH");
  if (__gconv_path_envvar != NULL)
    return -1;

  /* See whether the cache file exists.  */
  fd = __open (GCONV_MODULES_CACHE, O_RDONLY);
  if (__builtin_expect (fd, 0) == -1)
    /* Not available.  */
    return -1;

  /* Get information about the file.  */
  if (__builtin_expect (__fxstat64 (_STAT_VER, fd, &st), 0) < 0
      /* We do not have to start looking at the file if it cannot contain
	 at least the cache header.  */
      || st.st_size < sizeof (struct gconvcache_header))
    {
    close_and_exit:
      close (fd);
      return -1;
    }

  /* Make the file content available.  */
  cache_size = st.st_size;
#ifdef _POSIX_MAPPED_FILES
  cache = __mmap (NULL, cache_size, PROT_READ, MAP_SHARED, fd, 0);
  if (__builtin_expect (cache == MAP_FAILED, 0))
#endif
    {
      size_t already_read;

      cache = malloc (cache_size);
      if (cache == NULL)
	goto close_and_exit;

      already_read = 0;
      do
	{
	  ssize_t n = __read (fd, (char *) cache + already_read,
			      cache_size - already_read);
	  if (__builtin_expect (n, 0) == -1)
	    {
	      free (cache);
	      cache = NULL;
	      goto close_and_exit;
	    }

	  already_read += n;
	}
      while (already_read < cache_size);

      cache_malloced = 1;
    }

  /* We don't need the file descriptor anymore.  */
  close (fd);

  /* Check the consistency.  */
  header = (struct gconvcache_header *) cache;
  if (__builtin_expect (header->magic, GCONVCACHE_MAGIC) != GCONVCACHE_MAGIC
      || __builtin_expect (header->string_offset >= cache_size, 0)
      || __builtin_expect (header->hash_offset >= cache_size, 0)
      || __builtin_expect (header->hash_size == 0, 0)
      || __builtin_expect ((header->hash_offset
			    + header->hash_size * sizeof (struct hash_entry))
			   > cache_size, 0)
      || __builtin_expect (header->module_offset >= cache_size, 0)
      || __builtin_expect (header->otherconv_offset > cache_size, 0))
    {
      if (cache_malloced)
	{
	  free (cache);
	  cache_malloced = 0;
	}
#ifdef _POSIX_MAPPED_FILES
      else
	munmap (cache, cache_size);
#endif
      cache = NULL;

      return -1;
    }

  /* That worked.  */
  return 0;
}


static int
internal_function
find_module_idx (const char *str, size_t *idxp)
{
  unsigned int idx;
  unsigned int hval;
  unsigned int hval2;
  const struct gconvcache_header *header;
  const char *strtab;
  const struct hash_entry *hashtab;
  unsigned int limit;

  header = (const struct gconvcache_header *) cache;
  strtab = (char *) cache + header->string_offset;
  hashtab = (struct hash_entry *) ((char *) cache + header->hash_offset);

  hval = hash_string (str);
  idx = hval % header->hash_size;
  hval2 = 1 + hval % (header->hash_size - 2);

  limit = cache_size - header->string_offset;
  while (hashtab[idx].string_offset != 0)
    if (hashtab[idx].string_offset < limit
	&& strcmp (str, strtab + hashtab[idx].string_offset) == 0)
      {
	*idxp = hashtab[idx].module_idx;
	return 0;
      }
    else
      if ((idx += hval2) >= header->hash_size)
	idx -= header->hash_size;

  /* Nothing found.  */
  return -1;
}


static int
internal_function
find_module (const char *directory, const char *filename,
	     struct __gconv_step *result)
{
  size_t dirlen = strlen (directory);
  size_t fnamelen = strlen (filename) + 1;
  char *fullname;
  int status = __GCONV_NOCONV;

  fullname = (char *) malloc (dirlen + fnamelen);
  if (fullname == NULL)
    return __GCONV_NOMEM;

  memcpy (__mempcpy (fullname, directory, dirlen), filename, fnamelen);

  result->__shlib_handle = __gconv_find_shlib (fullname);
  if (result->__shlib_handle != NULL)
    {
      status = __GCONV_OK;

      result->__modname = fullname;
      result->__fct = result->__shlib_handle->fct;
      result->__init_fct = result->__shlib_handle->init_fct;
      result->__end_fct = result->__shlib_handle->end_fct;

      result->__data = NULL;
      if (result->__init_fct != NULL)
	status = DL_CALL_FCT (result->__init_fct, (result));
    }

  if (__builtin_expect (status, __GCONV_OK) != __GCONV_OK)
    free (fullname);

  return status;
}


int
internal_function
__gconv_lookup_cache (const char *toset, const char *fromset,
		      struct __gconv_step **handle, size_t *nsteps, int flags)
{
  const struct gconvcache_header *header;
  const char *strtab;
  size_t fromidx;
  size_t toidx;
  const struct module_entry *modtab;
  const struct module_entry *from_module;
  const struct module_entry *to_module;
  struct __gconv_step *result;

  if (cache == NULL)
    /* We have no cache available.  */
    return __GCONV_NODB;

  header = (const struct gconvcache_header *) cache;
  strtab = (char *) cache + header->string_offset;
  modtab = (const struct module_entry *) ((char *) cache
					  + header->module_offset);

  if (find_module_idx (fromset, &fromidx) != 0
      || (header->module_offset + (fromidx + 1) * sizeof (struct module_entry)
	  > cache_size))
    return __GCONV_NOCONV;
  from_module = &modtab[fromidx];

  if (find_module_idx (toset, &toidx) != 0
      || (header->module_offset + (toidx + 1) * sizeof (struct module_entry)
	  > cache_size))
    return __GCONV_NOCONV;
  to_module = &modtab[toidx];

  /* Avoid copy-only transformations if the user requests.   */
  if (__builtin_expect (flags & GCONV_AVOID_NOCONV, 0) && fromidx == toidx)
    return __GCONV_NOCONV;

  /* If there are special conversions available examine them first.  */
  if (fromidx != 0 && toidx != 0
      && __builtin_expect (from_module->extra_offset, 0) != 0)
    {
      /* Search through the list to see whether there is a module
	 matching the destination character set.  */
      const struct extra_entry *extra;

      /* Note the -1.  This is due to the offset added in iconvconfig.
	 See there for more explanations.  */
      extra = (const struct extra_entry *) ((char *) cache
					    + header->otherconv_offset
					    + from_module->extra_offset - 1);
      while (extra->module_cnt != 0
	     && extra->module[extra->module_cnt - 1].outname_offset != toidx)
	extra = (const struct extra_entry *) ((char *) extra
					      + sizeof (struct extra_entry)
					      + (extra->module_cnt
						 * sizeof (struct extra_entry_module)));

      if (extra->module_cnt != 0)
	{
	  /* Use the extra module.  First determine how many steps.  */
	  char *fromname;
	  int idx;

	  *nsteps = extra->module_cnt;
	  *handle = result =
	    (struct __gconv_step *) malloc (extra->module_cnt
					    * sizeof (struct __gconv_step));
	  if (result == NULL)
	    return __GCONV_NOMEM;

	  fromname = (char *) strtab + from_module->canonname_offset;
	  idx = 0;
	  do
	    {
	      result[idx].__from_name = fromname;
	      fromname = result[idx].__to_name =
		(char *) strtab + modtab[extra->module[idx].outname_offset].canonname_offset;

	      result[idx].__counter = 1;
	      result[idx].__data = NULL;

#ifndef STATIC_GCONV
	      if (strtab[extra->module[idx].dir_offset] != '\0')
		{
		  /* Load the module, return handle for it.  */
		  int res;

		  res = find_module (strtab + extra->module[idx].dir_offset,
				     strtab + extra->module[idx].name_offset,
				     &result[idx]);
		  if (__builtin_expect (res, __GCONV_OK) != __GCONV_OK)
		    {
		      /* Something went wrong.  */
		      free (result);
		      goto try_internal;
		    }
		}
	      else
#endif
		/* It's a builtin transformation.  */
		__gconv_get_builtin_trans (strtab
					   + extra->module[idx].name_offset,
					   &result[idx]);

	    }
	  while (++idx < extra->module_cnt);

	  return __GCONV_OK;
	}
    }

 try_internal:
  /* See whether we can convert via the INTERNAL charset.  */
  if ((fromidx != 0 && __builtin_expect (from_module->fromname_offset, 1) == 0)
      || (toidx != 0 && __builtin_expect (to_module->toname_offset, 1) == 0)
      || (fromidx == 0 && toidx == 0))
    /* Not possible.  Nothing we can do.  */
    return __GCONV_NOCONV;

  /* We will use up to two modules.  Always allocate room for two.  */
  result = (struct __gconv_step *) malloc (2 * sizeof (struct __gconv_step));
  if (result == NULL)
    return __GCONV_NOMEM;

  *handle = result;
  *nsteps = 0;

  /* Generate data structure for conversion to INTERNAL.  */
  if (fromidx != 0)
    {
      result[0].__from_name = (char *) strtab + from_module->canonname_offset;
      result[0].__to_name = (char *) "INTERNAL";

      result[0].__counter = 1;
      result[0].__data = NULL;

#ifndef STATIC_GCONV
      if (strtab[from_module->todir_offset] != '\0')
	{
	  /* Load the module, return handle for it.  */
	  int res = find_module (strtab + from_module->todir_offset,
				 strtab + from_module->toname_offset,
				 &result[0]);
	  if (__builtin_expect (res, __GCONV_OK) != __GCONV_OK)
	    {
	      /* Something went wrong.  */
	      free (result);
	      return res;
	    }
	}
      else
#endif
	/* It's a builtin transformation.  */
	__gconv_get_builtin_trans (strtab + from_module->toname_offset,
				   &result[0]);

      ++*nsteps;
    }

  /* Generate data structure for conversion from INTERNAL.  */
  if (toidx != 0)
    {
      int idx = *nsteps;

      result[idx].__from_name = (char *) "INTERNAL";
      result[idx].__to_name = (char *) strtab + to_module->canonname_offset;

      result[idx].__counter = 1;
      result[idx].__data = NULL;

#ifndef STATIC_GCONV
      if (strtab[to_module->fromdir_offset] != '\0')
	{
	  /* Load the module, return handle for it.  */
	  int res = find_module (strtab + to_module->fromdir_offset,
				 strtab + to_module->fromname_offset,
				 &result[idx]);
	  if (__builtin_expect (res, __GCONV_OK) != __GCONV_OK)
	    {
	      /* Something went wrong.  */
	      if (idx != 0)
		__gconv_release_step (&result[0]);
	      free (result);
	      return res;
	    }
	}
      else
#endif
	/* It's a builtin transformation.  */
	__gconv_get_builtin_trans (strtab + to_module->fromname_offset,
				   &result[idx]);

      ++*nsteps;
    }

  return __GCONV_OK;
}


/* Free all resources if necessary.  */
static void __attribute__ ((unused))
free_mem (void)
{
  if (cache_malloced)
    free (cache);
#ifdef _POSIX_MAPPED_FILES
  else
    munmap (cache, cache_size);
#endif
}

text_set_element (__libc_subfreeres, free_mem);
