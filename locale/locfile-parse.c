/* Copyright (C) 1995 Free Software Foundation, Inc.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <langinfo.h>
#include <libintl.h>
#include <limits.h>
#include <obstack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include "localedef.h"
#include "localeinfo.h"
#include "token.h"

/* We don't have these constants defined because we don't use them.  Give
   default values.  */
#define CTYPE_MB_CUR_MIN 0
#define CTYPE_MB_CUR_MAX 0
#define CTYPE_HASH_SIZE 0
#define CTYPE_HASH_LAYERS 0
#define CTYPE_CLASS 0
#define CTYPE_TOUPPER_EB 0
#define CTYPE_TOLOWER_EB 0
#define CTYPE_TOUPPER_EL 0
#define CTYPE_TOLOWER_EL 0
 

/* We have all categories defined in `categories.def'.  Now construct
   the description and data structure used for all categories.  */
#define DEFINE_CATEGORY(category, name, items, postload, in, check, out)      \
    struct cat_item category##_desc[] =					      \
      {									      \
        NO_PAREN items							      \
      };								      \
									      \
    char *category##_values[NELEMS (category##_desc) - 1] = { NULL, };
#include "categories.def"
#undef DEFINE_CATEGORY

struct category category[] =
  {
#define DEFINE_CATEGORY(category, name, items, postload, in, check, out)      \
    [category] = { _NL_NUM_##category, name, NELEMS (category##_desc) - 1,    \
                   category##_desc, category##_values, in, check, out },
#include "categories.def"
#undef DEFINE_CATEGORY
  };
#define NCATEGORIES NELEMS (category)


#define SYNTAX_ERROR							      \
    error (0, 0, gettext ("%s:%Zd: syntax error in locale definition file"),  \
	   locfile_data.filename, locfile_data.line_no)


/* Prototypes for local functions.  */
static int get_byte (char *byte_ptr);
static char *is_locale_name (int cat_no, const char *str, int len);


/* Read a locale definition file FILE.  The format is defined in
   POSIX.2 2.5.3.  */
void
locfile_read (const char *fname)
{
  /* Pointer to text of last token.  */
  char *ptr;
  /* Length of last token (or if NUMBER the value itself).  */
  int len;
  /* The last returned token.  */
  int token;
  /* For error correction we remember whether the last token was correct.  */
  int correct_token = 1;

  /* Open the desired input file on stdin.  */
  locfile_open (fname);

  while ((token = locfile_lex (&ptr, &len)) != 0)
    {
      int cat_no;

      for (cat_no = 0; cat_no < NCATEGORIES; ++cat_no)
	if (token == category[cat_no].cat_id)
	  break;

      if (cat_no >= NCATEGORIES)
	/* A syntax error occured.  No valid category defintion starts.  */
	{
	  if (correct_token != 0)
	    error (0, 0, gettext ("%s:%Zd: locale category start expected"),
		   locfile_data.filename, locfile_data.line_no);

	  /* To prevent following errors mark as error case.  */
	  correct_token = 0;

	  /* Synchronization point is the beginning of a new category.
	     Overread all line upto this silently.  */
	  ignore_to_eol (0, 0);
	  continue;
	}

      /* Rest of the line should be empty.  */
      ignore_to_eol (0, 1);

      /* Perhaps these category is already specified.  We simply give a
         warning and overwrite the values.  */
      if (category[cat_no].filled != 0)
	error (0, 0, gettext ("%s:%Zd: multiple definition of locale "
			      "category %s"), locfile_data.filename,
	       locfile_data.line_no, category[cat_no].name);

      /* We read the first token because this could be the copy statement.  */
      token = xlocfile_lex (&ptr, &len);

      if (token == TOK_COPY)
	/* Copying the definitions from an existing locale is requested.  */
	{
	  char *str;

	  /* Get the name of the locale to copy from.  */
	  token = xlocfile_lex (&ptr, &len);
	  if (token != TOK_IDENT && token != TOK_STRING)
	    /* No name, then mark error and ignore everything upto next
	       start of an category section.  */
	    {
	      /* To prevent following errors mark as error case.  */
	      correct_token = 0;

	      /* Synchronization point is the beginning of a new category.
		 Overread all line upto this silently.  */
	      ignore_to_eol (0, 0);
	    }
	  else if ((str = is_locale_name (cat_no, ptr, len)) != NULL)
	    /* Yes the name really names an existing locale file.  We are
	       returned the complete file name.  Store it so that we can
	       copy it in the output phase.  */
	    {
	      category[cat_no].copy_locale = str;
	      category[cat_no].filled = 1;
	      
	      ignore_to_eol (0, 1);
	    }
	  else
	    /* No, the name does not address a valid locale file.  Mark
	       error case and ignore rest of category.  */
	    {
	      char tmp[len + 1];
	      memcpy (tmp, ptr, len);
	      tmp[len] = '\0';
	      error (0, 0, gettext ("%s:%Zd: invalid locale `%s' in copy "
				    "statement"), locfile_data.filename,
			 locfile_data.line_no, tmp);
	      correct_token = 0;
	      ignore_to_eol (0, 0);
	    }

	  /* This should END as the next token.  */
	  token = xlocfile_lex (&ptr, &len);

	  if (token == TOK_END)
	    /* This is the end of the category.  */
	    {
	      token = xlocfile_lex (&ptr, &len);

	      if (token != category[cat_no].cat_id)
		/* Wrong category name after END.  */
		{
		  error (0, 0, gettext ("%s:%Zd: category `%s' does not "
					"end with `END %s'"),
			 locfile_data.filename, locfile_data.line_no,
			 category[cat_no].name, category[cat_no].name);
		  ignore_to_eol (0, 0);
		}
	      else
		ignore_to_eol (0, 1);

	      correct_token = 1;
	    }
	  else
	    /* No END following copy.  Give error while not in error case.  */
	    {
	      if (correct_token != 0)
		error (0, 0, gettext ("%s:%Zd: `copy' must be sole rule"),
		       locfile_data.filename, locfile_data.line_no);
	      correct_token = 0;
	      ignore_to_eol (0, 0);
	    }

	  continue;
	}

      /* Now it's time to mark as mentioned in the locale file.  */
      category[cat_no].filled = 1;

      if (category[cat_no].infct != NULL)
	/* The category needs a special input handling.  */
	{
	  category[cat_no].infct(token);
	  continue;
	}

      /* Now process the given items.  */
      while (1)
	{
	  int item_no;

	  if (token == TOK_END)
	    /* This is the end of the category.  */
	    {
	      token = xlocfile_lex (&ptr, &len);

	      if (token != category[cat_no].cat_id)
		{
		  error (0, 0, gettext ("%s:%Zd: category `%s' does not end "
					"with `END %s'"),
			 locfile_data.filename, locfile_data.line_no,
			 category[cat_no].name, category[cat_no].name);
		  ignore_to_eol (0, 0);
		}
	      else
		ignore_to_eol (0, 1);

	      /* Start next category.  */
	      break;
	    }

	  /* All other lines should describe valid items of the category.  */
	  for (item_no = 0; item_no < category[cat_no].number; ++item_no)
	    if (category[cat_no].item_desc[item_no].item_id == token)
	      break;

	  if (item_no >= category[cat_no].number)
	    /* This is not a valid item of the category.  */
	    {
	      SYNTAX_ERROR;
	      ignore_to_eol (0, 0);

	      token = xlocfile_lex (&ptr, &len);

	      /* And process next item.  */
	      continue;
	    }

	  /* Test whether already a value is defined.  */
	  if (category[cat_no].item_value[item_no] != NULL)
	    error (0, 0, gettext ("%s:%Zd: category item `%s' already "
				  "defined"),
		   locfile_data.filename, locfile_data.line_no,
		   category[cat_no].item_desc[item_no].name);

	  switch (category[cat_no].item_desc[item_no].value_type)
	    {
	    case string:
	      /* Get next token.  This is the argument to the item.  */
	      token = xlocfile_lex (&ptr, &len);

	      if (token != TOK_STRING)
		SYNTAX_ERROR;
	      else
		category[cat_no].item_value[item_no] = strdup (ptr);
	      ignore_to_eol (0, ptr != NULL);
	      break;
	    case stringarray:
	      /* This is a difficult case.  The number of strings in
		 the array may vary.  But for now its only necessary
		 with ALT_DIGITS from LC_TIME.  This item is the last
		 so be can solve it by storing the number of string in
		 the first place and the string indeces following
		 that.  */
	      {
		int cnt;
		char **buffer;
		if (category[cat_no].item_value[item_no] != NULL)
		  buffer = (char **) category[cat_no].item_value[item_no];
		else
		  buffer = (char **) xmalloc (
		    sizeof (char *) * category[cat_no].item_desc[item_no].max);

		category[cat_no].item_value[item_no] = (char *) buffer;

		/* As explained we may need a place to store the real number
		   of strings.  */
		if (category[cat_no].item_desc[item_no].min
		    != category[cat_no].item_desc[item_no].max)
		  ++buffer;

		cnt = 0;
		do
		  {
		    token = xlocfile_lex (&ptr, &len);
		    if (token != TOK_STRING)
		      {
			SYNTAX_ERROR;
			break;
		      }

		    if (cnt >= category[cat_no].item_desc[item_no].max)
		      {
			error (0, 0, gettext ("%s:%Zd: too many elements "
					      "for item `%s`"),
			       locfile_data.filename, locfile_data.line_no,
			       category[cat_no].item_desc[item_no].name);
			break;
		      }

		    buffer[cnt++] = strdup (ptr);

		    token = locfile_lex (&ptr, &len);
		  }
		while (token == TOK_CHAR && len == ';');

		ignore_to_eol (token, ptr != NULL);

		if (cnt < category[cat_no].item_desc[item_no].min)
		  error (0, 0, gettext ("%s:%Zd: too few elements for item "
					"`%s'"),
			 locfile_data.filename, locfile_data.line_no,
			 category[cat_no].item_desc[item_no].name);

		if (category[cat_no].item_desc[item_no].min
		    != category[cat_no].item_desc[item_no].max)
		  *(int *) category[cat_no].item_value[item_no] = cnt;
	      }
	      break;
	    case byte:
	      {
		int ok;
		category[cat_no].item_value[item_no] = (char *) xmalloc (
		  __alignof__ (char));
		ok = get_byte (category[cat_no].item_value[item_no]);
		ignore_to_eol (0, ok);
	      }
	      break;
	    case bytearray:
	      {
		char *buffer;
		int maxsize;
		int cnt;
		char byte;
		int ok;

		buffer = (char *) xmalloc ((maxsize = 30));
		cnt = 0;

		while ((ok = get_byte (&byte)))
		  {
		    if (cnt >= maxsize)
		      buffer = (char *) xmalloc ((maxsize *= 2));

		    buffer[cnt++] = byte;

		    token = locfile_lex (&ptr, &len);
		    if (token != TOK_CHAR || len != ';')
		      break;
		  }

		buffer[cnt] = '\0';
		category[cat_no].item_value[item_no] = buffer;
		ignore_to_eol (token, ok);
	      }
	      break;
	    default:
	      error (5, 0, gettext ("internal error in %s, line %u"),
		     __FUNCTION__, __LINE__);
	      /* NOTREACHED */
	    }

	  /* Get next token.  */
	  token = xlocfile_lex (&ptr, &len);
	} /* while (1) */
    }
}


/* Check given values for categories for consistency.  */
void
categories_check (void)
{
  int cat_no;

  for (cat_no = 0; cat_no < NCATEGORIES; ++cat_no)
    if (category[cat_no].copy_locale == NULL)
      if (category[cat_no].filled != 0)
	if (category[cat_no].checkfct)
	  category[cat_no].checkfct();
	else
	  {
	    int item_no;

	    for (item_no = 0; item_no < category[cat_no].number; ++item_no)
	      if (category[cat_no].item_value[item_no] == NULL)
		{
		  int errcode;

		  /* If the item is defined in the standard is it an error to
		     have it not defined.  */
		  errcode = category[cat_no].item_desc[item_no].status == std
		            ? 5 : 0;

		  error (errcode, 0, gettext ("item `%s' of category `%s' "
					      "undefined"),
			 category[cat_no].item_desc[item_no].name,
			 category[cat_no].name);
		}
	  }
      else
	error (0, 0, gettext ("category `%s' not defined"),
	       category[cat_no].name);
}


/* Write out the binary representation of the category data which can be
   loaded by setlocale(1).  */
void
categories_write (void)
{
  struct locale_file
  {
      int magic;
      int n;
      int idx[0];
  } *data;
  struct obstack obstk;
  int cat_no;

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free
  obstack_init (&obstk);

  for (cat_no = 0; cat_no < NCATEGORIES; ++cat_no)
    {
      int result = 0;

      if (category[cat_no].copy_locale != NULL)
	/* Simply copy the addressed locale file of the specified
	   category.  Please note that this is tried before the distinction
	   between categories which need special handling is made.  */
	{
	  int source;

	  /* Open source file.  */
	  source = open (category[cat_no].copy_locale, O_RDONLY);
	  if (source < 0)
	    error (0, 0, gettext ("cannot copy locale definition file `%s'"),
		   category[cat_no].copy_locale);
	  else
	    {
	      /* Construct file name of output file and open for writing.  */
	      char path[strlen (output_path)
                        + strlen(category[cat_no].name) + 1];
	      int dest;
	      char *t;

	      t = stpcpy (path, output_path);
	      strcpy (t, category[cat_no].name);

	      dest = creat (path, 0666);
	      if (dest == -1)
		error (0, 0, gettext ("cannot open output file `%s': %m"),
		       path);
	      else
		{
		  char buffer[BUFSIZ];
		  int size;
		  
		  /* Copy the files.  */
		  do
		    {
		      size = read (source, buffer, BUFSIZ);
		      write (dest, buffer, size);
		    }
		  while (size > 0);

		  close (dest);

		  /* Show success.  */
		  puts (category[cat_no].name);
		}
	      close (source);
	    }

	  /* Next category.   */
	  continue;
	}

      if (category[cat_no].outfct)
	result = category[cat_no].outfct();
      else
	{
	  char *path, *t;
	  int fd;
	  struct iovec *iov;
	  int item_no, len, slen, cnt;
	  int elems = 0;

	  /* Count number of elements.  */
	  for (item_no = 0; item_no < category[cat_no].number; ++item_no)
	    {
	      switch (category[cat_no].item_desc[item_no].value_type)
		{
		case string:
		case byte:
		case bytearray:
		  ++elems;
		  break;
		case stringarray:
		  elems += category[cat_no].item_desc[item_no].max;
		  break;
		default:
		  error (5, 0, gettext ("internal error in %s, line %u"),
			 __FUNCTION__, __LINE__);
		  /* NOTREACHED */
		}
	    }

	  /* We now have the number of elements.  We build the structure
	     and a helper structure for writing all out.  */
	  len = sizeof (struct locale_file) + elems * sizeof (int);
	  data = obstack_alloc (&obstk, len);
	  iov = obstack_alloc (&obstk, (elems + 1) * sizeof (struct iovec));

	  data->magic = LIMAGIC (cat_no);
	  data->n = elems;
	  iov[0].iov_base = data;
	  iov[0].iov_len = len;

	  cnt = 0;
	  for (item_no = 0; item_no < category[cat_no].number; ++item_no)
	    if (category[cat_no].item_value[item_no] == NULL)
	      {
		switch (category[cat_no].item_desc[item_no].value_type)
		  {
		  case string:
		  case byte:
		  case bytearray:
		    data->idx[cnt] = len;
		    ++len;  /* We reserve one single byte for this entry.  */
		    iov[1 + cnt].iov_base = (char *) "";
		    iov[1 + cnt].iov_len = 1;
		    ++cnt;
		    break;
		  case stringarray:
		    {
		      int max;
		      int nstr;

		      max = category[cat_no].item_desc[item_no].max;

		      for (nstr = 0; nstr < max; ++nstr)
			{
			  data->idx[cnt] = len;
			  ++len;
			  iov[1 + cnt].iov_base = (char *) "";
			  iov[1 + cnt].iov_len = 1;
			  ++cnt;
			}
		    }
		  }
	      }
	    else
	      switch (category[cat_no].item_desc[item_no].value_type)
		{
		case string:
		case bytearray:
		  data->idx[cnt] = len;
		  slen = strlen (category[cat_no].item_value[item_no]) + 1;
		  len += slen;
		  iov[1 + cnt].iov_base = category[cat_no].item_value[item_no];
		  iov[1 + cnt].iov_len = slen;
		  ++cnt;
		  break;
		case byte:
		  data->idx[cnt] = len;
		  slen = 1;
		  len += slen;
		  iov[1 + cnt].iov_base = category[cat_no].item_value[item_no];
		  iov[1 + cnt].iov_len = slen;
		  ++cnt;
		  break;
		case stringarray:
		  {
		    int nstr, nact;
		    char **first;
		  
		    if (category[cat_no].item_desc[item_no].min
			== category[cat_no].item_desc[item_no].max)
		      {
			nstr = category[cat_no].item_desc[item_no].min;
			first = (char **) category[cat_no].item_value[item_no];
		      }
		    else
		      {
			nstr = *(int *) category[cat_no].item_value[item_no];
			first =
			  ((char **) category[cat_no].item_value[item_no]) + 1;
		      }
		    nact = nstr;
		    while (nstr > 0)
		      {
			data->idx[cnt] = len;
			if (*first != NULL)
			  {
			    slen = strlen (*first) + 1;
			    iov[1 + cnt].iov_base = first;
			  }
			else
			  {
			    slen = 1;
			    iov[1 + cnt].iov_base = (char *) "";
			  }
			len += slen;
			iov[1 + cnt].iov_len = slen;
			++cnt;
			++first;
			--nstr;
		      }
		    while (nact < category[cat_no].item_desc[item_no].max)
		      {
			data->idx[cnt] = len;
			len += 1;
			iov[1 + cnt].iov_base = (char *) "";
			iov[1 + cnt].iov_len = 1;
			++cnt;
			++nact;
		      }
		  }
		  break;
		default:
		  /* Cannot happen.  */
		  break;
		}
	  assert (cnt <= elems);

	  /* Construct the output filename from the argument given to
	     localedef on the command line.  */
	  path = (char *) obstack_alloc (&obstk, strlen (output_path) +
					 strlen (category[cat_no].name) + 1);
	  t = stpcpy (path, output_path);
	  strcpy (t, category[cat_no].name);

	  fd = creat (path, 0666);
	  if (fd == -1)
	    {
	      error (0, 0, gettext ("cannot open output file `%s': %m"), path);
	      result = 1;
	    }
	  else
	    {
	      if (writev (fd, iov, cnt + 1) == -1)
		{
		  error (0, 0, gettext ("cannot write output file `%s': %m"),
			 path);
		  result = 1;
		}

if (elems==0) write(fd, &elems, 1);

	      close (fd);
	    }
	  /* The old data is not needed anymore, but keep the obstack
	     intact.  */
	  obstack_free (&obstk, data);
	}

      if (result == 0)
	puts (category[cat_no].name);
    }
  /* Now the whole obstack can be removed.  */
  obstack_free (&obstk, NULL);
}


/* Get the representation of a number.  This is a positive integer or
   the number -1 which is handled as a special symbol by the scanner.  */
static int
get_byte (char *byte_ptr)
{
  int token;
  char *ptr;
  int len;

  token = locfile_lex (&ptr, &len);
  if (token != TOK_NUMBER && token != TOK_MINUS1)
    /* None of the valid number format.  */
    {
      error (0, 0, gettext ("%s:%Zd: number expected"),
	     locfile_data.filename, locfile_data.line_no);
      *byte_ptr = 0;
      return 0;
    }

  if (token == TOK_MINUS1)
    {
      *byte_ptr = CHAR_MAX;
      return 1;
    }

  if (len > CHAR_MAX)
    /* The value of the numbers has to be less than CHAR_MAX.  This is
       ok for the information they have to express.  */
    {
      error (0, 0, gettext ("%s:%Zd: invalid number"),
	     locfile_data.filename, locfile_data.line_no);
      *byte_ptr = 0;
      return 0;
    }

  *byte_ptr = len;
  return 1;
}


/* Test whether the string STR with length LEN is the name of an existing
   locale and whether a file for category CAT_NO is found in this directory.
   This categories are looked for in the system locale definition file
   directory.
   Return the complete file name for the category file.  */
static char *
is_locale_name (int cat_no, const char *str, int len)
{
  static char **locale_names = NULL;
  static int max_count = 0;
  static int locale_count = 0;
  int cnt, exist, fd;
  char *fname;
  struct stat st;

  if (locale_names == NULL)
    /* Read in the list of all available locales.  */
    {
      DIR *dir;
      struct dirent *dirent;
      
      /* LOCALE_NAMES is not NULL anymore, but LOCALE_COUNT == 0.  */
      ++locale_names;
	  
      dir = opendir (LOCALE_PATH);
      if (dir == NULL)
	{
	  error (1, errno, gettext ("cannot read locale directory `%s'"),
		 LOCALE_PATH);

	  return NULL;
	}

      /* Now we can look for all files in the directory.  */
      while ((dirent = readdir (dir)) != NULL)
	if (strcmp (dirent->d_name, ".") != 0
	    && strcmp (dirent->d_name, "..") != 0)
	  {
	    if (max_count == 0)
	      locale_names = (char **) xmalloc ((max_count = 10)
						* sizeof (char *));
	    else
	      locale_names = (char **) xrealloc (locale_names,
						 (max_count *= 2)
						 * sizeof (char *));
	    locale_names[locale_count++] = strdup (dirent->d_name);
	  }
      closedir (dir);
    }

  for (cnt = 0; cnt < locale_count; ++cnt)
    if (strncmp (str, locale_names[cnt], len) == 0
	&& locale_names[cnt][len] == '\0')
      break;
  
  if (cnt >= locale_count)
    return NULL;
  
  /* Now search for this specific locale file.  */
  asprintf (&fname, "%s/%s/%s", LOCALE_PATH, locale_names[cnt],
	    category[cat_no].name);
  
  fd = open (fname, O_RDONLY);
  if (fd < 0)
    {
      free (fname);
      return NULL;
    }
  
  exist = fstat (fd, &st);
  close (fd);
  
  if (exist < 0)
    {
      free (fname);
      return NULL;
    }

  return fname;
}

/*
 * Local Variables:
 *  mode:c
 *  c-basic-offset:2
 * End:
 */
