/* Temporary file handling for tests.
   Copyright (C) 1998-2017 Free Software Foundation, Inc.
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

/* This is required to get an mkstemp which can create large files on
   some 32-bit platforms. */
#define _FILE_OFFSET_BITS 64

#include <support/temp_file.h>
#include <support/temp_file-internal.h>
#include <support/support.h>

#include <paths.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* List of temporary files.  */
static struct temp_name_list
{
  struct qelem q;
  char *name;
} *temp_name_list;

/* Location of the temporary files.  Set by the test skeleton via
   support_set_test_dir.  The string is not be freed.  */
static const char *test_dir = _PATH_TMP;

void
add_temp_file (const char *name)
{
  struct temp_name_list *newp
    = (struct temp_name_list *) xcalloc (sizeof (*newp), 1);
  char *newname = strdup (name);
  if (newname != NULL)
    {
      newp->name = newname;
      if (temp_name_list == NULL)
	temp_name_list = (struct temp_name_list *) &newp->q;
      else
	insque (newp, temp_name_list);
    }
  else
    free (newp);
}

int
create_temp_file (const char *base, char **filename)
{
  char *fname;
  int fd;

  fname = (char *) xmalloc (strlen (test_dir) + 1 + strlen (base)
			    + sizeof ("XXXXXX"));
  strcpy (stpcpy (stpcpy (stpcpy (fname, test_dir), "/"), base), "XXXXXX");

  fd = mkstemp (fname);
  if (fd == -1)
    {
      printf ("cannot open temporary file '%s': %m\n", fname);
      free (fname);
      return -1;
    }

  add_temp_file (fname);
  if (filename != NULL)
    *filename = fname;
  else
    free (fname);

  return fd;
}

/* Helper functions called by the test skeleton follow.  */

void
support_set_test_dir (const char *path)
{
  test_dir = path;
}

void
support_delete_temp_files (void)
{
  while (temp_name_list != NULL)
    {
      remove (temp_name_list->name);
      free (temp_name_list->name);

      struct temp_name_list *next
	= (struct temp_name_list *) temp_name_list->q.q_forw;
      free (temp_name_list);
      temp_name_list = next;
    }
}

void
support_print_temp_files (FILE *f)
{
  if (temp_name_list != NULL)
    {
      struct temp_name_list *n;
      fprintf (f, "temp_files=(\n");
      for (n = temp_name_list;
           n != NULL;
           n = (struct temp_name_list *) n->q.q_forw)
        fprintf (f, "  '%s'\n", n->name);
      fprintf (f, ")\n");
    }
}
