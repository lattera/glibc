/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libintl.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "../../crypt/md5.h"
#include "../localeinfo.h"
#include "../locarchive.h"
#include "simple-hash.h"
#include "localedef.h"

extern const char *output_prefix;

#define ARCHIVE_NAME LOCALEDIR "/locale-archive"

static const char *locnames[] =
  {
#define DEFINE_CATEGORY(category, category_name, items, a) \
  [category] = category_name,
#include "categories.def"
#undef  DEFINE_CATEGORY
  };


/* Size of the initial archive header.  */
#define INITIAL_NUM_NANES	450
#define INITIAL_SIZE_STRINGS	3500
#define INITIAL_NUM_LOCREC	350
#define INITIAL_NUM_SUMS	2000


static void
create_archive (const char *archivefname, struct locarhandle *ah)
{
  int fd;
  char fname[strlen (archivefname) + sizeof (".XXXXXX")];
  struct locarhead head;
  void *p;
  size_t total;

  strcpy (stpcpy (fname, archivefname), ".XXXXXX");

  /* Create a temporary file in the correct directory.  */
  fd = mkstemp (fname);
  if (fd == -1)
    error (EXIT_FAILURE, errno, _("cannot create temporary file"));

  /* Create the initial content of the archive.  */
  head.magic = AR_MAGIC;
  head.namehash_offset = sizeof (struct locarhead);
  head.namehash_used = 0;
  head.namehash_size = next_prime (INITIAL_NUM_NANES);

  head.string_offset = (head.namehash_offset
			+ head.namehash_size * sizeof (struct namehashent));
  head.string_used = 0;
  head.string_size = INITIAL_SIZE_STRINGS;

  head.locrectab_offset = head.string_offset + head.string_size;
  head.locrectab_used = 0;
  head.locrectab_size = INITIAL_NUM_LOCREC;

  head.sumhash_offset = (head.locrectab_offset
			 + head.locrectab_size * sizeof (struct locrecent));
  head.sumhash_used = 0;
  head.sumhash_size = next_prime (INITIAL_NUM_SUMS);

  total = head.sumhash_offset + head.sumhash_size * sizeof (struct sumhashent);

  /* Write out the header and create room for the other data structures.  */
  if (TEMP_FAILURE_RETRY (write (fd, &head, sizeof (head))) != sizeof (head))
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot initialize archive file"));
    }

  if (ftruncate64 (fd, total) != 0)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot resize archive file"));
    }

  /* Map the header and all the administration data structures.  */
  p = mmap64 (NULL, total, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot map archive header"));
    }

  /* Now try to rename it.  We don't use the rename function since
     this would overwrite a file which has been created in
     parallel.  */
  if (link (fname, archivefname) == -1)
    {
      int errval = errno;

      /* We cannot use the just created file.  */
      close (fd);
      unlink (fname);

      if (errval == EEXIST)
	{
	  /* There is already an archive.  Must have been a localedef run
	     which happened in parallel.  Simply open this file then.  */
	  open_archive (ah);
	  return;
	}

      error (EXIT_FAILURE, errval, _("failed to create new locale archive"));
    }

  /* Remove the temporary name.  */
  unlink (fname);

  /* Make the file globally readable.  */
  if (fchmod (fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) == -1)
    {
      int errval = errno;
      unlink (archivefname);
      error (EXIT_FAILURE, errval,
	     _("cannot change mode of new locale archive"));
    }

  ah->fd = fd;
  ah->addr = p;
  ah->len = total;
}


static void
enlarge_archive (struct locarhandle *ah, const struct locarhead *head)
{
  struct stat64 st;
  int fd;
  struct locarhead newhead;
  size_t total;
  void *p;
  unsigned int cnt;
  struct namehashent *oldnamehashtab;
  struct locrecent *oldlocrectab;
  struct locarhandle new_ah;
  size_t prefix_len = output_prefix ? strlen (output_prefix) : 0;
  char archivefname[prefix_len + sizeof (ARCHIVE_NAME)];
  char fname[prefix_len + sizeof (ARCHIVE_NAME) + sizeof (".XXXXXX") - 1];

  if (output_prefix)
    memcpy (archivefname, output_prefix, prefix_len);
  strcpy (archivefname + prefix_len, ARCHIVE_NAME);
  strcpy (stpcpy (fname, archivefname), ".XXXXXX");

  /* Not all of the old file has to be mapped.  Change this now this
     we will have to access the whole content.  */
  if (fstat64 (ah->fd, &st) != 0
      || (ah->addr = mmap64 (NULL, st.st_size, PROT_READ | PROT_WRITE,
			     MAP_SHARED, ah->fd, 0)) == MAP_FAILED)
    error (EXIT_FAILURE, errno, _("cannot map locale archive file"));
  ah->len = st.st_size;

  /* Create a temporary file in the correct directory.  */
  fd = mkstemp (fname);
  if (fd == -1)
    error (EXIT_FAILURE, errno, _("cannot create temporary file"));

  /* Copy the existing head information.  */
  newhead = *head;

  /* Create the new archive header.  The sizes of the various tables
     should be double from what is currently used.  */
  newhead.namehash_size = MAX (next_prime (2 * newhead.namehash_used),
			       newhead.namehash_size);
  printf ("name: size: %u, used: %d, new: size: %u\n",
	  head->namehash_size, head->namehash_used, newhead.namehash_size);

  newhead.string_offset = (newhead.namehash_offset
			   + (newhead.namehash_size
			      * sizeof (struct namehashent)));
  newhead.string_size = MAX (2 * newhead.string_used, newhead.string_size);

  newhead.locrectab_offset = newhead.string_offset + newhead.string_size;
  newhead.locrectab_size = MAX (2 * newhead.locrectab_used,
				newhead.locrectab_size);

  newhead.sumhash_offset = (newhead.locrectab_offset
			    + (newhead.locrectab_size
			       * sizeof (struct locrecent)));
  newhead.sumhash_size = MAX (next_prime (2 * newhead.sumhash_used),
			      newhead.sumhash_size);

  total = (newhead.sumhash_offset
	   + newhead.sumhash_size * sizeof (struct sumhashent));

  /* The new file is empty now.  */
  newhead.namehash_used = 0;
  newhead.string_used = 0;
  newhead.locrectab_used = 0;
  newhead.sumhash_used = 0;

  /* Write out the header and create room for the other data structures.  */
  if (TEMP_FAILURE_RETRY (write (fd, &newhead, sizeof (newhead)))
      != sizeof (newhead))
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot initialize archive file"));
    }

  if (ftruncate64 (fd, total) != 0)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot resize archive file"));
    }

  /* Map the header and all the administration data structures.  */
  p = mmap64 (NULL, total, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot map archive header"));
    }

  /* Lock the new file.  */
  if (lockf64 (fd, F_LOCK, total) != 0)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot lock new archive"));
    }

  new_ah.len = total;
  new_ah.addr = p;
  new_ah.fd = fd;

  /* Walk through the hash name hash table to find out what data is
     still referenced and transfer it into the new file.  */
  oldnamehashtab = (struct namehashent *) ((char *) ah->addr
					   + head->namehash_offset);
  oldlocrectab = (struct locrecent *) ((char *) ah->addr
				       + head->locrectab_offset);
  for (cnt = 0; cnt < head->namehash_size; ++cnt)
    if (oldnamehashtab[cnt].locrec_offset != 0)
      {
	/* Insert this entry in the new hash table.  */
	locale_data_t old_data;
	unsigned int idx;
	struct locrecent *oldlocrec;

	oldlocrec = (struct locrecent *) ((char *) ah->addr
					  + oldnamehashtab[cnt].locrec_offset);

	for (idx = 0; idx < __LC_LAST; ++idx)
	  if (idx != LC_ALL)
	    {
	      old_data[idx].size = oldlocrec->record[idx].len;
	      old_data[idx].addr
		= ((char *) ah->addr + oldlocrec->record[idx].offset);

	      __md5_buffer (old_data[idx].addr, old_data[idx].size,
			    old_data[idx].sum);
	    }

	if (add_locale_to_archive (&new_ah,
				   ((char *) ah->addr
				    + oldnamehashtab[cnt].name_offset),
				   old_data, 0) != 0)
	  error (EXIT_FAILURE, 0, _("cannot extend locale archive file"));
      }


  /* Make the file globally readable.  */
  if (fchmod (fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) == -1)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval,
	     _("cannot change mode of resized locale archive"));
    }

  /* Rename the new file.  */
  if (rename (fname, archivefname) != 0)
    {
      int errval = errno;
      unlink (fname);
      error (EXIT_FAILURE, errval, _("cannot rename new archive"));
    }

  /* Close the old file.  */
  close_archive (ah);

  /* Add the information for the new one.  */
  *ah = new_ah;
}


void
open_archive (struct locarhandle *ah)
{
  struct stat64 st;
  struct stat64 st2;
  int fd;
  struct locarhead head;
  int retry = 0;
  size_t prefix_len = output_prefix ? strlen (output_prefix) : 0;
  char archivefname[prefix_len + sizeof (ARCHIVE_NAME)];

  if (output_prefix)
    memcpy (archivefname, output_prefix, prefix_len);
  strcpy (archivefname + prefix_len, ARCHIVE_NAME);

 again:
  /* Open the archive.  We must have exclusive write access.  */
  fd = open64 (archivefname, O_RDWR);
  if (fd == -1)
    {
      /* Maybe the file does not yet exist.  */
      if (errno == ENOENT)
	{
	  create_archive (archivefname, ah);
	  return;
	}
      else
	error (EXIT_FAILURE, errno, _("cannot open locale archive \"%s\""),
	       archivefname);
    }

  if (fstat64 (fd, &st) < 0)
    error (EXIT_FAILURE, errno, _("cannot stat locale archive \"%s\""),
	   archivefname);

  if (lockf64 (fd, F_LOCK, st.st_size) == -1)
    {
      close (fd);

      if (retry++ < max_locarchive_open_retry)
	{
	  struct timespec req;

	  /* Wait for a bit.  */
	  req.tv_sec = 0;
	  req.tv_nsec = 1000000 * (random () % 500 + 1);
	  (void) nanosleep (&req, NULL);

	  goto again;
	}

      error (EXIT_FAILURE, errno, _("cannot lock locale archive \"%s\""),
	     archivefname);
    }

  /* One more check.  Maybe another process replaced the archive file
     with a new, larger one since we opened the file.  */
  if (stat64 (archivefname, &st2) == -1
      || st.st_dev != st2.st_dev
      || st.st_ino != st2.st_ino)
    {
      close (fd);
      goto again;
    }

  /* Read the header.  */
  if (TEMP_FAILURE_RETRY (read (fd, &head, sizeof (head))) != sizeof (head))
    error (EXIT_FAILURE, errno, _("cannot read archive header"));

  ah->fd = fd;
  ah->len = (head.sumhash_offset
	     + head.sumhash_size * sizeof (struct sumhashent));

  /* Now we know how large the administrative information part is.
     Map all of it.  */
  ah->addr = mmap64 (NULL, ah->len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ah->addr == MAP_FAILED)
    error (EXIT_FAILURE, errno, _("cannot map archive header"));
}


void
close_archive (struct locarhandle *ah)
{
  munmap (ah->addr, ah->len);
  close (ah->fd);
}


/* Check the content of the archive for duplicates.  Add the content
   of the files if necessary.  Add all the names, possibly overwriting
   old files.  */
int
add_locale_to_archive (ah, name, data, replace)
     struct locarhandle *ah;
     const char *name;
     locale_data_t data;
     bool replace;
{
  /* First look for the name.  If it already exists and we are not
     supposed to replace it don't do anything.  If it does not exist
     we have to allocate a new locale record.  */
  size_t name_len = strlen (name);
  uint32_t file_offsets[__LC_LAST];
  unsigned int num_new_offsets = 0;
  struct sumhashent *sumhashtab;
  uint32_t hval;
  unsigned int cnt;
  unsigned int idx;
  unsigned int insert_idx;
  struct locarhead *head;
  struct namehashent *namehashtab;
  struct namehashent *namehashent;
  unsigned int incr;
  struct locrecent *locrecent;

  head = ah->addr;
  sumhashtab = (struct sumhashent *) ((char *) ah->addr
				      + head->sumhash_offset);
  namehashtab = (struct namehashent *) ((char *) ah->addr
					+ head->namehash_offset);


  /* For each locale category data set determine whether the same data
     is already somewhere in the archive.  */
  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL)
      {
	/* By default signal that we have no data.  */
	file_offsets[cnt] = 0;
	++num_new_offsets;

	/* Compute the hash value of the checksum to determine a
	   starting point for the search in the MD5 hash value
	   table.  */
	hval = compute_hashval (data[cnt].sum, 16);

	idx = hval % head->sumhash_size;
	incr = 1 + hval % (head->sumhash_size - 2);

	while (sumhashtab[idx].file_offset != 0)
	  {
	    if (memcmp (data[cnt].sum, sumhashtab[idx].sum, 16) == 0)
	      {
		/* Found it.  */
		file_offsets[cnt] = sumhashtab[idx].file_offset;
		--num_new_offsets;
		break;
	      }

	    idx += incr;
	    if (idx >= head->sumhash_size)
	      idx -= head->sumhash_size;
	  }
      }


  /* Hash value of the locale name.  */
  hval = compute_hashval (name, name_len);

  insert_idx = -1;
  idx = hval % head->namehash_size;
  incr = 1 + hval % (head->namehash_size - 2);

  /* If the name_offset field is zero this means this is no
     deleted entry and therefore no entry can be found.  */
  while (namehashtab[idx].name_offset != 0)
    {
      if (namehashtab[idx].hashval == hval
	  && strcmp (name,
		     (char *) ah->addr + namehashtab[idx].name_offset) == 0)
	{
	  /* Found the entry.  */
	  if (! replace)
	    {
	      if (! be_quiet)
		error (0, 0, _("locale '%s' already exists"), name);
	      return 1;
	    }

	  break;
	}

      /* Remember the first place we can insert the new entry.  */
      if (namehashtab[idx].locrec_offset == 0 && insert_idx == -1)
	insert_idx = idx;

      idx += incr;
      if (idx >= head->namehash_size)
	idx -= head->namehash_size;
    }

  /* Add as early as possible.  */
  if (insert_idx != -1)
    idx = insert_idx;

  namehashent = &namehashtab[idx];

  /* Determine whether we have to resize the file.  */
  if (100 * (head->sumhash_used + num_new_offsets) > 75 * head->sumhash_size
      || (namehashent->locrec_offset == 0
	  && (head->locrectab_used == head->locrectab_size
	      || head->string_used + name_len + 1 > head->string_size
	      || 100 * head->namehash_used > 75 * head->namehash_size)))
    {
      /* The current archive is not large enough.  */
      enlarge_archive (ah, head);
      return add_locale_to_archive (ah, name, data, replace);
    }

  /* Add the locale data which is not yet in the archive.  */
  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL && file_offsets[cnt] == 0)
      {
	/* The data for this section is not yet available in the
	   archive.  Append it.  */
	off64_t lastpos;
	uint32_t md5hval;

	lastpos = lseek64 (ah->fd, 0, SEEK_END);
	if (lastpos == (off64_t) -1)
	  error (EXIT_FAILURE, errno, _("cannot add to locale archive"));

	/* Align all data to a 16 byte boundary.  */
	if ((lastpos & 15) != 0)
	  {
	    static const char zeros[15] = { 0, };

	    if (TEMP_FAILURE_RETRY (write (ah->fd, zeros, 16 - (lastpos & 15)))
		!= 16 - (lastpos & 15))
	      error (EXIT_FAILURE, errno, _("cannot add to locale archive"));

	    lastpos += 16 - (lastpos & 15);
	  }

	/* Remember the position.  */
	file_offsets[cnt] = lastpos;

	/* Write the data.  */
	if (TEMP_FAILURE_RETRY (write (ah->fd, data[cnt].addr, data[cnt].size))
	    != data[cnt].size)
	  error (EXIT_FAILURE, errno, _("cannot add to locale archive"));

	/* Add the hash value to the hash table.  */
	md5hval = compute_hashval (data[cnt].sum, 16);

	idx = md5hval % head->sumhash_size;
	incr = 1 + md5hval % (head->sumhash_size - 2);

	while (sumhashtab[idx].file_offset != 0)
	  {
	    idx += incr;
	    if (idx >= head->sumhash_size)
	      idx -= head->sumhash_size;
	  }

	memcpy (sumhashtab[idx].sum, data[cnt].sum, 16);
	sumhashtab[idx].file_offset = file_offsets[cnt];

	++head->sumhash_used;
      }


  if (namehashent->locrec_offset == 0)
    {
      /* Add the name string.  */
      memcpy ((char *) ah->addr + head->string_offset + head->string_used,
	      name, name_len + 1);
      namehashent->name_offset = head->string_offset + head->string_used;
      head->string_used += name_len + 1;

      /* Allocate a name location record.  */
      namehashent->locrec_offset = (head->locrectab_offset
				    + (head->locrectab_used++
				       * sizeof (struct locrecent)));

      namehashent->hashval = hval;

      ++head->namehash_used;
    }


  /* Fill in the table with the locations of the locale data.  */
  locrecent = (struct locrecent *) ((char *) ah->addr
				    + namehashent->locrec_offset);
  for (cnt = 0; cnt < __LC_LAST; ++cnt)
    if (cnt != LC_ALL)
      {
	locrecent->record[cnt].offset = file_offsets[cnt];
	locrecent->record[cnt].len = data[cnt].size;
      }


  /* Read the locale.alias file to see whether any matching record is
     found.  If an entry is available check whether it is already in
     the archive.  If this is the case check whether the new locale's
     name is more specific than the one currently referred to by the
     alias.  */


  return 0;
}


int
add_locales_to_archive (nlist, list, replace)
     size_t nlist;
     char *list[];
     bool replace;
{
  struct locarhandle ah;
  int result = 0;

  /* Open the archive.  This call never returns if we cannot
     successfully open the archive.  */
  open_archive (&ah);

  while (nlist-- > 0)
    {
      const char *fname = *list++;
      size_t fnamelen = strlen (fname);
      struct stat64 st;
      DIR *dirp;
      struct dirent64 *d;
      int seen;
      locale_data_t data;
      int cnt;

      if (! be_quiet)
	printf (_("Adding %s\n"), fname);

      /* First see whether this really is a directory and whether it
	 contains all the require locale category files.  */
      if (stat64 (fname, &st) < 0)
	{
	  error (0, 0, _("stat of \"%s\" failed: %s: ignored"), fname,
		 strerror (errno));
	  continue;
	}
      if (!S_ISDIR (st.st_mode))
	{
	  error (0, 0, _("\"%s\" is no directory; ignored"), fname);
	  continue;
	}

      dirp = opendir (fname);
      if (dirp == NULL)
	{
	  error (0, 0, _("cannot open directory \"%s\": %s: ignored"),
		 fname, strerror (errno));
	  continue;
	}

      seen = 0;
      while ((d = readdir64 (dirp)) != NULL)
	{
	  for (cnt = 0; cnt < __LC_LAST; ++cnt)
	    if (cnt != LC_ALL)
	      if (strcmp (d->d_name, locnames[cnt]) == 0)
		{
		  unsigned char d_type;

		  /* We have an object of the required name.  If it's
		     a directory we have to look at a file with the
		     prefix "SYS_".  Otherwise we have found what we
		     are looking for.  */
#ifdef _DIRENT_HAVE_D_TYPE
		  d_type = d->d_type;

		  if (d_type != DT_REG)
#endif
		    {
		      char fullname[fnamelen + 2 * strlen (d->d_name) + 7];

#ifdef _DIRENT_HAVE_D_TYPE
		      if (d_type == DT_UNKNOWN)
#endif
			{
			  strcpy (stpcpy (stpcpy (fullname, fname), "/"),
				  d->d_name);

			  if (stat64 (fullname, &st) == -1)
			    /* We cannot stat the file, ignore it.  */
			    break;

			  d_type = IFTODT (st.st_mode);
			}

		      if (d_type == DT_DIR)
			{
			  /* We have to do more tests.  The file is a
			     directory and it therefore must contain a
			     regular file with the same name except a
			     "SYS_" prefix.  */
			  char *t = stpcpy (stpcpy (fullname, fname), "/");
			  strcpy (stpcpy (stpcpy (t, d->d_name), "/SYS_"),
				  d->d_name);

			  if (stat64 (fullname, &st) == -1)
			    /* There is no SYS_* file or we cannot
			       access it.  */
			    break;

			  d_type = IFTODT (st.st_mode);
			}
		    }

		  /* If we found a regular file (eventually after
		     following a symlink) we are successful.  */
		  if (d_type == DT_REG)
		    ++seen;
		  break;
		}
	}

      closedir (dirp);

      if (seen != __LC_LAST - 1)
	{
	  /* We don't have all locale category files.  Ignore the name.  */
	  error (0, 0, _("incomplete set of locale files in \"%s\""),
		 fname);
	  continue;
	}

      /* Add the files to the archive.  To do this we first compute
	 sizes and the MD5 sums of all the files.  */
      for (cnt = 0; cnt < __LC_LAST; ++cnt)
	if (cnt != LC_ALL)
	  {
	    char fullname[fnamelen + 2 * strlen (locnames[cnt]) + 7];
	    int fd;

	    strcpy (stpcpy (stpcpy (fullname, fname), "/"), locnames[cnt]);
	    fd = open64 (fullname, O_RDONLY);
	    if (fd == -1 || fstat64 (fd, &st) == -1)
	      {
		/* Cannot read the file.  */
		if (fd != -1)
		  close (fd);
		break;
	      }

	    if (S_ISDIR (st.st_mode))
	      {
		char *t;
		close (fd);
		t = stpcpy (stpcpy (fullname, fname), "/");
		strcpy (stpcpy (stpcpy (t, locnames[cnt]), "/SYS_"),
			locnames[cnt]);

		fd = open64 (fullname, O_RDONLY);
		if (fd == -1 || fstat64 (fd, &st) == -1
		    || !S_ISREG (st.st_mode))
		  {
		    if (fd != -1)
		      close (fd);
		    break;
		  }
	      }

	    /* Map the file.  */
	    data[cnt].addr = mmap64 (NULL, st.st_size, PROT_READ, MAP_SHARED,
				     fd, 0);
	    if (data[cnt].addr == MAP_FAILED)
	      {
		/* Cannot map it.  */
		close (fd);
		break;
	      }

	    data[cnt].size = st.st_size;
	    __md5_buffer (data[cnt].addr, st.st_size, data[cnt].sum);

	    /* We don't need the file descriptor anymore.  */
	    close (fd);
	  }

      if (cnt != __LC_LAST)
	{
	  while (cnt-- > 0)
	    if (cnt != LC_ALL)
	      munmap (data[cnt].addr, data[cnt].size);

	  error (0, 0, _("cannot read all files in \"%s\": ignored"), fname);

	  continue;
	}

      result |= add_locale_to_archive (&ah, basename (fname), data, replace);

      for (cnt = 0; cnt < __LC_LAST; ++cnt)
	if (cnt != LC_ALL)
	  munmap (data[cnt].addr, data[cnt].size);
    }

  /* We are done.  */
  close_archive (&ah);

  return result;
}


int
delete_locales_from_archive (nlist, list)
     size_t nlist;
     char *list[];
{
  struct locarhandle ah;
  struct locarhead *head;
  struct namehashent *namehashtab;

  /* Open the archive.  This call never returns if we cannot
     successfully open the archive.  */
  open_archive (&ah);

  head = ah.addr;
  namehashtab = (struct namehashent *) ((char *) ah.addr
					+ head->namehash_offset);

  while (nlist-- > 0)
    {
      const char *locname = *list++;
      uint32_t hval;
      unsigned int idx;
      unsigned int incr;

      /* Search for this locale in the archive.  */
      hval = compute_hashval (locname, strlen (locname));

      idx = hval % head->namehash_size;
      incr = 1 + hval % (head->namehash_size - 2);

      /* If the name_offset field is zero this means this is no
	 deleted entry and therefore no entry can be found.  */
      while (namehashtab[idx].name_offset != 0)
	{
	  if (namehashtab[idx].hashval == hval
	      && (strcmp (locname,
			  (char *) ah.addr + namehashtab[idx].name_offset)
		  == 0))
	    {
	      /* Found the entry.  Now mark it as removed by zero-ing
		 the reference to the locale record.  */
	      namehashtab[idx].locrec_offset = 0;
	      --head->namehash_used;
	      break;
	    }

	  idx += incr;
	  if (idx >= head->namehash_size)
	    idx -= head->namehash_size;
	}

      if (namehashtab[idx].name_offset == 0 && ! be_quiet)
	error (0, 0, _("locale \"%s\" not in archive"), locname);
    }

  close_archive (&ah);

  return 0;
}


static int
xstrcmp (const void *a, const void *b)
{
  return strcmp (*(const char **) a, *(const char **) b);
}


void
show_archive_content (void)
{
  struct locarhandle ah;
  struct locarhead *head;
  struct namehashent *namehashtab;
  int cnt;
  char **names;
  int used;

  /* Open the archive.  This call never returns if we cannot
     successfully open the archive.  */
  open_archive (&ah);

  head = ah.addr;

  names = (char **) xmalloc (head->namehash_used * sizeof (char *));

  namehashtab = (struct namehashent *) ((char *) ah.addr
					+ head->namehash_offset);
  for (cnt = used = 0; cnt < head->namehash_size; ++cnt)
    if (namehashtab[cnt].locrec_offset != 0)
      {
	assert (used < head->namehash_used);
	names[used++] = ah.addr + namehashtab[cnt].name_offset;
      }

  /* Sort the names.  */
  qsort (names, used, sizeof (char *), xstrcmp);

  for (cnt = 0; cnt < used; ++cnt)
    puts (names[cnt]);

  close_archive (&ah);

  exit (EXIT_SUCCESS);
}
