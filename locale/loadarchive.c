/* Code to load locale data from the locale archive file.
   Copyright (C) 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <locale.h>
#include <stddef.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "localeinfo.h"
#include "locarchive.h"

/* Define the hash function.  We define the function as static inline.  */
#define compute_hashval static inline compute_hashval
#include "hashval.h"
#undef compute_hashval


/* Name of the locale archive file.  */
static const char archfname[] = LOCALEDIR "locale-archive";


/* Record of contiguous pages already mapped from the locale archive.  */
struct archmapped
{
  void *ptr;
  uint32_t from;
  uint32_t len;
  struct archmapped *next;
};
static struct archmapped *archmapped;

/* This describes the mapping at the beginning of the file that contains
   the header data.  There could be data in the following partial page,
   so this is searched like any other.  Once the archive has been used,
   ARCHMAPPED points to this; if mapping the archive header failed,
   then headmap.ptr is null.  */
static struct archmapped headmap;
static struct stat64 archive_stat; /* stat of archive when header mapped.  */

/* Record of locales that we have already loaded from the archive.  */
struct locale_in_archive
{
  struct locale_in_archive *next;
  const char *name;
  struct locale_data *data[__LC_LAST];
};
static struct locale_in_archive *archloaded;


/* Local structure and subroutine of _nl_load_archive, see below.  */
struct range
{
  uint32_t from;
  uint32_t len;
  int category;
  void *result;
};

static int
rangecmp (const void *p1, const void *p2)
{
  return ((struct range *) p1)->from - ((struct range *) p2)->from;
}


/* Calculate the amount of space needed for all the tables described
   by the given header.  Note we do not include the empty table space
   that has been preallocated in the file, so our mapping may not be
   large enough if localedef adds data to the file in place.  However,
   doing that would permute the header fields while we are accessing
   them and thus not be safe anyway, so we don't allow for that.  */
static inline off_t
calculate_head_size (const struct locarhead *h)
{
  off_t namehash_end = (h->namehash_offset
			+ h->namehash_size * sizeof (struct namehashent));
  off_t string_end =  h->string_offset + h->string_used;
  off_t locrectab_end = (h->locrectab_offset
			 + h->locrectab_used * sizeof (struct locrecent));
  return MAX (namehash_end, MAX (string_end, locrectab_end));
}


/* Find the locale *NAMEP in the locale archive, and return the
   internalized data structure for its CATEGORY data.  If this locale has
   already been loaded from the archive, just returns the existing data
   structure.  If successful, sets *NAMEP to point directly into the mapped
   archive string table; that way, the next call can short-circuit strcmp.  */
struct locale_data *
internal_function
_nl_load_locale_from_archive (int category, const char **namep)
{
  const char *name = *namep;
  struct
  {
    void *addr;
    size_t len;
  } results[__LC_LAST];
  struct locale_in_archive *lia;
  struct locarhead *head;
  struct namehashent *namehashtab;
  struct locrecent *locrec;
  struct archmapped *mapped;
  struct archmapped *last;
  unsigned long int hval;
  size_t idx;
  size_t incr;
  struct range ranges[__LC_LAST - 1];
  int nranges;
  int cnt;
  size_t ps = __sysconf (_SC_PAGE_SIZE);
  int fd = -1;

  /* Check if we have already loaded this locale from the archive.
     If we previously loaded the locale but found bogons in the data,
     then we will have stored a null pointer to return here.  */
  for (lia = archloaded; lia != NULL; lia = lia->next)
    if (name == lia->name || !strcmp (name, lia->name))
      {
	*namep = lia->name;
	return lia->data[category];
      }

  {
    /* If the name contains a codeset, then we normalize the name before
       doing the lookup.  */
    const char *p = strchr (name, '.');
    if (p != NULL && p[1] != '@' && p[1] != '\0')
      {
	const char *rest = __strchrnul (++p, '@');
	const char *normalized_codeset = _nl_normalize_codeset (p, rest - p);
	if (normalized_codeset == NULL)	/* malloc failure */
	  return NULL;
	if (strncmp (normalized_codeset, p, rest - p) != 0
	    || normalized_codeset[rest - p] != '\0')
	  {
	    /* There is a normalized codeset name that is different from
	       what was specified; reconstruct a new locale name using it.  */
	    size_t normlen = strlen (normalized_codeset);
	    size_t restlen = strlen (rest) + 1;
	    char *newname = alloca (p - name + normlen + restlen);
	    memcpy (__mempcpy (__mempcpy (newname, name, p - name),
			       normalized_codeset, normlen),
		    rest, restlen);
	    free ((char *) normalized_codeset);
	    name = newname;
	  }
      }
  }

  /* Make sure the archive is loaded.  */
  if (archmapped == NULL)
    {
      /* We do this early as a sign that we have tried to open the archive.
	 If headmap.ptr remains null, that's an indication that we tried
	 and failed, so we won't try again.  */
      archmapped = &headmap;

      /* The archive has never been opened.  */
      fd = __open64 (archfname, O_RDONLY);
      if (fd < 0)
	/* Cannot open the archive, for whatever reason.  */
	return NULL;

      if (__fxstat64 (_STAT_VER, fd, &archive_stat) == -1)
	{
	  /* stat failed, very strange.  */
	close_and_out:
	  __close (fd);
	  return NULL;
	}

      if (sizeof (void *) > 4)
	{
	  /* We will just map the whole file, what the hell.  */
	  void *result = __mmap64 (NULL, archive_stat.st_size,
				   PROT_READ, MAP_SHARED, fd, 0);
	  if (result == MAP_FAILED)
	    goto close_and_out;
	  /* Check whether the file is large enough for the sizes given in the
	     header.  */
	  if (calculate_head_size ((const struct locarhead *) result)
	      > archive_stat.st_size)
	    {
	      (void) munmap (result, archive_stat.st_size);
	      goto close_and_out;
	    }
	  __close (fd);
	  fd = -1;

	  headmap.ptr = result;
	  /* headmap.from already initialized to zero.  */
	  headmap.len = archive_stat.st_size;
	}
      else
	{
	  struct locarhead head;
	  off_t head_size;
	  void *result;

	  if (TEMP_FAILURE_RETRY (__read (fd, &head, sizeof (head)))
	      != sizeof (head))
	    goto close_and_out;
	  head_size = calculate_head_size (&head);
	  if (head_size > archive_stat.st_size)
	    goto close_and_out;
	  result = __mmap64 (NULL, head_size, PROT_READ, MAP_SHARED, fd, 0);
	  if (result == MAP_FAILED)
	    goto close_and_out;

	  /* Record that we have mapped the initial pages of the file.  */
	  headmap.ptr = result;
	  headmap.len = MIN ((head_size + ps - 1) & ~(ps - 1),
			     archive_stat.st_size);
	}
    }

  /* If there is no archive or it cannot be loaded for some reason fail.  */
  if (__builtin_expect (headmap.ptr == NULL, 0))
    return NULL;

  /* We have the archive available.  To find the name we first have to
     determine its hash value.  */
  hval = compute_hashval (name, strlen (name));

  head = headmap.ptr;
  namehashtab = (struct namehashent *) ((char *) head
					+ head->namehash_offset);

  idx = hval % head->namehash_size;
  incr = 1 + hval % (head->namehash_size - 2);

  /* If the name_offset field is zero this means this is a
     deleted entry and therefore no entry can be found.  */
  while (1)
    {
      if (namehashtab[idx].name_offset == 0)
	/* Not found.  */
	return NULL;

      if (namehashtab[idx].hashval == hval
	  && strcmp (name, headmap.ptr + namehashtab[idx].name_offset) == 0)
	/* Found the entry.  */
	break;

      idx += incr;
      if (idx >= head->namehash_size)
	idx -= head->namehash_size;
    }

  /* We found an entry.  It might be a placeholder for a removed one.  */
  if (namehashtab[idx].locrec_offset == 0)
    return NULL;

  locrec = (struct locrecent *) (headmap.ptr + namehashtab[idx].locrec_offset);

  if (sizeof (void *) > 4 /* || headmap.len == archive_stat.st_size */)
    {
      /* We already have the whole locale archive mapped in.  */
      assert (headmap.len == archive_stat.st_size);
      for (cnt = 0; cnt < __LC_LAST; ++cnt)
	if (cnt != LC_ALL)
	  {
	    if (locrec->record[cnt].offset + locrec->record[cnt].len
		> headmap.len)
	      /* The archive locrectab contains bogus offsets.  */
	      return NULL;
	    results[cnt].addr = headmap.ptr + locrec->record[cnt].offset;
	    results[cnt].len = locrec->record[cnt].len;
	  }
    }
  else
    {
      /* Get the offsets of the data files and sort them.  */
      for (cnt = nranges = 0; cnt < __LC_LAST; ++cnt)
	if (cnt != LC_ALL)
	  {
	    ranges[nranges].from = locrec->record[cnt].offset;
	    ranges[nranges].len = locrec->record[cnt].len;
	    ranges[nranges].category = cnt;
	    ranges[nranges].result = NULL;

	    ++nranges;
	  }

      qsort (ranges, nranges, sizeof (ranges[0]), rangecmp);

      /* The information about mmap'd blocks is kept in a list.
	 Skip over the blocks which are before the data we need.  */
      last = mapped = archmapped;
      for (cnt = 0; cnt < nranges; ++cnt)
	{
	  int upper;
	  size_t from;
	  size_t to;
	  void *addr;
	  struct archmapped *newp;

	  /* Determine whether the appropriate page is already mapped.  */
	  while (mapped != NULL
		 && mapped->from + mapped->len <= ranges[cnt].from)
	    {
	      last = mapped;
	      mapped = mapped->next;
	    }

	  /* Do we have a match?  */
	  if (mapped != NULL
	      && mapped->from <= ranges[cnt].from
	      && ((char *) ranges[cnt].from + ranges[cnt].len
		  <= (char *) mapped->from + mapped->len))
	    {
	      /* Yep, already loaded.  */
	      results[ranges[cnt].category].addr = ((char *) mapped->ptr
						    + ranges[cnt].from
						    - mapped->from);
	      results[ranges[cnt].category].len = ranges[cnt].len;
	      continue;
	    }

	  /* Map the range with the locale data from the file.  We will
	     try to cover as much of the locale as possible.  I.e., if the
	     next category (next as in "next offset") is on the current or
	     immediately following page we use it as well.  */
	  assert (powerof2 (ps));
	  from = ranges[cnt].from & ~(ps - 1);
	  upper = cnt;
	  do
	    {
	      to = ((ranges[upper].from + ranges[upper].len + ps - 1)
		    & ~(ps - 1));
	      ++upper;
	    }
	  /* Loop while still in contiguous pages. */
	  while (upper < nranges && ranges[upper].from < to + ps);

	  if (to > archive_stat.st_size)
	    /* The archive locrectab contains bogus offsets.  */
	    return NULL;

	  /* Open the file if it hasn't happened yet.  */
	  if (fd == -1)
	    {
	      struct stat64 st;
	      fd = __open64 (archfname, O_RDONLY);
	      if (fd == -1)
		/* Cannot open the archive, for whatever reason.  */
		return NULL;
	      /* Now verify we think this is really the same archive file
		 we opened before.  If it has been changed we cannot trust
		 the header we read previously.  */
	      if (__fxstat64 (_STAT_VER, fd, &st) < 0
		  || st.st_size != archive_stat.st_size
		  || st.st_mtime != archive_stat.st_mtime
		  || st.st_dev != archive_stat.st_dev
		  || st.st_ino != archive_stat.st_ino)
		return NULL;
	    }

	  /* Map the range from the archive.  */
	  addr = __mmap64 (NULL, to - from, PROT_READ, MAP_SHARED, fd, from);
	  if (addr == MAP_FAILED)
	    return NULL;

	  /* Allocate a record for this mapping.  */
	  newp = (struct archmapped *) malloc (sizeof (struct archmapped));
	  if (newp == NULL)
	    {
	      (void) munmap (addr, to - from);
	      return NULL;
	    }

	  /* And queue it.  */
	  newp->ptr = addr;
	  newp->from = from;
	  newp->len = to - from;
	  assert (last->next == mapped);
	  newp->next = mapped;
	  last->next = newp;
	  last = newp;

	  /* Determine the load addresses for the category data.  */
	  do
	    {
	      assert (ranges[cnt].from >= from);
	      results[ranges[cnt].category].addr = ((char *) addr
						    + ranges[cnt].from - from);
	      results[ranges[cnt].category].len = ranges[cnt].len;
	    }
	  while (++cnt < upper);
	  --cnt;		/* The 'for' will increase 'cnt' again.  */
	}
    }

  /* We succeeded in mapping all the necessary regions of the archive.
     Now we need the expected data structures to point into the data.  */

  lia = malloc (sizeof *lia);
  if (__builtin_expect (lia == NULL, 0))
    return NULL;

  lia->name = headmap.ptr + namehashtab[idx].name_offset;
  lia->next = archloaded;
  archloaded = lia;

  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL)
      {
	lia->data[cnt] = _nl_intern_locale_data (cnt,
						 results[cnt].addr,
						 results[cnt].len);
	if (__builtin_expect (lia->data[cnt] != NULL, 1))
	  {
	    /* _nl_intern_locale_data leaves us these fields to initialize.  */
	    lia->data[cnt]->alloc = ld_archive;
	    lia->data[cnt]->name = lia->name;
	  }
      }

  *namep = lia->name;
  return lia->data[category];
}

void
_nl_archive_subfreeres (void)
{
  struct locale_in_archive *lia;
  struct archmapped *am;

  /* Toss out our cached locales.  */
  lia = archloaded;
  while (lia != NULL)
    {
      int category;
      struct locale_in_archive *dead = lia;
      lia = lia->next;

      for (category = 0; category < __LC_LAST; ++category)
	if (category != LC_ALL)
	  /* _nl_unload_locale just does this free for the archive case.  */
	  free (dead->data[category]);
      free (dead);
    }
  archloaded = NULL;

  if (archmapped != NULL)
    {
      /* Now toss all the mapping windows, which we know nothing is using any
	 more because we just tossed all the locales that point into them.  */

      assert (archmapped == &headmap);
      archmapped = NULL;
      (void) munmap (headmap.ptr, headmap.len);
      am = headmap.next;
      while (am != NULL)
	{
	  struct archmapped *dead = am;
	  am = am->next;
	  (void) munmap (dead->ptr, dead->len);
	  free (dead);
	}
    }
}
