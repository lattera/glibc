/* locale - Implementation of the locale program according to POSIX 1003.2
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <dirent.h>
#include <error.h>
#include <getopt.h>
#include <langinfo.h>
#include <libintl.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "localeinfo.h"


/* If set print the name of the category.  */
static int show_category_name;

/* If set print the name of the item.  */
static int show_keyword_name;

/* Long options.  */
static const struct option long_options[] =
{
  { "all-locales", no_argument, NULL, 'a' },
  { "category-name", no_argument, &show_category_name, 1 },
  { "charmaps", no_argument, NULL, 'm' },
  { "help", no_argument, NULL, 'h' },
  { "keyword-name", no_argument, &show_keyword_name, 1 },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};


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

/* Definition of the data structure which represents a category and its
   items.  */
struct category
{
  int cat_id;
  const char *name;
  size_t number;
  struct cat_item
  {
    int item_id;
    const char *name;
    enum { std, opt } status;
    enum value_type value_type;
    int min;
    int max;
  } *item_desc;
};

/* Simple helper macro.  */
#define NELEMS(arr) ((sizeof (arr)) / (sizeof (arr[0])))

/* For some tricky stuff.  */
#define NO_PAREN(Item, More...) Item, ## More

/* We have all categories defined in `categories.def'.  Now construct
   the description and data structure used for all categories.  */
#define DEFINE_ELEMENT(Item, More...) { Item, ## More },
#define DEFINE_CATEGORY(category, name, items, postload, in, check, out)      \
    static struct cat_item category##_desc[] =				      \
      {									      \
        NO_PAREN items							      \
      };

#include "categories.def"
#undef DEFINE_CATEGORY

static struct category category[] =
  {
#define DEFINE_CATEGORY(category, name, items, postload, in, check, out)      \
    [category] = { _NL_NUM_##category, name, NELEMS (category##_desc),	      \
		   category##_desc },
#include "categories.def"
#undef DEFINE_CATEGORY
  };
#define NCATEGORIES NELEMS (category)


/* Automatically set variable.  */
extern const char *__progname;

/* helper function for extended name handling.  */
extern void locale_special (const char *name, int show_category_name,
			    int show_keyword_name);

/* Prototypes for local functions.  */
static void usage (int status) __attribute__ ((noreturn));
static void write_locales (void);
static void write_charmaps (void);
static void show_locale_vars (void);
static void show_info (const char *name);


int
main (int argc, char *argv[])
{
  int optchar;
  int do_all = 0;
  int do_help = 0;
  int do_version = 0;
  int do_charmaps = 0;

  /* Set initial values for global variables.  */
  show_category_name = 0;
  show_keyword_name = 0;

  /* Set locale.  Do not set LC_ALL because the other categories must
     not be affected (acccording to POSIX.2).  */
  setlocale (LC_CTYPE, "");
  setlocale (LC_MESSAGES, "");

  /* Initialize the message catalog.  */
  textdomain (PACKAGE);

  while ((optchar = getopt_long (argc, argv, "achkmV", long_options, NULL))
         != EOF)
    switch (optchar)
      {
      case '\0':		/* Long option.  */
	break;
      case 'a':
	do_all = 1;
	break;
      case 'c':
	show_category_name = 1;
	break;
      case 'h':
	do_help = 1;
	break;
      case 'k':
	show_keyword_name = 1;
	break;
      case 'm':
	do_charmaps = 1;
	break;
      case 'V':
	do_version = 1;
	break;
      default:
	usage (EXIT_FAILURE);
      }

  /* Version information is requested.  */
  if (do_version)
    {
      fprintf (stderr, "%s - GNU %s %s\n", __progname, "libc", VERSION);
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* `-a' requests the names of all available locales.  */
  if (do_all != 0)
    {
      write_locales ();
      exit (EXIT_SUCCESS);
    }

  /* `m' requests the names of all available charmaps.  The names can be
     used for the -f argument to localedef(3).  */
  if (do_charmaps != 0)
    {
      write_charmaps ();
      exit (EXIT_SUCCESS);
    }

  /* Specific information about the current locale are requested.
     Change to this locale now.  */
  setlocale (LC_ALL, "");

  /* If no real argument is given we have to print the contents of the
     current locale definition variables.  These are LANG and the LC_*.  */
  if (optind == argc && show_keyword_name == 0 && show_category_name == 0)
    {
      show_locale_vars ();
      exit (EXIT_SUCCESS);
    }

  /* Process all given names.  */
  while (optind <  argc)
    show_info (argv[optind++]);

  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, gettext ("Try `%s --help' for more information.\n"),
	     __progname);
  else
    printf (gettext ("\
Usage: %s [OPTION]... name\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -h, --help            display this help and exit\n\
  -V, --version         output version information and exit\n\
\n\
  -a, --all-locales     write names of available locales\n\
  -m, --charmaps        write names of available charmaps\n\
\n\
  -c, --category-name   write names of selected categories\n\
  -k, --keyword-name    write names of selected keywords\n"),
	    __progname);

  exit (status);
}


/* Write the names of all available locales to stdout.  */
static void
write_locales (void)
{
  DIR *dir;
  struct dirent *dirent;

  /* `POSIX' locale is always available (POSIX.2 4.34.3).  */
  puts ("POSIX");

  dir = opendir (LOCALE_PATH);
  if (dir == NULL)
    {
      error (1, errno, gettext ("cannot read locale directory `%s'"),
	     LOCALE_PATH);
      return;
    }

  /* Now we can look for all files in the directory.  */
  while ((dirent = readdir (dir)) != NULL)
    if (strcmp (dirent->d_name, ".") != 0
	&& strcmp (dirent->d_name, "..") != 0)
      puts (dirent->d_name);

  closedir (dir);
}


/* Write the names of all available character maps to stdout.  */
static void
write_charmaps (void)
{
  DIR *dir;
  struct dirent *dirent;

  dir = opendir (CHARMAP_PATH);
  if (dir == NULL)
    {
      error (1, errno, gettext ("cannot read character map directory `%s'"),
	     CHARMAP_PATH);
      return;
    }

  /* Now we can look for all files in the directory.  */
  while ((dirent = readdir (dir)) != NULL)
    if (strcmp (dirent->d_name, ".") != 0
	&& strcmp (dirent->d_name, "..") != 0)
      puts (dirent->d_name);

  closedir (dir);
}


/* We have to show the contents of the environments determining the
   locale.  */
static void
show_locale_vars (void)
{
  size_t cat_no;
  const char *lcall = getenv ("LC_ALL");
  const char *lang = getenv ("LANG") ? : "POSIX";

  void get_source (const char *name)
    {
      char *val = getenv (name);

      if (lcall != NULL || val == NULL)
	printf ("%s=\"%s\"\n", name, lcall ? : lang);
      else
	printf ("%s=%s\n", name, val);
    }

  /* LANG has to be the first value.  */
  printf ("LANG=%s\n", lang);

  /* Now all categories in an unspecified order.  */
  for (cat_no = 0; cat_no < NCATEGORIES; ++cat_no)
    get_source (category[cat_no].name);

  /* The last is the LC_ALL value.  */
  printf ("LC_ALL=%s\n", lcall ? : "");
}


/* Show the information request for NAME.  */
static void
show_info (const char *name)
{
  size_t cat_no;

  void print_item (struct cat_item *item)
    {
      if (show_keyword_name != 0)
	printf ("%s=", item->name);

      switch (item->value_type)
	{
	case string:
	  printf ("%s%s%s", show_keyword_name ? "\"" : "",
		  nl_langinfo (item->item_id) ? : "",
		  show_keyword_name ? "\"" : "");
	  break;
	case stringarray:
	  {
	    int cnt;
	    const char *val;

	    if (show_keyword_name)
	      putchar ('"');

	    for (cnt = 0; cnt < item->max - 1; ++cnt)
	      {
		val = nl_langinfo (item->item_id + cnt);
		printf ("%s;", val ? : "");
	      }

	    val = nl_langinfo (item->item_id + cnt);
	    printf ("%s", val ? : "");

	    if (show_keyword_name)
	      putchar ('"');
	  }
	  break;
	case byte:
	  {
	    const char *val = nl_langinfo (item->item_id);

	    if (val != NULL)
	      printf ("%d", *val == CHAR_MAX ? -1 : *val);
	  }
	  break;
	case bytearray:
	  {
	    const char *val = nl_langinfo (item->item_id);
	    int cnt = val ? strlen (val) : 0;

	    while (cnt > 1)
	      {
		printf ("%d;", *val == CHAR_MAX ? -1 : *val);
                --cnt;
		++val;
	      }

	    printf ("%d", cnt == 0 || *val == CHAR_MAX ? -1 : *val);
	  }
	  break;
	case word:
	  {
	    unsigned int val = (unsigned int) nl_langinfo (item->item_id);
	    printf ("%d", val);
	  }
	  break;
	default:
	}
      putchar ('\n');
    }

  for (cat_no = 0; cat_no < NCATEGORIES; ++cat_no)
    {
      size_t item_no;

      if (strcmp (name, category[cat_no].name) == 0)
	/* Print the whole category.  */
	{
	  if (show_category_name != 0)
	    puts (category[cat_no].name);

	  for (item_no = 0; item_no < category[cat_no].number; ++item_no)
	    print_item (&category[cat_no].item_desc[item_no]);

	  return;
	}

      for (item_no = 0; item_no < category[cat_no].number; ++item_no)
	if (strcmp (name, category[cat_no].item_desc[item_no].name) == 0)
	  {
	    if (show_category_name != 0)
	      puts (category[cat_no].name);

	    print_item (&category[cat_no].item_desc[item_no]);
	    return;
	  }
    }

  /* When we get to here the name is not standard ones.  For testing
     and perhpas advanced use we allow some more symbols.  */
  locale_special (name, show_category_name, show_keyword_name);
}
