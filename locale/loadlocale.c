/* Functions to read locale data files.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "localeinfo.h"


static const size_t _nl_category_num_items[] =
{
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
  [category] = _NL_ITEM_INDEX (_NL_NUM_##category),
#include "categories.def"
#undef	DEFINE_CATEGORY
};


#define NO_PAREN(arg, rest...) arg, ##rest

#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
static const enum value_type _nl_value_type_##category[] = { NO_PAREN items };
#define DEFINE_ELEMENT(element, element_name, optstd, type, rest...) \
  [_NL_ITEM_INDEX (element)] = type,
#include "categories.def"
#undef DEFINE_CATEGORY

static const enum value_type *_nl_value_types[] =
{
#define DEFINE_CATEGORY(category, category_name, items, a, b, c, d) \
  [category] = _nl_value_type_##category,
#include "categories.def"
#undef DEFINE_CATEGORY
};


void
_nl_load_locale (struct loaded_l10nfile *file, int category)
{
  int fd;
  struct
    {
      unsigned int magic;
      unsigned int nstrings;
      unsigned int strindex[0];
    } *filedata;
  struct stat st;
  struct locale_data *newdata;
  int save_err;
  int swap = 0;
  int mmaped = 1;
  size_t cnt;
  inline unsigned int SWAP (const unsigned int *inw)
    {
      const unsigned char *inc = (const unsigned char *) inw;
      if (!swap)
	return *inw;
      return (inc[3] << 24) | (inc[2] << 16) | (inc[1] << 8) | inc[0];
    }

  file->decided = 1;
  file->data = NULL;

  fd = __open (file->filename, O_RDONLY);
  if (fd < 0)
    /* Cannot open the file.  */
    return;

  if (__fstat (fd, &st) < 0)
    goto puntfd;
  if (S_ISDIR (st.st_mode))
    {
      /* LOCALE/LC_foo is a directory; open LOCALE/LC_foo/SYS_LC_foo
           instead.  */
      char *newp;

      __close (fd);

      newp = (char *) alloca (strlen (file->filename)
			      + 5 + _nl_category_name_sizes[category] + 1);
      __stpcpy (__stpcpy (__stpcpy (newp, file->filename), "/SYS_"),
		_nl_category_names[category]);

      fd = __open (newp, O_RDONLY);
      if (fd < 0)
	return;

      if (__fstat (fd, &st) < 0)
	goto puntfd;
    }

  /* Map in the file's data.  */
  save_err = errno;
#ifndef MAP_COPY
  /* Linux seems to lack read-only copy-on-write.  */
#define MAP_COPY MAP_PRIVATE
#endif
#ifndef	MAP_FILE
  /* Some systems do not have this flag; it is superfluous.  */
#define	MAP_FILE 0
#endif
#ifndef MAP_INHERIT
  /* Some systems might lack this; they lose.  */
#define MAP_INHERIT 0
#endif
  filedata = (void *) __mmap ((caddr_t) 0, st.st_size, PROT_READ,
			      MAP_FILE|MAP_COPY|MAP_INHERIT, fd, 0);
  if ((void *) filedata == MAP_FAILED)
    {
      if (errno == ENOSYS)
	{
	  /* No mmap; allocate a buffer and read from the file.  */
	  mmaped = 0;
	  filedata = malloc (st.st_size);
	  if (filedata != NULL)
	    {
	      off_t to_read = st.st_size;
	      ssize_t nread;
	      char *p = (char *) filedata;
	      while (to_read > 0)
		{
		  nread = __read (fd, p, to_read);
		  if (nread <= 0)
		    {
		      free (filedata);
		      if (nread == 0)
			__set_errno (EINVAL); /* Bizarreness going on.  */
		      goto puntfd;
		    }
		  p += nread;
		  to_read -= nread;
		}
	    }
	  else
	    goto puntfd;
	  __set_errno (save_err);
	}
      else
	goto puntfd;
    }
  else if (st.st_size < sizeof (*filedata))
    /* This cannot be a locale data file since it's too small.  */
    goto puntfd;

  if (filedata->magic == LIMAGIC (category))
    /* Good data file in our byte order.  */
    swap = 0;
  else
    {
      /* Try the other byte order.  */
      swap = 1;
      if (SWAP (&filedata->magic) != LIMAGIC (category))
	/* Bad data file in either byte order.  */
	{
	puntmap:
	  __munmap ((caddr_t) filedata, st.st_size);
	puntfd:
	  __close (fd);
	  return;
	}
    }

#define W(word)	SWAP (&(word))

  if (W (filedata->nstrings) < _nl_category_num_items[category] ||
      (sizeof *filedata + W (filedata->nstrings) * sizeof (unsigned int)
       >= (size_t) st.st_size))
    {
      /* Insufficient data.  */
      __set_errno (EINVAL);
      goto puntmap;
    }

  newdata = malloc (sizeof *newdata +
		    (_nl_category_num_items[category]
		     * sizeof (union locale_data_value)));
  if (! newdata)
    goto puntmap;

  newdata->name = NULL;	/* This will be filled if necessary in findlocale.c. */
  newdata->filedata = (void *) filedata;
  newdata->filesize = st.st_size;
  newdata->mmaped = mmaped;
  newdata->usage_count = 0;
  newdata->nstrings = _nl_category_num_items[category];
  for (cnt = 0; cnt < newdata->nstrings; ++cnt)
    {
      off_t idx = W (filedata->strindex[cnt]);
      if (idx >= newdata->filesize)
	{
	  free (newdata);
	  __set_errno (EINVAL);
	  goto puntmap;
	}
      if (_nl_value_types[category][cnt] == word)
	newdata->values[cnt].word = W (*((u_int32_t *) (newdata->filedata
							+ idx)));
      else
	newdata->values[cnt].string = newdata->filedata + idx;
    }

  __close (fd);
  file->data = newdata;
}

void
_nl_unload_locale (struct locale_data *locale)
{
  if (locale->mmaped)
    __munmap ((caddr_t) locale->filedata, locale->filesize);
  else
    free ((void *) locale->filedata);

  free (locale);
}
