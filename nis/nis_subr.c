/* Copyright (c) 1997, 1999, 2000, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <errno.h>
#include <string.h>
#include <rpcsvc/nis.h>

nis_name
nis_leaf_of (const_nis_name name)
{
  static char result[NIS_MAXNAMELEN + 1];

  return nis_leaf_of_r (name, result, NIS_MAXNAMELEN);
}

nis_name
nis_leaf_of_r (const_nis_name name, char *buffer, size_t buflen)
{
  size_t i = 0;

  buffer[0] = '\0';

  while (name[i] != '.' && name[i] != '\0')
    i++;

  if (i > buflen - 1)
    {
      __set_errno (ERANGE);
      return NULL;
    }

  if (i > 0)
    {
      if ((size_t)i >= buflen)
	{
	  __set_errno (ERANGE);
	  return NULL;
	}

      *((char *) __mempcpy (buffer, name, i)) = '\0';
    }

  return buffer;
}
libnsl_hidden_def (nis_leaf_of_r)

nis_name
nis_name_of (const_nis_name name)
{
  static char result[NIS_MAXNAMELEN + 1];

  return nis_name_of_r (name, result, NIS_MAXNAMELEN);
}

nis_name
nis_name_of_r (const_nis_name name, char *buffer, size_t buflen)
{
  char *local_domain;
  int diff;

  local_domain = nis_local_directory ();

  diff = strlen (name) - strlen (local_domain);
  if (diff <= 0)
    return NULL;

  if (strcmp (&name[diff], local_domain) != 0)
    return NULL;

  if ((size_t) diff >= buflen)
    {
      __set_errno (ERANGE);
      return NULL;
    }

  *((char *) __mempcpy (buffer, name, diff - 1)) = '\0';

  if (diff - 1 == 0)
    return NULL;

  return buffer;
}
libnsl_hidden_def (nis_name_of_r)

static int
count_dots (const_nis_name str)
{
  int count = 0;
  size_t l = strlen (str);

  for (size_t i = 0; i < l; ++i)
    if (str[i] == '.')
      ++count;

  return count;
}

/* If we run out of memory, we don't give already allocated memory
   free. The overhead for bringing getnames back in a safe state to
   free it is to big. */
nis_name *
nis_getnames (const_nis_name name)
{
  nis_name *getnames = NULL;
  char local_domain[NIS_MAXNAMELEN + 1];
  char *path;
  char *cp;
  int count;
  int pos = 0;
  int have_point;
  char *saveptr;

  strncpy (local_domain, nis_local_directory (), NIS_MAXNAMELEN);
  local_domain[NIS_MAXNAMELEN] = '\0';

  count = 1;
  getnames = malloc ((count + 1) * sizeof (char *));
  if (__builtin_expect (getnames == NULL, 0))
      return NULL;

  /* Do we have a fully qualified NIS+ name ? If yes, give it back */
  if (name[strlen (name) - 1] == '.')
    {
      if ((getnames[0] = strdup (name)) == NULL)
	{
	free_null:
	  while (pos-- > 0)
	    free (getnames[pos]);
	  free (getnames);
	  return NULL;
	}

      getnames[1] = NULL;

      return getnames;
    }

  /* Get the search path, where we have to search "name" */
  path = getenv ("NIS_PATH");
  if (path == NULL)
    path = strdupa ("$");
  else
    path = strdupa (path);

  have_point = (strchr (name, '.') != NULL);

  cp = __strtok_r (path, ":", &saveptr);
  while (cp)
    {
      if (strcmp (cp, "$") == 0)
	{
	  char *cptr = local_domain;
	  char *tmp;

	  while ((have_point && *cptr != '\0') || (count_dots (cptr) >= 2))
	    {
	      if (pos >= count)
		{
		  count += 5;
		  nis_name *newp = realloc (getnames,
					    (count + 1) * sizeof (char *));
		  if (__builtin_expect (newp == NULL, 0))
		    goto free_null;
		  getnames = newp;
		}
	      tmp = malloc (strlen (cptr) + strlen (local_domain) +
			    strlen (name) + 2);
	      if (__builtin_expect (tmp == NULL, 0))
		goto free_null;

	      getnames[pos] = tmp;
	      tmp = stpcpy (tmp, name);
	      *tmp++ = '.';
	      if (cptr[1] != '\0')
		stpcpy (tmp, cptr);
	      else
		++cptr;

	      ++pos;

	      while (*cptr != '.' && *cptr != '\0')
		++cptr;
	      if (cptr[0] != '\0' && cptr[1] != '\0')
		/* If we have only ".", don't remove the "." */
		++cptr;
	    }
	}
      else
	{
	  char *tmp;
	  size_t cplen = strlen (cp);

	  if (cp[cplen - 1] == '$')
	    {
	      char *p;

	      tmp = malloc (cplen + strlen (local_domain) + strlen (name) + 2);
	      if (__builtin_expect (tmp == NULL, 0))
		goto free_null;

	      p = __stpcpy (tmp, name);
	      *p++ = '.';
	      p = __mempcpy (p, cp, cplen);
	      --p;
	      if (p[-1] != '.')
		*p++ = '.';
	      __stpcpy (p, local_domain);
	    }
	  else
	    {
	      char *p;

	      tmp = malloc (cplen + strlen (name) + 2);
	      if (__builtin_expect (tmp == NULL, 0))
		goto free_null;

	      p = __stpcpy (tmp, name);
	      *p++ = '.';
	      memcpy (p, cp, cplen + 1);
	    }

	  if (pos >= count)
	    {
	      count += 5;
	      nis_name *newp = realloc (getnames,
					(count + 1) * sizeof (char *));
	      if (__builtin_expect (newp == NULL, 0))
		goto free_null;
	      getnames = newp;
	    }
	  getnames[pos] = tmp;
	  ++pos;
	}
      cp = __strtok_r (NULL, ":", &saveptr);
    }

  getnames[pos] = NULL;

  return getnames;
}
libnsl_hidden_def (nis_getnames)

void
nis_freenames (nis_name *names)
{
  int i = 0;

  while (names[i] != NULL)
    {
      free (names[i]);
      ++i;
    }

  free (names);
}
libnsl_hidden_def  (nis_freenames)

name_pos
nis_dir_cmp (const_nis_name n1, const_nis_name n2)
{
  int len1, len2;

  len1 = strlen (n1);
  len2 = strlen (n2);

  if (len1 == len2)
    {
      if (strcmp (n1, n2) == 0)
	return SAME_NAME;
      else
	return NOT_SEQUENTIAL;
    }

  if (len1 < len2)
    {
      if (n2[len2 - len1 - 1] != '.')
	return NOT_SEQUENTIAL;
      else if (strcmp (&n2[len2 - len1], n1) == 0)
	return HIGHER_NAME;
      else
	return NOT_SEQUENTIAL;
    }
  else
    {
      if (n1[len1 - len2 - 1] != '.')
	return NOT_SEQUENTIAL;
      else if (strcmp (&n1[len1 - len2], n2) == 0)
	return LOWER_NAME;
      else
	return NOT_SEQUENTIAL;

    }
}
libnsl_hidden_def (nis_dir_cmp)

void
nis_destroy_object (nis_object *obj)
{
  nis_free_object (obj);
}
libnsl_hidden_def (nis_destroy_object)
