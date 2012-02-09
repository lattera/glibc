/* Copyright (C) 1999-2011, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1999.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#define PROCINFO_CLASS static
#include <alloca.h>
#include <argp.h>
#include <dirent.h>
#include <elf.h>
#include <error.h>
#include <errno.h>
#include <inttypes.h>
#include <libintl.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glob.h>
#include <libgen.h>

#include <ldconfig.h>
#include <dl-cache.h>

#include <dl-procinfo.h>

#ifdef _DL_FIRST_PLATFORM
# define _DL_FIRST_EXTRA (_DL_FIRST_PLATFORM + _DL_PLATFORMS_COUNT)
#else
# define _DL_FIRST_EXTRA _DL_HWCAP_COUNT
#endif

#ifndef LD_SO_CONF
# define LD_SO_CONF SYSCONFDIR "/ld.so.conf"
#endif

/* Get libc version number.  */
#include <version.h>

#define PACKAGE _libc_intl_domainname

static const struct
{
  const char *name;
  int flag;
} lib_types[] =
{
  {"libc4", FLAG_LIBC4},
  {"libc5", FLAG_ELF_LIBC5},
  {"libc6", FLAG_ELF_LIBC6},
  {"glibc2", FLAG_ELF_LIBC6}
};


/* List of directories to handle.  */
struct dir_entry
{
  char *path;
  int flag;
  ino64_t ino;
  dev_t dev;
  struct dir_entry *next;
};

/* The list is unsorted, contains no duplicates.  Entries are added at
   the end.  */
static struct dir_entry *dir_entries;

/* Flags for different options.  */
/* Print Cache.  */
static int opt_print_cache;

/* Be verbose.  */
int opt_verbose;

/* Format to support.  */
/* 0: only libc5/glibc2; 1: both; 2: only glibc 2.2.  */
int opt_format = 1;

/* Build cache.  */
static int opt_build_cache = 1;

/* Generate links.  */
static int opt_link = 1;

/* Only process directories specified on the command line.  */
static int opt_only_cline;

/* Path to root for chroot.  */
static char *opt_chroot;

/* Manually link given shared libraries.  */
static int opt_manual_link;

/* Should we ignore an old auxiliary cache file?  */
static int opt_ignore_aux_cache;

/* Cache file to use.  */
static char *cache_file;

/* Configuration file.  */
static const char *config_file;

/* Mask to use for important hardware capabilities.  */
static unsigned long int hwcap_mask = HWCAP_IMPORTANT;

/* Configuration-defined capabilities defined in kernel vDSOs.  */
static const char *hwcap_extra[64 - _DL_FIRST_EXTRA];

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *)
     = print_version;

/* Function to print some extra text in the help message.  */
static char *more_help (int key, const char *text, void *input);

/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { "print-cache", 'p', NULL, 0, N_("Print cache"), 0},
  { "verbose", 'v', NULL, 0, N_("Generate verbose messages"), 0},
  { NULL, 'N', NULL, 0, N_("Don't build cache"), 0},
  { NULL, 'X', NULL, 0, N_("Don't generate links"), 0},
  { NULL, 'r', N_("ROOT"), 0, N_("Change to and use ROOT as root directory"), 0},
  { NULL, 'C', N_("CACHE"), 0, N_("Use CACHE as cache file"), 0},
  { NULL, 'f', N_("CONF"), 0, N_("Use CONF as configuration file"), 0},
  { NULL, 'n', NULL, 0, N_("Only process directories specified on the command line.  Don't build cache."), 0},
  { NULL, 'l', NULL, 0, N_("Manually link individual libraries."), 0},
  { "format", 'c', N_("FORMAT"), 0, N_("Format to use: new, old or compat (default)"), 0},
  { "ignore-aux-cache", 'i', NULL, 0, N_("Ignore auxiliary cache file"), 0},
  { NULL, 0, NULL, 0, NULL, 0 }
};

#define PROCINFO_CLASS static
#include <dl-procinfo.c>

/* Short description of program.  */
static const char doc[] = N_("Configure Dynamic Linker Run Time Bindings.");

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, NULL, doc, NULL, more_help, NULL
};

/* Check if string corresponds to an important hardware capability or
   a platform.  */
static int
is_hwcap_platform (const char *name)
{
  int hwcap_idx = _dl_string_hwcap (name);

  if (hwcap_idx != -1 && ((1 << hwcap_idx) & hwcap_mask))
    return 1;

  hwcap_idx = _dl_string_platform (name);
  if (hwcap_idx != -1)
    return 1;

  for (hwcap_idx = _DL_FIRST_EXTRA; hwcap_idx < 64; ++hwcap_idx)
    if (hwcap_extra[hwcap_idx - _DL_FIRST_EXTRA] != NULL
	&& !strcmp (name, hwcap_extra[hwcap_idx - _DL_FIRST_EXTRA]))
      return 1;

  return 0;
}

/* Get hwcap (including platform) encoding of path.  */
static uint64_t
path_hwcap (const char *path)
{
  char *str = xstrdup (path);
  char *ptr;
  uint64_t hwcap = 0;
  uint64_t h;

  size_t len;

  len = strlen (str);
  if (str[len] == '/')
    str[len] = '\0';

  /* Search pathname from the end and check for hwcap strings.  */
  for (;;)
    {
      ptr = strrchr (str, '/');

      if (ptr == NULL)
	break;

      h = _dl_string_hwcap (ptr + 1);

      if (h == (uint64_t) -1)
	{
	  h = _dl_string_platform (ptr + 1);
	  if (h == (uint64_t) -1)
	    {
	      for (h = _DL_FIRST_EXTRA; h < 64; ++h)
		if (hwcap_extra[h - _DL_FIRST_EXTRA] != NULL
		    && !strcmp (ptr + 1, hwcap_extra[h - _DL_FIRST_EXTRA]))
		  break;
	      if (h == 64)
		break;
	    }
	}
      hwcap += 1ULL << h;

      /* Search the next part of the path.  */
      *ptr = '\0';
    }

  free (str);
  return hwcap;
}

/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'C':
      cache_file = arg;
      /* Ignore auxiliary cache since we use non-standard cache.  */
      opt_ignore_aux_cache = 1;
      break;
    case 'f':
      config_file = arg;
      break;
    case 'i':
      opt_ignore_aux_cache = 1;
      break;
    case 'l':
      opt_manual_link = 1;
      break;
    case 'N':
      opt_build_cache = 0;
      break;
    case 'n':
      opt_build_cache = 0;
      opt_only_cline = 1;
      break;
    case 'p':
      opt_print_cache = 1;
      break;
    case 'r':
      opt_chroot = arg;
      break;
    case 'v':
      opt_verbose = 1;
      break;
    case 'X':
      opt_link = 0;
      break;
    case 'c':
      if (strcmp (arg, "old") == 0)
	opt_format = 0;
      else if (strcmp (arg, "compat") == 0)
	opt_format = 1;
      else if (strcmp (arg, "new") == 0)
	opt_format = 2;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

/* Print bug-reporting information in the help message.  */
static char *
more_help (int key, const char *text, void *input)
{
  switch (key)
    {
    case ARGP_KEY_HELP_EXTRA:
      /* We print some extra information.  */
      return strdup (gettext ("\
For bug reporting instructions, please see:\n\
<http://www.gnu.org/software/libc/bugs.html>.\n"));
    default:
      break;
    }
  return (char *) text;
}

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "ldconfig (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "2012");
  fprintf (stream, gettext ("Written by %s.\n"),
	   "Andreas Jaeger");
}

/* Add a single directory entry.  */
static void
add_single_dir (struct dir_entry *entry, int verbose)
{
  struct dir_entry *ptr, *prev;

  ptr = dir_entries;
  prev = ptr;
  while (ptr != NULL)
    {
      /* Check for duplicates.  */
      if (ptr->ino == entry->ino && ptr->dev == entry->dev)
	{
	  if (opt_verbose && verbose)
	    error (0, 0, _("Path `%s' given more than once"), entry->path);
	  /* Use the newer information.  */
	  ptr->flag = entry->flag;
	  free (entry->path);
	  free (entry);
	  break;
	}
      prev = ptr;
      ptr = ptr->next;
    }
  /* Is this the first entry?  */
  if (ptr == NULL && dir_entries == NULL)
    dir_entries = entry;
  else if (ptr == NULL)
    prev->next = entry;
}

/* Add one directory to the list of directories to process.  */
static void
add_dir (const char *line)
{
  unsigned int i;
  struct dir_entry *entry = xmalloc (sizeof (struct dir_entry));
  entry->next = NULL;

  /* Search for an '=' sign.  */
  entry->path = xstrdup (line);
  char *equal_sign = strchr (entry->path, '=');
  if (equal_sign)
    {
      *equal_sign = '\0';
      ++equal_sign;
      entry->flag = FLAG_ANY;
      for (i = 0; i < sizeof (lib_types) / sizeof (lib_types[0]); ++i)
	if (strcmp (equal_sign, lib_types[i].name) == 0)
	  {
	    entry->flag = lib_types[i].flag;
	    break;
	  }
      if (entry->flag == FLAG_ANY)
	error (0, 0, _("%s is not a known library type"), equal_sign);
    }
  else
    {
      entry->flag = FLAG_ANY;
    }

  /* Canonify path: for now only remove leading and trailing
     whitespace and the trailing slashes.  */
  i = strlen (entry->path);

  while (i > 0 && isspace (entry->path[i - 1]))
    entry->path[--i] = '\0';

  while (i > 0 && entry->path[i - 1] == '/')
    entry->path[--i] = '\0';

  if (i == 0)
    return;

  char *path = entry->path;
  if (opt_chroot)
    path = chroot_canon (opt_chroot, path);

  struct stat64 stat_buf;
  if (path == NULL || stat64 (path, &stat_buf))
    {
      if (opt_verbose)
	error (0, errno, _("Can't stat %s"), entry->path);
      free (entry->path);
      free (entry);
    }
  else
    {
      entry->ino = stat_buf.st_ino;
      entry->dev = stat_buf.st_dev;

      add_single_dir (entry, 1);
    }

  if (opt_chroot)
    free (path);
}


static int
chroot_stat (const char *real_path, const char *path, struct stat64 *st)
{
  int ret;
  char *canon_path;

  if (!opt_chroot)
    return stat64 (real_path, st);

  ret = lstat64 (real_path, st);
  if (ret || !S_ISLNK (st->st_mode))
    return ret;

  canon_path = chroot_canon (opt_chroot, path);
  if (canon_path == NULL)
    return -1;

  ret = stat64 (canon_path, st);
  free (canon_path);
  return ret;
}

/* Create a symbolic link from soname to libname in directory path.  */
static void
create_links (const char *real_path, const char *path, const char *libname,
	      const char *soname)
{
  char *full_libname, *full_soname;
  char *real_full_libname, *real_full_soname;
  struct stat64 stat_lib, stat_so, lstat_so;
  int do_link = 1;
  int do_remove = 1;
  /* XXX: The logics in this function should be simplified.  */

  /* Get complete path.  */
  full_libname = alloca (strlen (path) + strlen (libname) + 2);
  full_soname = alloca (strlen (path) + strlen (soname) + 2);
  sprintf (full_libname, "%s/%s", path, libname);
  sprintf (full_soname, "%s/%s", path, soname);
  if (opt_chroot)
    {
      real_full_libname = alloca (strlen (real_path) + strlen (libname) + 2);
      real_full_soname = alloca (strlen (real_path) + strlen (soname) + 2);
      sprintf (real_full_libname, "%s/%s", real_path, libname);
      sprintf (real_full_soname, "%s/%s", real_path, soname);
    }
  else
    {
      real_full_libname = full_libname;
      real_full_soname = full_soname;
    }

  /* Does soname already exist and point to the right library?  */
  if (chroot_stat (real_full_soname, full_soname, &stat_so) == 0)
    {
      if (chroot_stat (real_full_libname, full_libname, &stat_lib))
	{
	  error (0, 0, _("Can't stat %s\n"), full_libname);
	  return;
	}
      if (stat_lib.st_dev == stat_so.st_dev
	  && stat_lib.st_ino == stat_so.st_ino)
	/* Link is already correct.  */
	do_link = 0;
      else if (lstat64 (full_soname, &lstat_so) == 0
	       && !S_ISLNK (lstat_so.st_mode))
	{
	  error (0, 0, _("%s is not a symbolic link\n"), full_soname);
	  do_link = 0;
	  do_remove = 0;
	}
    }
  else if (lstat64 (real_full_soname, &lstat_so) != 0
	   || !S_ISLNK (lstat_so.st_mode))
    /* Unless it is a stale symlink, there is no need to remove.  */
    do_remove = 0;

  if (opt_verbose)
    printf ("\t%s -> %s", soname, libname);

  if (do_link && opt_link)
    {
      /* Remove old link.  */
      if (do_remove)
	if (unlink (real_full_soname))
	  {
	    error (0, 0, _("Can't unlink %s"), full_soname);
	    do_link = 0;
	  }
      /* Create symbolic link.  */
      if (do_link && symlink (libname, real_full_soname))
	{
	  error (0, 0, _("Can't link %s to %s"), full_soname, libname);
	  do_link = 0;
	}
      if (opt_verbose)
	{
	  if (do_link)
	    fputs (_(" (changed)\n"), stdout);
	  else
	    fputs (_(" (SKIPPED)\n"), stdout);
	}
    }
  else if (opt_verbose)
    fputs ("\n", stdout);
}

/* Manually link the given library.  */
static void
manual_link (char *library)
{
  char *path;
  char *real_path;
  char *real_library;
  char *libname;
  char *soname;
  struct stat64 stat_buf;
  int flag;
  unsigned int osversion;

  /* Prepare arguments for create_links call.  Split library name in
     directory and filename first.  Since path is allocated, we've got
     to be careful to free at the end.  */
  path = xstrdup (library);
  libname = strrchr (path, '/');

  if (libname)
    {
      /* Successfully split names.  Check if path is just "/" to avoid
	 an empty path.  */
      if (libname == path)
	{
	  libname = library + 1;
	  path = xrealloc (path, 2);
	  strcpy (path, "/");
	}
      else
	{
	  *libname = '\0';
	  ++libname;
	}
    }
  else
    {
      /* There's no path, construct one. */
      libname = library;
      path = xrealloc (path, 2);
      strcpy (path, ".");
    }

  if (opt_chroot)
    {
      real_path = chroot_canon (opt_chroot, path);
      if (real_path == NULL)
	{
	  error (0, errno, _("Can't find %s"), path);
	  free (path);
	  return;
	}
      real_library = alloca (strlen (real_path) + strlen (libname) + 2);
      sprintf (real_library, "%s/%s", real_path, libname);
    }
  else
    {
      real_path = path;
      real_library = library;
    }

  /* Do some sanity checks first.  */
  if (lstat64 (real_library, &stat_buf))
    {
      error (0, errno, _("Cannot lstat %s"), library);
      free (path);
      return;
    }
  /* We don't want links here!  */
  else if (!S_ISREG (stat_buf.st_mode))
    {
      error (0, 0, _("Ignored file %s since it is not a regular file."),
	     library);
      free (path);
      return;
    }

  if (process_file (real_library, library, libname, &flag, &osversion,
		    &soname, 0, &stat_buf))
    {
      error (0, 0, _("No link created since soname could not be found for %s"),
	     library);
      free (path);
      return;
    }
  if (soname == NULL)
    soname = implicit_soname (libname, flag);
  create_links (real_path, path, libname, soname);
  free (soname);
  free (path);
}


/* Read a whole directory and search for libraries.
   The purpose is two-fold:
   - search for libraries which will be added to the cache
   - create symbolic links to the soname for each library

   This has to be done separatly for each directory.

   To keep track of which libraries to add to the cache and which
   links to create, we save a list of all libraries.

   The algorithm is basically:
   for all libraries in the directory do
     get soname of library
     if soname is already in list
       if new library is newer, replace entry
       otherwise ignore this library
     otherwise add library to list

   For example, if the two libraries libxy.so.1.1 and libxy.so.1.2
   exist and both have the same soname, e.g. libxy.so, a symbolic link
   is created from libxy.so.1.2 (the newer one) to libxy.so.
   libxy.so.1.2 and libxy.so are added to the cache - but not
   libxy.so.1.1.  */

/* Information for one library.  */
struct dlib_entry
{
  char *name;
  char *soname;
  int flag;
  int is_link;
  unsigned int osversion;
  struct dlib_entry *next;
};


static void
search_dir (const struct dir_entry *entry)
{
  uint64_t hwcap = path_hwcap (entry->path);
  if (opt_verbose)
    {
      if (hwcap != 0)
	printf ("%s: (hwcap: %#.16" PRIx64 ")\n", entry->path, hwcap);
      else
	printf ("%s:\n", entry->path);
    }

  char *dir_name;
  char *real_file_name;
  size_t real_file_name_len;
  size_t file_name_len = PATH_MAX;
  char *file_name = alloca (file_name_len);
  if (opt_chroot)
    {
      dir_name = chroot_canon (opt_chroot, entry->path);
      real_file_name_len = PATH_MAX;
      real_file_name = alloca (real_file_name_len);
    }
  else
    {
      dir_name = entry->path;
      real_file_name_len = 0;
      real_file_name = file_name;
    }

  DIR *dir;
  if (dir_name == NULL || (dir = opendir (dir_name)) == NULL)
    {
      if (opt_verbose)
	error (0, errno, _("Can't open directory %s"), entry->path);
      if (opt_chroot && dir_name)
	free (dir_name);
      return;
    }

  struct dirent64 *direntry;
  struct dlib_entry *dlibs = NULL;
  while ((direntry = readdir64 (dir)) != NULL)
    {
      int flag;
#ifdef _DIRENT_HAVE_D_TYPE
      /* We only look at links and regular files.  */
      if (direntry->d_type != DT_UNKNOWN
	  && direntry->d_type != DT_LNK
	  && direntry->d_type != DT_REG
	  && direntry->d_type != DT_DIR)
	continue;
#endif /* _DIRENT_HAVE_D_TYPE  */
      /* Does this file look like a shared library or is it a hwcap
	 subdirectory?  The dynamic linker is also considered as
	 shared library.  */
      if (((strncmp (direntry->d_name, "lib", 3) != 0
	    && strncmp (direntry->d_name, "ld-", 3) != 0)
	   || strstr (direntry->d_name, ".so") == NULL)
	  && (
#ifdef _DIRENT_HAVE_D_TYPE
	      direntry->d_type == DT_REG ||
#endif
	      !is_hwcap_platform (direntry->d_name)))
	continue;

      size_t len = strlen (direntry->d_name);
      /* Skip temporary files created by the prelink program.  Files with
	 names like these are never really DSOs we want to look at.  */
      if (len >= sizeof (".#prelink#") - 1)
	{
	  if (strcmp (direntry->d_name + len - sizeof (".#prelink#") + 1,
		      ".#prelink#") == 0)
	    continue;
	  if (len >= sizeof (".#prelink#.XXXXXX") - 1
	      && memcmp (direntry->d_name + len - sizeof (".#prelink#.XXXXXX")
			 + 1, ".#prelink#.", sizeof (".#prelink#.") - 1) == 0)
	    continue;
	}
      len += strlen (entry->path) + 2;
      if (len > file_name_len)
	{
	  file_name_len = len;
	  file_name = alloca (file_name_len);
	  if (!opt_chroot)
	    real_file_name = file_name;
	}
      sprintf (file_name, "%s/%s", entry->path, direntry->d_name);
      if (opt_chroot)
	{
	  len = strlen (dir_name) + strlen (direntry->d_name) + 2;
	  if (len > real_file_name_len)
	    {
	      real_file_name_len = len;
	      real_file_name = alloca (real_file_name_len);
	    }
	  sprintf (real_file_name, "%s/%s", dir_name, direntry->d_name);
	}

      struct stat64 lstat_buf;
#ifdef _DIRENT_HAVE_D_TYPE
      /* We optimize and try to do the lstat call only if needed.  */
      if (direntry->d_type != DT_UNKNOWN)
	lstat_buf.st_mode = DTTOIF (direntry->d_type);
      else
#endif
	if (__builtin_expect (lstat64 (real_file_name, &lstat_buf), 0))
	  {
	    error (0, errno, _("Cannot lstat %s"), file_name);
	    continue;
	  }

      struct stat64 stat_buf;
      int is_dir;
      int is_link = S_ISLNK (lstat_buf.st_mode);
      if (is_link)
	{
	  /* In case of symlink, we check if the symlink refers to
	     a directory. */
	  char *target_name = real_file_name;
	  if (opt_chroot)
	    {
	      target_name = chroot_canon (opt_chroot, file_name);
	      if (target_name == NULL)
		{
		  if (strstr (file_name, ".so") == NULL)
		    error (0, 0, _("Input file %s not found.\n"), file_name);
		  continue;
		}
	    }
	  if (__builtin_expect (stat64 (target_name, &stat_buf), 0))
	    {
	      if (opt_verbose)
		error (0, errno, _("Cannot stat %s"), file_name);

	      /* Remove stale symlinks.  */
	      if (strstr (direntry->d_name, ".so."))
		unlink (real_file_name);
	      continue;
	    }
	  is_dir = S_ISDIR (stat_buf.st_mode);

	  /* lstat_buf is later stored, update contents.  */
	  lstat_buf.st_dev = stat_buf.st_dev;
	  lstat_buf.st_ino = stat_buf.st_ino;
	  lstat_buf.st_size = stat_buf.st_size;
	  lstat_buf.st_ctime = stat_buf.st_ctime;
	}
      else
	is_dir = S_ISDIR (lstat_buf.st_mode);

      if (is_dir && is_hwcap_platform (direntry->d_name))
	{
	  /* Handle subdirectory later.  */
	  struct dir_entry *new_entry;

	  new_entry = xmalloc (sizeof (struct dir_entry));
	  new_entry->path = xstrdup (file_name);
	  new_entry->flag = entry->flag;
	  new_entry->next = NULL;
#ifdef _DIRENT_HAVE_D_TYPE
	  /* We have filled in lstat only #ifndef
	     _DIRENT_HAVE_D_TYPE.  Fill it in if needed.  */
	  if (!is_link
	      && direntry->d_type != DT_UNKNOWN
	      && __builtin_expect (lstat64 (real_file_name, &lstat_buf), 0))
	    {
	      error (0, errno, _("Cannot lstat %s"), file_name);
	      free (new_entry->path);
	      free (new_entry);
	      continue;
	    }
#endif
	  new_entry->ino = lstat_buf.st_ino;
	  new_entry->dev = lstat_buf.st_dev;
	  add_single_dir (new_entry, 0);
	  continue;
	}
      else if (!S_ISREG (lstat_buf.st_mode) && !is_link)
	continue;

      char *real_name;
      if (opt_chroot && is_link)
	{
	  real_name = chroot_canon (opt_chroot, file_name);
	  if (real_name == NULL)
	    {
	      if (strstr (file_name, ".so") == NULL)
		error (0, 0, _("Input file %s not found.\n"), file_name);
	      continue;
	    }
	}
      else
	real_name = real_file_name;

#ifdef _DIRENT_HAVE_D_TYPE
      /* Call lstat64 if not done yet.  */
      if (!is_link
	  && direntry->d_type != DT_UNKNOWN
	  && __builtin_expect (lstat64 (real_file_name, &lstat_buf), 0))
	{
	  error (0, errno, _("Cannot lstat %s"), file_name);
	  continue;
	}
#endif

      /* First search whether the auxiliary cache contains this
	 library already and it's not changed.  */
      char *soname;
      unsigned int osversion;
      if (!search_aux_cache (&lstat_buf, &flag, &osversion, &soname))
	{
	  if (process_file (real_name, file_name, direntry->d_name, &flag,
			    &osversion, &soname, is_link, &lstat_buf))
	    {
	      if (real_name != real_file_name)
		free (real_name);
	      continue;
	    }
	  else if (opt_build_cache)
	    add_to_aux_cache (&lstat_buf, flag, osversion, soname);
	}

      if (soname == NULL)
	soname = implicit_soname (direntry->d_name, flag);

      /* A link may just point to itself.  */
      if (is_link)
	{
	  /* If the path the link points to isn't its soname and it is not
	     .so symlink for ld(1) only, we treat it as a normal file.  */
	  const char *real_base_name = basename (real_file_name);

	  if (strcmp (real_base_name, soname) != 0)
	    {
	      len = strlen (real_base_name);
	      if (len < strlen (".so")
		  || strcmp (real_base_name + len - strlen (".so"), ".so") != 0
		  || strncmp (real_base_name, soname, len) != 0)
		is_link = 0;
	    }
	}

      if (real_name != real_file_name)
	free (real_name);

      if (is_link)
	{
	  free (soname);
	  soname = xstrdup (direntry->d_name);
	}

      if (flag == FLAG_ELF
	  && (entry->flag == FLAG_ELF_LIBC5
	      || entry->flag == FLAG_ELF_LIBC6))
	flag = entry->flag;

      /* Some sanity checks to print warnings.  */
      if (opt_verbose)
	{
	  if (flag == FLAG_ELF_LIBC5 && entry->flag != FLAG_ELF_LIBC5
	      && entry->flag != FLAG_ANY)
	    error (0, 0, _("libc5 library %s in wrong directory"), file_name);
	  if (flag == FLAG_ELF_LIBC6 && entry->flag != FLAG_ELF_LIBC6
	      && entry->flag != FLAG_ANY)
	    error (0, 0, _("libc6 library %s in wrong directory"), file_name);
	  if (flag == FLAG_LIBC4 && entry->flag != FLAG_LIBC4
	      && entry->flag != FLAG_ANY)
	    error (0, 0, _("libc4 library %s in wrong directory"), file_name);
	}

      /* Add library to list.  */
      struct dlib_entry *dlib_ptr;
      for (dlib_ptr = dlibs; dlib_ptr != NULL; dlib_ptr = dlib_ptr->next)
	{
	  /* Is soname already in list?  */
	  if (strcmp (dlib_ptr->soname, soname) == 0)
	    {
	      /* Prefer a file to a link, otherwise check which one
		 is newer.  */
	      if ((!is_link && dlib_ptr->is_link)
		  || (is_link == dlib_ptr->is_link
		      && _dl_cache_libcmp (dlib_ptr->name, direntry->d_name) < 0))
		{
		  /* It's newer - add it.  */
		  /* Flag should be the same - sanity check.  */
		  if (dlib_ptr->flag != flag)
		    {
		      if (dlib_ptr->flag == FLAG_ELF
			  && (flag == FLAG_ELF_LIBC5 || flag == FLAG_ELF_LIBC6))
			dlib_ptr->flag = flag;
		      else if ((dlib_ptr->flag == FLAG_ELF_LIBC5
				|| dlib_ptr->flag == FLAG_ELF_LIBC6)
			       && flag == FLAG_ELF)
			dlib_ptr->flag = flag;
		      else
			error (0, 0, _("libraries %s and %s in directory %s have same soname but different type."),
			       dlib_ptr->name, direntry->d_name,
			       entry->path);
		    }
		  free (dlib_ptr->name);
		  dlib_ptr->name = xstrdup (direntry->d_name);
		  dlib_ptr->is_link = is_link;
		  dlib_ptr->osversion = osversion;
		}
	      /* Don't add this library, abort loop.  */
	      /* Also free soname, since it's dynamically allocated.  */
	      free (soname);
	      break;
	    }
	}
      /* Add the library if it's not already in.  */
      if (dlib_ptr == NULL)
	{
	  dlib_ptr = (struct dlib_entry *)xmalloc (sizeof (struct dlib_entry));
	  dlib_ptr->name = xstrdup (direntry->d_name);
	  dlib_ptr->soname = soname;
	  dlib_ptr->flag = flag;
	  dlib_ptr->is_link = is_link;
	  dlib_ptr->osversion = osversion;
	  /* Add at head of list.  */
	  dlib_ptr->next = dlibs;
	  dlibs = dlib_ptr;
	}
    }

  closedir (dir);

  /* Now dlibs contains a list of all libs - add those to the cache
     and created all symbolic links.  */
  struct dlib_entry *dlib_ptr;
  for (dlib_ptr = dlibs; dlib_ptr != NULL; dlib_ptr = dlib_ptr->next)
    {
      /* Don't create links to links.  */
      if (dlib_ptr->is_link == 0)
	create_links (dir_name, entry->path, dlib_ptr->name,
		      dlib_ptr->soname);
      if (opt_build_cache)
	add_to_cache (entry->path, dlib_ptr->soname, dlib_ptr->flag,
		      dlib_ptr->osversion, hwcap);
    }

  /* Free all resources.  */
  while (dlibs)
    {
      dlib_ptr = dlibs;
      free (dlib_ptr->soname);
      free (dlib_ptr->name);
      dlibs = dlibs->next;
      free (dlib_ptr);
    }

  if (opt_chroot && dir_name)
    free (dir_name);
}

/* Search through all libraries.  */
static void
search_dirs (void)
{
  struct dir_entry *entry;

  for (entry = dir_entries; entry != NULL; entry = entry->next)
    search_dir (entry);

  /* Free all allocated memory.  */
  while (dir_entries)
    {
      entry = dir_entries;
      dir_entries = dir_entries->next;
      free (entry->path);
      free (entry);
    }
}


static void parse_conf_include (const char *config_file, unsigned int lineno,
				bool do_chroot, const char *pattern);

/* Parse configuration file.  */
static void
parse_conf (const char *filename, bool do_chroot)
{
  FILE *file = NULL;
  char *line = NULL;
  const char *canon;
  size_t len = 0;
  unsigned int lineno;

  if (do_chroot && opt_chroot)
    {
      canon = chroot_canon (opt_chroot, filename);
      if (canon)
	file = fopen (canon, "r");
      else
	canon = filename;
    }
  else
    {
      canon = filename;
      file = fopen (filename, "r");
    }

  if (file == NULL)
    {
      error (0, errno, _("Can't open configuration file %s"), canon);
      if (canon != filename)
	free ((char *) canon);
      return;
    }

  /* No threads use this stream.  */
  __fsetlocking (file, FSETLOCKING_BYCALLER);

  if (canon != filename)
    free ((char *) canon);

  lineno = 0;
  do
    {
      ssize_t n = getline (&line, &len, file);
      if (n < 0)
	break;

      ++lineno;
      if (line[n - 1] == '\n')
	line[n - 1] = '\0';

      /* Because the file format does not know any form of quoting we
	 can search forward for the next '#' character and if found
	 make it terminating the line.  */
      *strchrnul (line, '#') = '\0';

      /* Remove leading whitespace.  NUL is no whitespace character.  */
      char *cp = line;
      while (isspace (*cp))
	++cp;

      /* If the line is blank it is ignored.  */
      if (cp[0] == '\0')
	continue;

      if (!strncmp (cp, "include", 7) && isblank (cp[7]))
	{
	  char *dir;
	  cp += 8;
	  while ((dir = strsep (&cp, " \t")) != NULL)
	    if (dir[0] != '\0')
	      parse_conf_include (filename, lineno, do_chroot, dir);
	}
      else if (!strncasecmp (cp, "hwcap", 5) && isblank (cp[5]))
	{
	  cp += 6;
	  char *p, *name = NULL;
	  unsigned long int n = strtoul (cp, &cp, 0);
	  if (cp != NULL && isblank (*cp))
	    while ((p = strsep (&cp, " \t")) != NULL)
	      if (p[0] != '\0')
		{
		  if (name == NULL)
		    name = p;
		  else
		    {
		      name = NULL;
		      break;
		    }
		}
	  if (name == NULL)
	    {
	      error (EXIT_FAILURE, 0, _("%s:%u: bad syntax in hwcap line"),
		     filename, lineno);
	      break;
	    }
	  if (n >= (64 - _DL_FIRST_EXTRA))
	    error (EXIT_FAILURE, 0,
		   _("%s:%u: hwcap index %lu above maximum %u"),
		   filename, lineno, n, 64 - _DL_FIRST_EXTRA - 1);
	  if (hwcap_extra[n] == NULL)
	    {
	      for (unsigned long int h = 0; h < (64 - _DL_FIRST_EXTRA); ++h)
		if (hwcap_extra[h] != NULL && !strcmp (name, hwcap_extra[h]))
		  error (EXIT_FAILURE, 0,
			 _("%s:%u: hwcap index %lu already defined as %s"),
			 filename, lineno, h, name);
	      hwcap_extra[n] = xstrdup (name);
	    }
	  else
	    {
	      if (strcmp (name, hwcap_extra[n]))
		error (EXIT_FAILURE, 0,
		       _("%s:%u: hwcap index %lu already defined as %s"),
		       filename, lineno, n, hwcap_extra[n]);
	      if (opt_verbose)
		error (0, 0, _("%s:%u: duplicate hwcap %lu %s"),
		       filename, lineno, n, name);
	    }
	}
      else
	add_dir (cp);
    }
  while (!feof_unlocked (file));

  /* Free buffer and close file.  */
  free (line);
  fclose (file);
}

/* Handle one word in an `include' line, a glob pattern of additional
   config files to read.  */
static void
parse_conf_include (const char *config_file, unsigned int lineno,
		    bool do_chroot, const char *pattern)
{
  if (opt_chroot && pattern[0] != '/')
    error (EXIT_FAILURE, 0,
	   _("need absolute file name for configuration file when using -r"));

  char *copy = NULL;
  if (pattern[0] != '/' && strchr (config_file, '/') != NULL)
    {
      if (asprintf (&copy, "%s/%s", dirname (strdupa (config_file)),
		    pattern) < 0)
	error (EXIT_FAILURE, 0, _("memory exhausted"));
      pattern = copy;
    }

  glob64_t gl;
  int result;
  if (do_chroot && opt_chroot)
    {
      char *canon = chroot_canon (opt_chroot, pattern);
      if (canon == NULL)
	return;
      result = glob64 (canon, 0, NULL, &gl);
      free (canon);
    }
  else
    result = glob64 (pattern, 0, NULL, &gl);

  switch (result)
    {
    case 0:
      for (size_t i = 0; i < gl.gl_pathc; ++i)
	parse_conf (gl.gl_pathv[i], false);
      globfree64 (&gl);
      break;

    case GLOB_NOMATCH:
      break;

    case GLOB_NOSPACE:
      errno = ENOMEM;
    case GLOB_ABORTED:
      if (opt_verbose)
	error (0, errno, _("%s:%u: cannot read directory %s"),
	       config_file, lineno, pattern);
      break;

    default:
      abort ();
      break;
    }

  free (copy);
}

/* Honour LD_HWCAP_MASK.  */
static void
set_hwcap (void)
{
  char *mask = getenv ("LD_HWCAP_MASK");

  if (mask)
    hwcap_mask = strtoul (mask, NULL, 0);
}


int
main (int argc, char **argv)
{
  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  textdomain (_libc_intl_domainname);

  /* Parse and process arguments.  */
  int remaining;
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  /* Remaining arguments are additional directories if opt_manual_link
     is not set.  */
  if (remaining != argc && !opt_manual_link)
    {
      int i;
      for (i = remaining; i < argc; ++i)
	if (opt_build_cache && argv[i][0] != '/')
	  error (EXIT_FAILURE, 0,
		 _("relative path `%s' used to build cache"),
		 argv[i]);
	else
	  add_dir (argv[i]);
    }

  hwcap_extra[63 - _DL_FIRST_EXTRA] = "tls";

  set_hwcap ();

  if (opt_chroot)
    {
      /* Normalize the path a bit, we might need it for printing later.  */
      char *endp = rawmemchr (opt_chroot, '\0');
      while (endp > opt_chroot && endp[-1] == '/')
	--endp;
      *endp = '\0';
      if (endp == opt_chroot)
	opt_chroot = NULL;

      if (opt_chroot)
	{
	  /* It is faster to use chroot if we can.  */
	  if (!chroot (opt_chroot))
	    {
	      if (chdir ("/"))
		error (EXIT_FAILURE, errno, _("Can't chdir to /"));
	      opt_chroot = NULL;
	    }
	}
    }

  if (cache_file == NULL)
    {
      cache_file = alloca (strlen (LD_SO_CACHE) + 1);
      strcpy (cache_file, LD_SO_CACHE);
    }

  if (config_file == NULL)
    config_file = LD_SO_CONF;

  if (opt_print_cache)
    {
      if (opt_chroot)
	{
	  char *p = chroot_canon (opt_chroot, cache_file);
	  if (p == NULL)
	    error (EXIT_FAILURE, errno, _("Can't open cache file %s\n"),
		   cache_file);
	  cache_file = p;
	}
      print_cache (cache_file);
      if (opt_chroot)
	free (cache_file);
      exit (0);
    }

  if (opt_chroot)
    {
      /* Canonicalize the directory name of cache_file, not cache_file,
	 because we'll rename a temporary cache file to it.  */
      char *p = strrchr (cache_file, '/');
      char *canon = chroot_canon (opt_chroot,
				  p ? (*p = '\0', cache_file) : "/");

      if (canon == NULL)
	error (EXIT_FAILURE, errno,
	       _("Can't open cache file directory %s\n"),
	       p ? cache_file : "/");

      if (p)
	++p;
      else
	p = cache_file;

      cache_file = alloca (strlen (canon) + strlen (p) + 2);
      sprintf (cache_file, "%s/%s", canon, p);
      free (canon);
    }

  if (opt_manual_link)
    {
      /* Link all given libraries manually.  */
      int i;

      for (i = remaining; i < argc; ++i)
	manual_link (argv[i]);

      exit (0);
    }


  if (opt_build_cache)
    init_cache ();

  if (!opt_only_cline)
    {
      parse_conf (config_file, true);

      /* Always add the standard search paths.  */
      add_system_dir (SLIBDIR);
      if (strcmp (SLIBDIR, LIBDIR))
	add_system_dir (LIBDIR);
    }

  const char *aux_cache_file = _PATH_LDCONFIG_AUX_CACHE;
  if (opt_chroot)
    aux_cache_file = chroot_canon (opt_chroot, aux_cache_file);

  if (! opt_ignore_aux_cache && aux_cache_file)
    load_aux_cache (aux_cache_file);
  else
    init_aux_cache ();

  search_dirs ();

  if (opt_build_cache)
    {
      save_cache (cache_file);
      if (aux_cache_file)
	save_aux_cache (aux_cache_file);
    }

  return 0;
}
