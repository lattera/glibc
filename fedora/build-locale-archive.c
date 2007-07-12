#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdarg.h>
#include "../locale/hashval.h"

const char *alias_file = DATADIR "/locale/locale.alias";
const char *locar_file = PREFIX "/lib/locale/locale-archive";
const char *loc_path = PREFIX "/lib/locale/";
int be_quiet = 1;
int verbose = 0;
int max_locarchive_open_retry = 10;
const char *output_prefix;

static int
is_prime (unsigned long candidate)
{
  /* No even number and none less than 10 will be passed here.  */
  unsigned long int divn = 3;
  unsigned long int sq = divn * divn;

  while (sq < candidate && candidate % divn != 0)
    {
      ++divn;
      sq += 4 * divn;
      ++divn;
    }

  return candidate % divn != 0;
}

unsigned long
next_prime (unsigned long seed)
{
  /* Make it definitely odd.  */
  seed |= 1;

  while (!is_prime (seed))
    seed += 2;

  return seed;
}

/* xmalloc is only used by show_archive_content.  */
void *
xmalloc (size_t size)
{
  exit (255);
}

void
error (int status, int errnum, const char *message, ...)
{
  va_list args;

  va_start (args, message);
  fflush (stdout);
  fprintf (stderr, "%s: ", program_invocation_name);
  vfprintf (stderr, message, args);
  va_end (args);
  if (errnum)
    fprintf (stderr, ": %s", strerror (errnum));
  putc ('\n', stderr);
  fflush (stderr);
  if (status)
    exit (errnum == EROFS ? 0 : status);
}

extern int add_locales_to_archive (size_t nlist, char *list[], bool replace);

int main ()
{
  char path[4096];
  DIR *dirp;
  struct dirent64 *d;
  struct stat64 st;
  char *list[16384], *primary;
  unsigned int cnt = 0;

  unlink (locar_file);
  dirp = opendir (loc_path);
  if (dirp == NULL)
    error (EXIT_FAILURE, errno, "cannot open directory \"%s\"", loc_path);

  primary = getenv ("LC_ALL");
  if (primary == NULL)
    primary = getenv ("LANG");
  if (primary != NULL)
    {
      if (strncmp (primary, "ja", 2) != 0
	  && strncmp (primary, "ko", 2) != 0
	  && strncmp (primary, "zh", 2) != 0)
	{
	  char *ptr = malloc (strlen (primary) + strlen (".utf8") + 1), *p, *q;

	  if (ptr)
	    {
	      p = ptr;
	      q = primary;
	      while (*q && *q != '.' && *q != '@')
		*p++ = *q++;
	      if (*q == '.')
		while (*q && *q != '@')
		  q++;
	      p = stpcpy (p, ".utf8");
	      strcpy (p, q);
	      primary = ptr;
	    }
	  else
	    primary = ".....";
	}
      strcpy (stpcpy (path, loc_path), primary);
      if (stat64 (path, &st) >= 0 && S_ISDIR (st.st_mode))
	{
          list[cnt] = strdup (path);
          if (list[cnt] == NULL)
	    error (0, errno, "cannot add file to list \"%s\"", path);
	  else
	    cnt++;
	}
      if (cnt == 0)
	primary = NULL;
    }

  while ((d = readdir64 (dirp)) != NULL)
    {
      if (strcmp (d->d_name, ".") == 0 || strcmp (d->d_name, "..") == 0)
	continue;

      if (primary && strcmp (d->d_name, primary) == 0)
	continue;

      strcpy (stpcpy (path, loc_path), d->d_name);
      if (stat64 (path, &st) < 0)
	{
	  error (0, errno, "cannot stat \"%s\"", path);
	  continue;
	}
      if (! S_ISDIR (st.st_mode))
	continue;
      if (cnt == 16384)
	error (EXIT_FAILURE, 0, "too many directories in \"%s\"", loc_path);
      list[cnt] = strdup (path);
      if (list[cnt] == NULL)
	{
	  error (0, errno, "cannot add file to list \"%s\"", path);
	  continue;
	}
      cnt++;
    }
  closedir (dirp);
  add_locales_to_archive (cnt, list, 0);
  char *argv[] = { "/usr/sbin/tzdata-update", NULL };
  execve (argv[0], (char *const *)argv, (char *const *)&argv[1]);
  exit (0);
}
