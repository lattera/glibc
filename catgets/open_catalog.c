/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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

#include <byteswap.h>
#include <endian.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "catgetsinfo.h"


#define SWAPU32(w) bswap_32 (w)


void
__open_catalog (__nl_catd catalog)
{
  int fd = -1;
  struct stat st;
  int swapping;

  /* Make sure we are alone.  */
  __libc_lock_lock (catalog->lock);

  /* Check whether there was no other thread faster.  */
  if (catalog->status != closed)
    /* While we waited some other thread tried to open the catalog.  */
    goto unlock_return;

  if (strchr (catalog->cat_name, '/') != NULL || catalog->nlspath == NULL)
    fd = __open (catalog->cat_name, O_RDONLY);
  else
    {
      const char *run_nlspath = catalog->nlspath;
#define ENOUGH(n)							      \
  if (bufact + (n) >=bufmax) 						      \
    {									      \
      char *old_buf = buf;						      \
      bufmax += 256 + (n);						      \
      buf = (char *) alloca (bufmax);					      \
      memcpy (buf, old_buf, bufact);					      \
    }

      /* The RUN_NLSPATH variable contains a colon separated list of
	 descriptions where we expect to find catalogs.  We have to
	 recognize certain % substitutions and stop when we found the
	 first existing file.  */
      char *buf;
      size_t bufact;
      size_t bufmax;
      size_t len;

      buf = NULL;
      bufmax = 0;

      fd = -1;
      while (*run_nlspath != '\0')
	{
	  bufact = 0;
	  while (*run_nlspath != ':' && *run_nlspath != '\0')
	    if (*run_nlspath == '%')
	      {
		const char *tmp;

		++run_nlspath;	/* We have seen the `%'.  */
		switch (*run_nlspath++)
		  {
		  case 'N':
		    /* Use the catalog name.  */
		    len = strlen (catalog->cat_name);
		    ENOUGH (len);
		    memcpy (&buf[bufact], catalog->cat_name, len);
		    bufact += len;
		    break;
		  case 'L':
		    /* Use the current locale category value.  */
		    len = strlen (catalog->env_var);
		    ENOUGH (len);
		    memcpy (&buf[bufact], catalog->env_var, len);
		    bufact += len;
		    break;
		  case 'l':
		    /* Use language element of locale category value.  */
		    tmp = catalog->env_var;
		    do
		      {
			ENOUGH (1);
			buf[bufact++] = *tmp++;
		      }
		    while (*tmp != '\0' && *tmp != '_' && *tmp != '.');
		    break;
		  case 't':
		    /* Use territory element of locale category value.  */
		    tmp = catalog->env_var;
		    do
		      ++tmp;
		    while (*tmp != '\0' && *tmp != '_' && *tmp != '.');
		    if (*tmp == '_')
		      {
			++tmp;
			do
			  {
			    ENOUGH (1);
			    buf[bufact++] = *tmp;
			  }
			while (*tmp != '\0' && *tmp != '.');
		      }
		    break;
		  case 'c':
		    /* Use code set element of locale category value.  */
		    tmp = catalog->env_var;
		    do
		      ++tmp;
		    while (*tmp != '\0' && *tmp != '.');
		    if (*tmp == '.')
		      {
			++tmp;
			do
			  {
			    ENOUGH (1);
			    buf[bufact++] = *tmp;
			  }
			while (*tmp != '\0');
		      }
		    break;
		  case '%':
		    ENOUGH (1);
		    buf[bufact++] = '%';
		    break;
		  default:
		    /* Unknown variable: ignore this path element.  */
		    bufact = 0;
		    while (*run_nlspath != '\0' && *run_nlspath != ':')
		      ++run_nlspath;
		    break;
		  }
	      }
	    else
	      {
		ENOUGH (1);
		buf[bufact++] = *run_nlspath++;
	      }
	  ENOUGH (1);
	  buf[bufact] = '\0';

	  if (bufact != 0)
	    {
	      fd = __open (buf, O_RDONLY);
	      if (fd >= 0)
		break;
	    }

	  ++run_nlspath;
	}
    }

  /* Avoid dealing with directories and block devices */
  if (fd < 0 || __fstat (fd, &st) < 0 || !S_ISREG (st.st_mode))
    {
      catalog->status = nonexisting;
      goto unlock_return;
    }

#ifndef MAP_COPY
    /* Linux seems to lack read-only copy-on-write.  */
# define MAP_COPY MAP_PRIVATE
#endif
#ifndef MAP_FILE
    /* Some systems do not have this flag; it is superfluous.  */
# define MAP_FILE 0
#endif
#ifndef MAP_INHERIT
    /* Some systems might lack this; they lose.  */
# define MAP_INHERIT 0
#endif
  catalog->file_size = st.st_size;
  catalog->file_ptr =
    (struct catalog_obj *) __mmap (NULL, st.st_size, PROT_READ,
				   MAP_FILE|MAP_COPY|MAP_INHERIT, fd, 0);
  if (catalog->file_ptr != (struct catalog_obj *) MAP_FAILED)
    /* Tell the world we managed to mmap the file.  */
    catalog->status = mmapped;
  else
    {
      /* mmap failed perhaps because the system call is not
	 implemented.  Try to load the file.  */
      size_t todo;
      catalog->file_ptr = malloc (st.st_size);
      if (catalog->file_ptr == NULL)
	{
	  catalog->status = nonexisting;
	  goto unlock_return;
	}
      todo = st.st_size;
      /* Save read, handle partial reads.  */
      do
	{
	  size_t now = __read (fd, (((char *) &catalog->file_ptr)
				    + (st.st_size - todo)), todo);
	  if (now == 0)
	    {
	      free ((void *) catalog->file_ptr);
	      catalog->status = nonexisting;
	      goto unlock_return;
	    }
	  todo -= now;
	}
      while (todo > 0);
      catalog->status = malloced;
    }

  /* We don't need the file anymore.  */
  __close (fd);
  fd = -1;

  /* Determine whether the file is a catalog file and if yes whether
     it is written using the correct byte order.  Else we have to swap
     the values.  */
  if (catalog->file_ptr->magic == CATGETS_MAGIC)
    swapping = 0;
  else if (catalog->file_ptr->magic == SWAPU32 (CATGETS_MAGIC))
    swapping = 1;
  else
    {
      /* Invalid file.  Free the resources and mark catalog as not
	 usable.  */
      if (catalog->status == mmapped)
	__munmap ((void *) catalog->file_ptr, catalog->file_size);
      else
	free (catalog->file_ptr);
      catalog->status = nonexisting;
      goto unlock_return;
    }

#define SWAP(x) (swapping ? SWAPU32 (x) : (x))

  /* Get dimensions of the used hashing table.  */
  catalog->plane_size = SWAP (catalog->file_ptr->plane_size);
  catalog->plane_depth = SWAP (catalog->file_ptr->plane_depth);

  /* The file contains two versions of the pointer tables.  Pick the
     right one for the local byte order.  */
#if __BYTE_ORDER == __LITTLE_ENDIAN
  catalog->name_ptr = &catalog->file_ptr->name_ptr[0];
#elif __BYTE_ORDER == __BIG_ENDIAN
  catalog->name_ptr = &catalog->file_ptr->name_ptr[catalog->plane_size
						  * catalog->plane_depth
						  * 3];
#else
# error Cannot handle __BYTE_ORDER byte order
#endif

  /* The rest of the file contains all the strings.  They are
     addressed relative to the position of the first string.  */
  catalog->strings =
    (const char *) &catalog->file_ptr->name_ptr[catalog->plane_size
					       * catalog->plane_depth * 3 * 2];

  /* Release the lock again.  */
 unlock_return:
  if (fd != -1)
    __close (fd);
  __libc_lock_unlock (catalog->lock);
}
