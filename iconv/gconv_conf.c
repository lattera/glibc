/* Handle configuration data.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#include <gconv_int.h>


/* This is the default path where we look for module lists.  */
static const char default_gconv_path[] = GCONV_PATH;

/* Name of the file containing the module information in the directories
   along the path.  */
static const char gconv_conf_filename[] = "gconv-modules";

/* Filename extension for the modules.  */
#ifndef MODULE_EXT
# define MODULE_EXT ".so"
#endif
static const char gconv_module_ext[] = MODULE_EXT;

/* We have a few builtin transformations.  */
static struct gconv_module builtin_modules[] =
{
#define BUILTIN_TRANSFORMATION(From, ConstPfx, ConstLen, To, Cost, Name, \
			       Fct, Init, End, MinF, MaxF, MinT, MaxT) \
  {									      \
    from_pattern: From,							      \
    from_constpfx: ConstPfx,						      \
    from_constpfx_len: ConstLen,					      \
    from_regex: NULL,							      \
    to_string: To,							      \
    cost_hi: Cost,							      \
    cost_lo: INT_MAX,							      \
    module_name: Name							      \
  },
#define BUILTIN_ALIAS(From, To)

#include "gconv_builtin.h"
};

#undef BUILTIN_TRANSFORMATION
#undef BUILTIN_ALIAS

static const char *
builtin_aliases[] =
{
#define BUILTIN_TRANSFORMATION(From, ConstPfx, ConstLen, To, Cost, Name, \
			       Fct, Init, End, MinF, MaxF, MinT, MaxT)
#define BUILTIN_ALIAS(From, To) From " " To,

#include "gconv_builtin.h"
};

#ifdef USE_IN_LIBIO
# include <libio/libioP.h>
# define __getdelim(line, len, c, fp) _IO_getdelim (line, len, c, fp)
#endif


/* Function for searching module.  */
static int
module_compare (const void *p1, const void *p2)
{
  struct gconv_module *s1 = (struct gconv_module *) p1;
  struct gconv_module *s2 = (struct gconv_module *) p2;
  int result;

  if (s1->from_pattern == NULL)
    {
      if (s2->from_pattern == NULL)
	result = strcmp (s1->from_constpfx, s2->from_constpfx);
      else
	result = -1;
    }
  else if (s2->from_pattern == NULL)
    result = 1;
  else
    result = strcmp (s1->from_pattern, s2->from_pattern);

  if (result == 0)
    result = strcmp (s1->to_string, s2->to_string);

  return result;
}


/* Add new alias.  */
static inline void
add_alias (char *rp)
{
  /* We now expect two more string.  The strings are normalized
     (converted to UPPER case) and strored in the alias database.  */
  struct gconv_alias *new_alias;
  char *from, *to, *wp;

  while (isspace (*rp))
    ++rp;
  from = wp = rp;
  while (*rp != '\0' && !isspace (*rp))
    ++rp;
  if (*rp == '\0')
    /* There is no `to' string on the line.  Ignore it.  */
    return;
  *rp++ = '\0';
  to = wp = rp;
  while (isspace (*rp))
    ++rp;
  while (*rp != '\0' && !isspace (*rp))
    *wp++ = *rp++;
  if (to == wp)
    /* No `to' string, ignore the line.  */
    return;
  *wp++ = '\0';

  new_alias = (struct gconv_alias *)
    malloc (sizeof (struct gconv_alias) + (wp - from));
  if (new_alias != NULL)
    {
      new_alias->fromname = memcpy ((char *) new_alias
				    + sizeof (struct gconv_alias),
				    from, wp - from);
      new_alias->toname = new_alias->fromname + (to - from);

      if (__tsearch (new_alias, &__gconv_alias_db, __gconv_alias_compare)
	  == NULL)
	/* Something went wrong, free this entry.  */
	free (new_alias);
    }
}


/* Add new module.  */
static inline void
add_module (char *rp, const char *directory, size_t dir_len, void **modules,
	    size_t *nmodules, int modcounter)
{
  /* We expect now
     1. `from' name
     2. `to' name
     3. filename of the module
     4. an optional cost value
  */
  struct gconv_module *new_module;
  char *from, *to, *module, *wp;
  size_t const_len;
  int from_is_regex;
  int need_ext;
  int cost_hi;

  while (isspace (*rp))
    ++rp;
  from = rp;
  from_is_regex = 0;
  while (*rp != '\0' && !isspace (*rp))
    {
      if (!isalnum (*rp) && *rp != '-' && *rp != '/' && *rp != '.'
	  && *rp != '_')
	from_is_regex = 1;
      ++rp;
    }
  if (*rp == '\0')
    return;
  *rp++ = '\0';
  to = wp = rp;
  while (isspace (*rp))
    ++rp;
  while (*rp != '\0' && !isspace (*rp))
    *wp++ = *rp++;
  if (*rp == '\0')
    return;
  *wp++ = '\0';
  do
    ++rp;
  while (isspace (*rp));
  module = wp;
  while (*rp != '\0' && !isspace (*rp))
    *wp++ = *rp++;
  if (*rp == '\0')
    {
      /* There is no cost, use one by default.  */
      *wp++ = '\0';
      cost_hi = 1;
    }
  else
    {
      /* There might be a cost value.  */
      char *endp;

      *wp++ = '\0';
      cost_hi = strtol (rp, &endp, 10);
      if (rp == endp)
	/* No useful information.  */
	cost_hi = 1;
    }

  if (module[0] == '\0')
    /* No module name given.  */
    return;
  if (module[0] == '/')
    dir_len = 0;
  else
    /* Increment by one for the slash.  */
    ++dir_len;

  /* See whether we must add the ending.  */
  need_ext = 0;
  if (wp - module < sizeof (gconv_module_ext)
      || memcmp (wp - sizeof (gconv_module_ext), gconv_module_ext,
		 sizeof (gconv_module_ext)) != 0)
    /* We must add the module extension.  */
    need_ext = sizeof (gconv_module_ext) - 1;

  /* We've collected all the information, now create an entry.  */

  if (from_is_regex)
    {
      const_len = 0;
      while (isalnum (from[const_len]) || from[const_len] == '-'
	     || from[const_len] == '/' || from[const_len] == '.'
	     || from[const_len] == '_')
	++const_len;
    }
  else
    const_len = to - from - 1;

  new_module = (struct gconv_module *) malloc (sizeof (struct gconv_module)
					       + (wp - from)
					       + dir_len + need_ext);
  if (new_module != NULL)
    {
      char *tmp;

      new_module->from_constpfx = memcpy ((char *) new_module
					  + sizeof (struct gconv_module),
					  from, to - from);
      if (from_is_regex)
	new_module->from_pattern = new_module->from_constpfx;
      else
	new_module->from_pattern = NULL;

      new_module->from_constpfx_len = const_len;

      new_module->from_regex = NULL;

      new_module->to_string = memcpy ((char *) new_module->from_constpfx
				      + (to - from), to, module - to);

      new_module->cost_hi = cost_hi;
      new_module->cost_lo = modcounter;

      new_module->module_name = (char *) new_module->to_string + (module - to);

      if (dir_len == 0)
	tmp = (char *) new_module->module_name;
      else
	{
	  tmp = __mempcpy ((char *) new_module->module_name,
			   directory, dir_len - 1);
	  *tmp++ = '/';
	}

      tmp = __mempcpy (tmp, module, wp - module);

      if (need_ext)
	memcpy (tmp - 1, gconv_module_ext, sizeof (gconv_module_ext));

      if (__tfind (new_module, modules, module_compare) == NULL)
	{
	  if (__tsearch (new_module, modules, module_compare) == NULL)
	    /* Something went wrong while inserting the new module.  */
	    free (new_module);
	  else
	    ++*nmodules;
	}
    }
}


static void
insert_module (const void *nodep, VISIT value, int level)
{
  if (value == preorder || value == leaf)
    __gconv_modules_db[__gconv_nmodules++] = *(struct gconv_module **) nodep;
}

static void
nothing (void *unused __attribute__ ((unused)))
{
}


/* Read the next configuration file.  */
static void
internal_function
read_conf_file (const char *filename, const char *directory, size_t dir_len,
		void **modules, size_t *nmodules)
{
  FILE *fp = fopen (filename, "r");
  char *line = NULL;
  size_t line_len = 0;
  int modcounter = 0;

  /* Don't complain if a file is not present or readable, simply silently
     ignore it.  */
  if (fp == NULL)
    return;

  /* Process the known entries of the file.  Comments start with `#' and
     end with the end of the line.  Empty lines are ignored.  */
  while (!feof_unlocked (fp))
    {
      char *rp, *endp, *word;
      ssize_t n = __getdelim (&line, &line_len, '\n', fp);
      if (n < 0)
	/* An error occurred.  */
	break;

      rp = line;
      /* Terminate the line (excluding comments or newline) by an NUL byte
	 to simplify the following code.  */
      endp = strchr (rp, '#');
      if (endp != NULL)
	*endp = '\0';
      else
	if (rp[n - 1] == '\n')
	  rp[n - 1] = '\0';

      while (isspace (*rp))
	++rp;

      /* If this is an empty line go on with the next one.  */
      if (rp == endp)
	continue;

      word = rp;
      while (*rp != '\0' && !isspace (*rp))
	++rp;

      if (rp - word == sizeof ("alias") - 1
	  && memcmp (word, "alias", sizeof ("alias") - 1) == 0)
	add_alias (rp);
      else if (rp - word == sizeof ("module") - 1
	       && memcmp (word, "module", sizeof ("module") - 1) == 0)
	add_module (rp, directory, dir_len, modules, nmodules, modcounter++);
      /* else */
	/* Otherwise ignore the line.  */
    }

  if (line != NULL)
    free (line);
  fclose (fp);
}


/* Read all configuration files found in the user-specified and the default
   path.  */
void
__gconv_read_conf (void)
{
  const char *user_path = __secure_getenv ("GCONV_PATH");
  char *gconv_path, *elem;
  void *modules = NULL;
  size_t nmodules = 0;
  int save_errno = errno;
  size_t cnt;

  if (user_path == NULL)
    /* No user-defined path.  Make a modifiable copy of the default path.  */
    gconv_path = strdupa (default_gconv_path);
  else
    {
      /* Append the default path to the user-defined path.  */
      size_t user_len = strlen (user_path);
      char *tmp;

      gconv_path = alloca (user_len + 1 + sizeof (default_gconv_path));
      tmp = __mempcpy (gconv_path, user_path, user_len);
      *tmp++ = ':';
      __mempcpy (tmp, default_gconv_path, sizeof (default_gconv_path));
    }

  elem = strtok_r (gconv_path, ":", &gconv_path);
  while (elem != NULL)
    {
#ifndef MAXPATHLEN
      /* We define a reasonable limit.  */
# define MAXPATHLEN 4096
#endif
      char real_elem[MAXPATHLEN];

      if (__realpath (elem, real_elem) != NULL)
	{
	  size_t elem_len = strlen (real_elem);
	  char *filename, *tmp;

	  filename = alloca (elem_len + 1 + sizeof (gconv_conf_filename));
	  tmp = __mempcpy (filename, real_elem, elem_len);
	  *tmp++ = '/';
	  __mempcpy (tmp, gconv_conf_filename, sizeof (gconv_conf_filename));

	  /* Read the next configuration file.  */
	  read_conf_file (filename, real_elem, elem_len, &modules, &nmodules);
	}

      /* Get next element in the path.  */
      elem = strtok_r (NULL, ":", &gconv_path);
    }

  /* If the configuration files do not contain any valid module specification
     remember this by setting the pointer to the module array to NULL.  */
  nmodules += sizeof (builtin_modules) / sizeof (builtin_modules[0]);
  if (nmodules == 0)
    __gconv_modules_db = NULL;
  else
    {
      __gconv_modules_db =
	(struct gconv_module **) malloc (nmodules
					 * sizeof (struct gconv_module));
      if (__gconv_modules_db != NULL)
	{
	  size_t cnt;

	  /* Insert all module entries into the array.  */
	  __twalk (modules, insert_module);

	  /* No remove the tree data structure.  */
	  __tdestroy (modules, nothing);

	  /* Finally insert the builtin transformations.  */
	  for (cnt = 0; cnt < (sizeof (builtin_modules)
			       / sizeof (struct gconv_module)); ++cnt)
	    __gconv_modules_db[__gconv_nmodules++] = &builtin_modules[cnt];
	}
    }

  /* Add aliases for builtin conversions.  */
  cnt = sizeof (builtin_aliases) / sizeof (builtin_aliases[0]);
  while (cnt > 0)
    {
      char *copy = strdupa (builtin_aliases[--cnt]);
      add_alias (copy);
    }

  /* Restore the error number.  */
  __set_errno (save_errno);
}
