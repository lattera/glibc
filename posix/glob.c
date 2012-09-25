/* Copyright (C) 1991-2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011
   Free Software Foundation, Inc.
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

#ifdef	HAVE_CONFIG_H
# include <config.h>
#endif

#include <glob.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>

/* Outcomment the following line for production quality code.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include <stdio.h>		/* Needed on stupid SunOS for assert.  */

#if !defined _LIBC || !defined GLOB_ONLY_P
#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
# ifndef POSIX
#  ifdef _POSIX_VERSION
#   define POSIX
#  endif
# endif
#endif

#include <pwd.h>

#if defined HAVE_STDINT_H || defined _LIBC
# include <stdint.h>
#elif !defined UINTPTR_MAX
# define UINTPTR_MAX (~((size_t) 0))
#endif

#include <errno.h>
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#if defined HAVE_DIRENT_H || defined __GNU_LIBRARY__
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
# ifdef HAVE_VMSDIR_H
#  include "vmsdir.h"
# endif /* HAVE_VMSDIR_H */
#endif


/* In GNU systems, <dirent.h> defines this macro for us.  */
#ifdef _D_NAMLEN
# undef NAMLEN
# define NAMLEN(d) _D_NAMLEN(d)
#endif

/* When used in the GNU libc the symbol _DIRENT_HAVE_D_TYPE is available
   if the `d_type' member for `struct dirent' is available.
   HAVE_STRUCT_DIRENT_D_TYPE plays the same role in GNULIB.  */
#if defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE
/* True if the directory entry D must be of type T.  */
# define DIRENT_MUST_BE(d, t)	((d)->d_type == (t))

/* True if the directory entry D might be a symbolic link.  */
# define DIRENT_MIGHT_BE_SYMLINK(d) \
    ((d)->d_type == DT_UNKNOWN || (d)->d_type == DT_LNK)

/* True if the directory entry D might be a directory.  */
# define DIRENT_MIGHT_BE_DIR(d)	 \
    ((d)->d_type == DT_DIR || DIRENT_MIGHT_BE_SYMLINK (d))

#else /* !HAVE_D_TYPE */
# define DIRENT_MUST_BE(d, t)		false
# define DIRENT_MIGHT_BE_SYMLINK(d)	true
# define DIRENT_MIGHT_BE_DIR(d)		true
#endif /* HAVE_D_TYPE */

/* If the system has the `struct dirent64' type we use it internally.  */
#if defined _LIBC && !defined COMPILE_GLOB64
# if defined HAVE_DIRENT_H || defined __GNU_LIBRARY__
#  define CONVERT_D_NAMLEN(d64, d32)
# else
#  define CONVERT_D_NAMLEN(d64, d32) \
  (d64)->d_namlen = (d32)->d_namlen;
# endif

# if (defined POSIX || defined WINDOWS32) && !defined __GNU_LIBRARY__
#  define CONVERT_D_INO(d64, d32)
# else
#  define CONVERT_D_INO(d64, d32) \
  (d64)->d_ino = (d32)->d_ino;
# endif

# ifdef _DIRENT_HAVE_D_TYPE
#  define CONVERT_D_TYPE(d64, d32) \
  (d64)->d_type = (d32)->d_type;
# else
#  define CONVERT_D_TYPE(d64, d32)
# endif

# define CONVERT_DIRENT_DIRENT64(d64, d32) \
  memcpy ((d64)->d_name, (d32)->d_name, NAMLEN (d32) + 1);		      \
  CONVERT_D_NAMLEN (d64, d32)						      \
  CONVERT_D_INO (d64, d32)						      \
  CONVERT_D_TYPE (d64, d32)
#endif


#if (defined POSIX || defined WINDOWS32) && !defined __GNU_LIBRARY__
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
# define REAL_DIR_ENTRY(dp) 1
#else
# define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
#endif /* POSIX */

#include <stdlib.h>
#include <string.h>

/* NAME_MAX is usually defined in <dirent.h> or <limits.h>.  */
#include <limits.h>
#ifndef NAME_MAX
# define NAME_MAX (sizeof (((struct dirent *) 0)->d_name))
#endif

#include <alloca.h>

#ifdef _LIBC
# undef strdup
# define strdup(str) __strdup (str)
# define sysconf(id) __sysconf (id)
# define closedir(dir) __closedir (dir)
# define opendir(name) __opendir (name)
# define readdir(str) __readdir64 (str)
# define getpwnam_r(name, bufp, buf, len, res) \
   __getpwnam_r (name, bufp, buf, len, res)
# ifndef __stat64
#  define __stat64(fname, buf) __xstat64 (_STAT_VER, fname, buf)
# endif
# define struct_stat64		struct stat64
#else /* !_LIBC */
# include "getlogin_r.h"
# include "mempcpy.h"
# include "stat-macros.h"
# include "strdup.h"
# define __stat64(fname, buf)	stat (fname, buf)
# define struct_stat64		struct stat
# define __stat(fname, buf)	stat (fname, buf)
# define __alloca		alloca
# define __readdir		readdir
# define __readdir64		readdir64
# define __glob_pattern_p	glob_pattern_p
#endif /* _LIBC */

#include <fnmatch.h>

#ifdef _SC_GETPW_R_SIZE_MAX
# define GETPW_R_SIZE_MAX()	sysconf (_SC_GETPW_R_SIZE_MAX)
#else
# define GETPW_R_SIZE_MAX()	(-1)
#endif
#ifdef _SC_LOGIN_NAME_MAX
# define GET_LOGIN_NAME_MAX()	sysconf (_SC_LOGIN_NAME_MAX)
#else
# define GET_LOGIN_NAME_MAX()	(-1)
#endif

static const char *next_brace_sub (const char *begin, int flags) __THROW;

#endif /* !defined _LIBC || !defined GLOB_ONLY_P */

#ifndef attribute_hidden
# define attribute_hidden
#endif

static int glob_in_dir (const char *pattern, const char *directory,
			int flags, int (*errfunc) (const char *, int),
			glob_t *pglob, size_t alloca_used);
extern int __glob_pattern_type (const char *pattern, int quote)
    attribute_hidden;

#if !defined _LIBC || !defined GLOB_ONLY_P
static int prefix_array (const char *prefix, char **array, size_t n) __THROW;
static int collated_compare (const void *, const void *) __THROW;


/* Find the end of the sub-pattern in a brace expression.  */
static const char *
next_brace_sub (const char *cp, int flags)
{
  size_t depth = 0;
  while (*cp != '\0')
    if ((flags & GLOB_NOESCAPE) == 0 && *cp == '\\')
      {
	if (*++cp == '\0')
	  break;
	++cp;
      }
    else
      {
	if ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
	  break;

	if (*cp++ == '{')
	  depth++;
      }

  return *cp != '\0' ? cp : NULL;
}

#endif /* !defined _LIBC || !defined GLOB_ONLY_P */

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   `errno' value from the failing call; if it returns non-zero
   `glob' returns GLOB_ABORTED; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, `glob' returns zero.  */
int
#ifdef GLOB_ATTRIBUTE
GLOB_ATTRIBUTE
#endif
glob (pattern, flags, errfunc, pglob)
     const char *pattern;
     int flags;
     int (*errfunc) (const char *, int);
     glob_t *pglob;
{
  const char *filename;
  char *dirname = NULL;
  size_t dirlen;
  int status;
  size_t oldcount;
  int meta;
  int dirname_modified;
  int malloc_dirname = 0;
  glob_t dirs;
  int retval = 0;
#ifdef _LIBC
  size_t alloca_used = 0;
#endif

  if (pattern == NULL || pglob == NULL || (flags & ~__GLOB_FLAGS) != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (!(flags & GLOB_DOOFFS))
    /* Have to do this so `globfree' knows where to start freeing.  It
       also makes all the code that uses gl_offs simpler. */
    pglob->gl_offs = 0;

  if (flags & GLOB_BRACE)
    {
      const char *begin;

      if (flags & GLOB_NOESCAPE)
	begin = strchr (pattern, '{');
      else
	{
	  begin = pattern;
	  while (1)
	    {
	      if (*begin == '\0')
		{
		  begin = NULL;
		  break;
		}

	      if (*begin == '\\' && begin[1] != '\0')
		++begin;
	      else if (*begin == '{')
		break;

	      ++begin;
	    }
	}

      if (begin != NULL)
	{
	  /* Allocate working buffer large enough for our work.  Note that
	    we have at least an opening and closing brace.  */
	  size_t firstc;
	  char *alt_start;
	  const char *p;
	  const char *next;
	  const char *rest;
	  size_t rest_len;
	  char *onealt;
	  size_t pattern_len = strlen (pattern) - 1;
#ifdef _LIBC
	  int alloca_onealt = __libc_use_alloca (alloca_used + pattern_len);
	  if (alloca_onealt)
	    onealt = alloca_account (pattern_len, alloca_used);
	  else
#endif
	    {
	      onealt = (char *) malloc (pattern_len);
	      if (onealt == NULL)
		{
		  if (!(flags & GLOB_APPEND))
		    {
		      pglob->gl_pathc = 0;
		      pglob->gl_pathv = NULL;
		    }
		  return GLOB_NOSPACE;
		}
	    }

	  /* We know the prefix for all sub-patterns.  */
	  alt_start = mempcpy (onealt, pattern, begin - pattern);

	  /* Find the first sub-pattern and at the same time find the
	     rest after the closing brace.  */
	  next = next_brace_sub (begin + 1, flags);
	  if (next == NULL)
	    {
	      /* It is an illegal expression.  */
	    illegal_brace:
#ifdef _LIBC
	      if (__builtin_expect (!alloca_onealt, 0))
#endif
		free (onealt);
	      return glob (pattern, flags & ~GLOB_BRACE, errfunc, pglob);
	    }

	  /* Now find the end of the whole brace expression.  */
	  rest = next;
	  while (*rest != '}')
	    {
	      rest = next_brace_sub (rest + 1, flags);
	      if (rest == NULL)
		/* It is an illegal expression.  */
		goto illegal_brace;
	    }
	  /* Please note that we now can be sure the brace expression
	     is well-formed.  */
	  rest_len = strlen (++rest) + 1;

	  /* We have a brace expression.  BEGIN points to the opening {,
	     NEXT points past the terminator of the first element, and END
	     points past the final }.  We will accumulate result names from
	     recursive runs for each brace alternative in the buffer using
	     GLOB_APPEND.  */

	  if (!(flags & GLOB_APPEND))
	    {
	      /* This call is to set a new vector, so clear out the
		 vector so we can append to it.  */
	      pglob->gl_pathc = 0;
	      pglob->gl_pathv = NULL;
	    }
	  firstc = pglob->gl_pathc;

	  p = begin + 1;
	  while (1)
	    {
	      int result;

	      /* Construct the new glob expression.  */
	      mempcpy (mempcpy (alt_start, p, next - p), rest, rest_len);

	      result = glob (onealt,
			     ((flags & ~(GLOB_NOCHECK | GLOB_NOMAGIC))
			      | GLOB_APPEND), errfunc, pglob);

	      /* If we got an error, return it.  */
	      if (result && result != GLOB_NOMATCH)
		{
#ifdef _LIBC
		  if (__builtin_expect (!alloca_onealt, 0))
#endif
		    free (onealt);
		  if (!(flags & GLOB_APPEND))
		    {
		      globfree (pglob);
		      pglob->gl_pathc = 0;
		    }
		  return result;
		}

	      if (*next == '}')
		/* We saw the last entry.  */
		break;

	      p = next + 1;
	      next = next_brace_sub (p, flags);
	      assert (next != NULL);
	    }

#ifdef _LIBC
	  if (__builtin_expect (!alloca_onealt, 0))
#endif
	    free (onealt);

	  if (pglob->gl_pathc != firstc)
	    /* We found some entries.  */
	    return 0;
	  else if (!(flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
	    return GLOB_NOMATCH;
	}
    }

  if (!(flags & GLOB_APPEND))
    {
      pglob->gl_pathc = 0;
      if (!(flags & GLOB_DOOFFS))
	pglob->gl_pathv = NULL;
      else
	{
	  size_t i;

	  if (pglob->gl_offs >= ~((size_t) 0) / sizeof (char *))
	    return GLOB_NOSPACE;

	  pglob->gl_pathv = (char **) malloc ((pglob->gl_offs + 1)
					      * sizeof (char *));
	  if (pglob->gl_pathv == NULL)
	    return GLOB_NOSPACE;

	  for (i = 0; i <= pglob->gl_offs; ++i)
	    pglob->gl_pathv[i] = NULL;
	}
    }

  oldcount = pglob->gl_pathc + pglob->gl_offs;

  /* Find the filename.  */
  filename = strrchr (pattern, '/');
#if defined __MSDOS__ || defined WINDOWS32
  /* The case of "d:pattern".  Since `:' is not allowed in
     file names, we can safely assume that wherever it
     happens in pattern, it signals the filename part.  This
     is so we could some day support patterns like "[a-z]:foo".  */
  if (filename == NULL)
    filename = strchr (pattern, ':');
#endif /* __MSDOS__ || WINDOWS32 */
  dirname_modified = 0;
  if (filename == NULL)
    {
      /* This can mean two things: a simple name or "~name".  The latter
	 case is nothing but a notation for a directory.  */
      if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && pattern[0] == '~')
	{
	  dirname = (char *) pattern;
	  dirlen = strlen (pattern);

	  /* Set FILENAME to NULL as a special flag.  This is ugly but
	     other solutions would require much more code.  We test for
	     this special case below.  */
	  filename = NULL;
	}
      else
	{
	  if (__builtin_expect (pattern[0] == '\0', 0))
	    {
	      dirs.gl_pathv = NULL;
	      goto no_matches;
	    }

	  filename = pattern;
#ifdef _AMIGA
	  dirname = (char *) "";
#else
	  dirname = (char *) ".";
#endif
	  dirlen = 0;
	}
    }
  else if (filename == pattern
	   || (filename == pattern + 1 && pattern[0] == '\\'
	       && (flags & GLOB_NOESCAPE) == 0))
    {
      /* "/pattern" or "\\/pattern".  */
      dirname = (char *) "/";
      dirlen = 1;
      ++filename;
    }
  else
    {
      char *newp;
      dirlen = filename - pattern;
#if defined __MSDOS__ || defined WINDOWS32
      if (*filename == ':'
	  || (filename > pattern + 1 && filename[-1] == ':'))
	{
	  char *drive_spec;

	  ++dirlen;
	  drive_spec = (char *) __alloca (dirlen + 1);
	  *((char *) mempcpy (drive_spec, pattern, dirlen)) = '\0';
	  /* For now, disallow wildcards in the drive spec, to
	     prevent infinite recursion in glob.  */
	  if (__glob_pattern_p (drive_spec, !(flags & GLOB_NOESCAPE)))
	    return GLOB_NOMATCH;
	  /* If this is "d:pattern", we need to copy `:' to DIRNAME
	     as well.  If it's "d:/pattern", don't remove the slash
	     from "d:/", since "d:" and "d:/" are not the same.*/
	}
#endif
#ifdef _LIBC
      if (__libc_use_alloca (alloca_used + dirlen + 1))
	newp = alloca_account (dirlen + 1, alloca_used);
      else
#endif
	{
	  newp = malloc (dirlen + 1);
	  if (newp == NULL)
	    return GLOB_NOSPACE;
	  malloc_dirname = 1;
	}
      *((char *) mempcpy (newp, pattern, dirlen)) = '\0';
      dirname = newp;
      ++filename;

      if (filename[0] == '\0'
#if defined __MSDOS__ || defined WINDOWS32
	  && dirname[dirlen - 1] != ':'
	  && (dirlen < 3 || dirname[dirlen - 2] != ':'
	      || dirname[dirlen - 1] != '/')
#endif
	  && dirlen > 1)
	/* "pattern/".  Expand "pattern", appending slashes.  */
	{
	  int orig_flags = flags;
	  if (!(flags & GLOB_NOESCAPE) && dirname[dirlen - 1] == '\\')
	    {
	      /* "pattern\\/".  Remove the final backslash if it hasn't
		 been quoted.  */
	      char *p = (char *) &dirname[dirlen - 1];

	      while (p > dirname && p[-1] == '\\') --p;
	      if ((&dirname[dirlen] - p) & 1)
		{
		  *(char *) &dirname[--dirlen] = '\0';
		  flags &= ~(GLOB_NOCHECK | GLOB_NOMAGIC);
		}
	    }
	  int val = glob (dirname, flags | GLOB_MARK, errfunc, pglob);
	  if (val == 0)
	    pglob->gl_flags = ((pglob->gl_flags & ~GLOB_MARK)
			       | (flags & GLOB_MARK));
	  else if (val == GLOB_NOMATCH && flags != orig_flags)
	    {
	      /* Make sure globfree (&dirs); is a nop.  */
	      dirs.gl_pathv = NULL;
	      flags = orig_flags;
	      oldcount = pglob->gl_pathc + pglob->gl_offs;
	      goto no_matches;
	    }
	  retval = val;
	  goto out;
	}
    }

#ifndef VMS
  if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && dirname[0] == '~')
    {
      if (dirname[1] == '\0' || dirname[1] == '/'
	  || (!(flags & GLOB_NOESCAPE) && dirname[1] == '\\'
	      && (dirname[2] == '\0' || dirname[2] == '/')))
	{
	  /* Look up home directory.  */
	  char *home_dir = getenv ("HOME");
	  int malloc_home_dir = 0;
# ifdef _AMIGA
	  if (home_dir == NULL || home_dir[0] == '\0')
	    home_dir = "SYS:";
# else
#  ifdef WINDOWS32
	  if (home_dir == NULL || home_dir[0] == '\0')
	    home_dir = "c:/users/default"; /* poor default */
#  else
	  if (home_dir == NULL || home_dir[0] == '\0')
	    {
	      int success;
	      char *name;
	      size_t buflen = GET_LOGIN_NAME_MAX () + 1;

	      if (buflen == 0)
		/* `sysconf' does not support _SC_LOGIN_NAME_MAX.  Try
		   a moderate value.  */
		buflen = 20;
	      name = alloca_account (buflen, alloca_used);

	      success = getlogin_r (name, buflen) == 0;
	      if (success)
		{
		  struct passwd *p;
#   if defined HAVE_GETPWNAM_R || defined _LIBC
		  long int pwbuflen = GETPW_R_SIZE_MAX ();
		  char *pwtmpbuf;
		  struct passwd pwbuf;
		  int malloc_pwtmpbuf = 0;
		  int save = errno;

#    ifndef _LIBC
		  if (pwbuflen == -1)
		    /* `sysconf' does not support _SC_GETPW_R_SIZE_MAX.
		       Try a moderate value.  */
		    pwbuflen = 1024;
#    endif
		  if (__libc_use_alloca (alloca_used + pwbuflen))
		    pwtmpbuf = alloca_account (pwbuflen, alloca_used);
		  else
		    {
		      pwtmpbuf = malloc (pwbuflen);
		      if (pwtmpbuf == NULL)
			{
			  retval = GLOB_NOSPACE;
			  goto out;
			}
		      malloc_pwtmpbuf = 1;
		    }

		  while (getpwnam_r (name, &pwbuf, pwtmpbuf, pwbuflen, &p)
			 != 0)
		    {
		      if (errno != ERANGE)
			{
			  p = NULL;
			  break;
			}

		      if (!malloc_pwtmpbuf
			  && __libc_use_alloca (alloca_used
						+ 2 * pwbuflen))
			pwtmpbuf = extend_alloca_account (pwtmpbuf, pwbuflen,
							  2 * pwbuflen,
							  alloca_used);
		      else
			{
			  char *newp = realloc (malloc_pwtmpbuf
						? pwtmpbuf : NULL,
						2 * pwbuflen);
			  if (newp == NULL)
			    {
			      if (__builtin_expect (malloc_pwtmpbuf, 0))
				free (pwtmpbuf);
			      retval = GLOB_NOSPACE;
			      goto out;
			    }
			  pwtmpbuf = newp;
			  pwbuflen = 2 * pwbuflen;
			  malloc_pwtmpbuf = 1;
			}
		      __set_errno (save);
		    }
#   else
		  p = getpwnam (name);
#   endif
		  if (p != NULL)
		    {
		      if (!malloc_pwtmpbuf)
			home_dir = p->pw_dir;
		      else
			{
			  size_t home_dir_len = strlen (p->pw_dir) + 1;
			  if (__libc_use_alloca (alloca_used + home_dir_len))
			    home_dir = alloca_account (home_dir_len,
						       alloca_used);
			  else
			    {
			      home_dir = malloc (home_dir_len);
			      if (home_dir == NULL)
				{
				  free (pwtmpbuf);
				  retval = GLOB_NOSPACE;
				  goto out;
				}
			      malloc_home_dir = 1;
			    }
			  memcpy (home_dir, p->pw_dir, home_dir_len);

			  free (pwtmpbuf);
			}
		    }
		}
	    }
	  if (home_dir == NULL || home_dir[0] == '\0')
	    {
	      if (flags & GLOB_TILDE_CHECK)
		{
		  if (__builtin_expect (malloc_home_dir, 0))
		    free (home_dir);
		  retval = GLOB_NOMATCH;
		  goto out;
		}
	      else
		home_dir = (char *) "~"; /* No luck.  */
	    }
#  endif /* WINDOWS32 */
# endif
	  /* Now construct the full directory.  */
	  if (dirname[1] == '\0')
	    {
	      if (__builtin_expect (malloc_dirname, 0))
		free (dirname);

	      dirname = home_dir;
	      dirlen = strlen (dirname);
	      malloc_dirname = malloc_home_dir;
	    }
	  else
	    {
	      char *newp;
	      size_t home_len = strlen (home_dir);
	      int use_alloca = __libc_use_alloca (alloca_used
						  + home_len + dirlen);
	      if (use_alloca)
		newp = alloca_account (home_len + dirlen, alloca_used);
	      else
		{
		  newp = malloc (home_len + dirlen);
		  if (newp == NULL)
		    {
		      if (__builtin_expect (malloc_home_dir, 0))
			free (home_dir);
		      retval = GLOB_NOSPACE;
		      goto out;
		    }
		}

	      mempcpy (mempcpy (newp, home_dir, home_len),
		       &dirname[1], dirlen);

	      if (__builtin_expect (malloc_dirname, 0))
		free (dirname);

	      dirname = newp;
	      dirlen += home_len - 1;
	      malloc_dirname = !use_alloca;
	    }
	  dirname_modified = 1;
	}
# if !defined _AMIGA && !defined WINDOWS32
      else
	{
	  char *end_name = strchr (dirname, '/');
	  char *user_name;
	  int malloc_user_name = 0;
	  char *unescape = NULL;

	  if (!(flags & GLOB_NOESCAPE))
	    {
	      if (end_name == NULL)
		{
		  unescape = strchr (dirname, '\\');
		  if (unescape)
		    end_name = strchr (unescape, '\0');
		}
	      else
		unescape = memchr (dirname, '\\', end_name - dirname);
	    }
	  if (end_name == NULL)
	    user_name = dirname + 1;
	  else
	    {
	      char *newp;
	      if (__libc_use_alloca (alloca_used + (end_name - dirname)))
		newp = alloca_account (end_name - dirname, alloca_used);
	      else
		{
		  newp = malloc (end_name - dirname);
		  if (newp == NULL)
		    {
		      retval = GLOB_NOSPACE;
		      goto out;
		    }
		  malloc_user_name = 1;
		}
	      if (unescape != NULL)
		{
		  char *p = mempcpy (newp, dirname + 1,
				     unescape - dirname - 1);
		  char *q = unescape;
		  while (*q != '\0')
		    {
		      if (*q == '\\')
			{
			  if (q[1] == '\0')
			    {
			      /* "~fo\\o\\" unescape to user_name "foo\\",
				 but "~fo\\o\\/" unescape to user_name
				 "foo".  */
			      if (filename == NULL)
				*p++ = '\\';
			      break;
			    }
			  ++q;
			}
		      *p++ = *q++;
		    }
		  *p = '\0';
		}
	      else
		*((char *) mempcpy (newp, dirname + 1, end_name - dirname))
		  = '\0';
	      user_name = newp;
	    }

	  /* Look up specific user's home directory.  */
	  {
	    struct passwd *p;
#  if defined HAVE_GETPWNAM_R || defined _LIBC
	    long int buflen = GETPW_R_SIZE_MAX ();
	    char *pwtmpbuf;
	    int malloc_pwtmpbuf = 0;
	    struct passwd pwbuf;
	    int save = errno;

#   ifndef _LIBC
	    if (buflen == -1)
	      /* `sysconf' does not support _SC_GETPW_R_SIZE_MAX.  Try a
		 moderate value.  */
	      buflen = 1024;
#   endif
	    if (__libc_use_alloca (alloca_used + buflen))
	      pwtmpbuf = alloca_account (buflen, alloca_used);
	    else
	      {
		pwtmpbuf = malloc (buflen);
		if (pwtmpbuf == NULL)
		  {
		  nomem_getpw:
		    if (__builtin_expect (malloc_user_name, 0))
		      free (user_name);
		    retval = GLOB_NOSPACE;
		    goto out;
		  }
		malloc_pwtmpbuf = 1;
	      }

	    while (getpwnam_r (user_name, &pwbuf, pwtmpbuf, buflen, &p) != 0)
	      {
		if (errno != ERANGE)
		  {
		    p = NULL;
		    break;
		  }
		if (!malloc_pwtmpbuf
		    && __libc_use_alloca (alloca_used + 2 * buflen))
		  pwtmpbuf = extend_alloca_account (pwtmpbuf, buflen,
						    2 * buflen, alloca_used);
		else
		  {
		    char *newp = realloc (malloc_pwtmpbuf ? pwtmpbuf : NULL,
					  2 * buflen);
		    if (newp == NULL)
		      {
			if (__builtin_expect (malloc_pwtmpbuf, 0))
			  free (pwtmpbuf);
			goto nomem_getpw;
		      }
		    pwtmpbuf = newp;
		    malloc_pwtmpbuf = 1;
		  }
		__set_errno (save);
	      }
#  else
	    p = getpwnam (user_name);
#  endif

	    if (__builtin_expect (malloc_user_name, 0))
	      free (user_name);

	    /* If we found a home directory use this.  */
	    if (p != NULL)
	      {
		size_t home_len = strlen (p->pw_dir);
		size_t rest_len = end_name == NULL ? 0 : strlen (end_name);

		if (__builtin_expect (malloc_dirname, 0))
		  free (dirname);
		malloc_dirname = 0;

		if (__libc_use_alloca (alloca_used + home_len + rest_len + 1))
		  dirname = alloca_account (home_len + rest_len + 1,
					    alloca_used);
		else
		  {
		    dirname = malloc (home_len + rest_len + 1);
		    if (dirname == NULL)
		      {
			if (__builtin_expect (malloc_pwtmpbuf, 0))
			  free (pwtmpbuf);
			retval = GLOB_NOSPACE;
			goto out;
		      }
		    malloc_dirname = 1;
		  }
		*((char *) mempcpy (mempcpy (dirname, p->pw_dir, home_len),
				    end_name, rest_len)) = '\0';

		dirlen = home_len + rest_len;
		dirname_modified = 1;

		if (__builtin_expect (malloc_pwtmpbuf, 0))
		  free (pwtmpbuf);
	      }
	    else
	      {
		if (__builtin_expect (malloc_pwtmpbuf, 0))
		  free (pwtmpbuf);

		if (flags & GLOB_TILDE_CHECK)
		  /* We have to regard it as an error if we cannot find the
		     home directory.  */
		  return GLOB_NOMATCH;
	      }
	  }
	}
# endif	/* Not Amiga && not WINDOWS32.  */
    }
#endif	/* Not VMS.  */

  /* Now test whether we looked for "~" or "~NAME".  In this case we
     can give the answer now.  */
  if (filename == NULL)
    {
      struct stat st;
      struct_stat64 st64;

      /* Return the directory if we don't check for error or if it exists.  */
      if ((flags & GLOB_NOCHECK)
	  || (((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0))
	       ? ((*pglob->gl_stat) (dirname, &st) == 0
		  && S_ISDIR (st.st_mode))
	       : (__stat64 (dirname, &st64) == 0 && S_ISDIR (st64.st_mode)))))
	{
	  size_t newcount = pglob->gl_pathc + pglob->gl_offs;
	  char **new_gl_pathv;

	  if (newcount > UINTPTR_MAX - (1 + 1)
	      || newcount + 1 + 1 > ~((size_t) 0) / sizeof (char *))
	    {
	    nospace:
	      free (pglob->gl_pathv);
	      pglob->gl_pathv = NULL;
	      pglob->gl_pathc = 0;
	      return GLOB_NOSPACE;
	    }

	  new_gl_pathv
	    = (char **) realloc (pglob->gl_pathv,
				 (newcount + 1 + 1) * sizeof (char *));
	  if (new_gl_pathv == NULL)
	    goto nospace;
	  pglob->gl_pathv = new_gl_pathv;

	  if (flags & GLOB_MARK)
	    {
	      char *p;
	      pglob->gl_pathv[newcount] = malloc (dirlen + 2);
	      if (pglob->gl_pathv[newcount] == NULL)
		goto nospace;
	      p = mempcpy (pglob->gl_pathv[newcount], dirname, dirlen);
	      p[0] = '/';
	      p[1] = '\0';
	    }
	  else
	    {
	      pglob->gl_pathv[newcount] = strdup (dirname);
	      if (pglob->gl_pathv[newcount] == NULL)
		goto nospace;
	    }
	  pglob->gl_pathv[++newcount] = NULL;
	  ++pglob->gl_pathc;
	  pglob->gl_flags = flags;

	  return 0;
	}

      /* Not found.  */
      return GLOB_NOMATCH;
    }

  meta = __glob_pattern_type (dirname, !(flags & GLOB_NOESCAPE));
  /* meta is 1 if correct glob pattern containing metacharacters.
     If meta has bit (1 << 2) set, it means there was an unterminated
     [ which we handle the same, using fnmatch.  Broken unterminated
     pattern bracket expressions ought to be rare enough that it is
     not worth special casing them, fnmatch will do the right thing.  */
  if (meta & 5)
    {
      /* The directory name contains metacharacters, so we
	 have to glob for the directory, and then glob for
	 the pattern in each directory found.  */
      size_t i;

      if (!(flags & GLOB_NOESCAPE) && dirlen > 0 && dirname[dirlen - 1] == '\\')
	{
	  /* "foo\\/bar".  Remove the final backslash from dirname
	     if it has not been quoted.  */
	  char *p = (char *) &dirname[dirlen - 1];

	  while (p > dirname && p[-1] == '\\') --p;
	  if ((&dirname[dirlen] - p) & 1)
	    *(char *) &dirname[--dirlen] = '\0';
	}

      if (__builtin_expect ((flags & GLOB_ALTDIRFUNC) != 0, 0))
	{
	  /* Use the alternative access functions also in the recursive
	     call.  */
	  dirs.gl_opendir = pglob->gl_opendir;
	  dirs.gl_readdir = pglob->gl_readdir;
	  dirs.gl_closedir = pglob->gl_closedir;
	  dirs.gl_stat = pglob->gl_stat;
	  dirs.gl_lstat = pglob->gl_lstat;
	}

      status = glob (dirname,
		     ((flags & (GLOB_ERR | GLOB_NOESCAPE
				| GLOB_ALTDIRFUNC))
		      | GLOB_NOSORT | GLOB_ONLYDIR),
		     errfunc, &dirs);
      if (status != 0)
	{
	  if ((flags & GLOB_NOCHECK) == 0 || status != GLOB_NOMATCH)
	    return status;
	  goto no_matches;
	}

      /* We have successfully globbed the preceding directory name.
	 For each name we found, call glob_in_dir on it and FILENAME,
	 appending the results to PGLOB.  */
      for (i = 0; i < dirs.gl_pathc; ++i)
	{
	  size_t old_pathc;

#ifdef	SHELL
	  {
	    /* Make globbing interruptible in the bash shell. */
	    extern int interrupt_state;

	    if (interrupt_state)
	      {
		globfree (&dirs);
		return GLOB_ABORTED;
	      }
	  }
#endif /* SHELL.  */

	  old_pathc = pglob->gl_pathc;
	  status = glob_in_dir (filename, dirs.gl_pathv[i],
				((flags | GLOB_APPEND)
				 & ~(GLOB_NOCHECK | GLOB_NOMAGIC)),
				errfunc, pglob, alloca_used);
	  if (status == GLOB_NOMATCH)
	    /* No matches in this directory.  Try the next.  */
	    continue;

	  if (status != 0)
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      return status;
	    }

	  /* Stick the directory on the front of each name.  */
	  if (prefix_array (dirs.gl_pathv[i],
			    &pglob->gl_pathv[old_pathc + pglob->gl_offs],
			    pglob->gl_pathc - old_pathc))
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      return GLOB_NOSPACE;
	    }
	}

      flags |= GLOB_MAGCHAR;

      /* We have ignored the GLOB_NOCHECK flag in the `glob_in_dir' calls.
	 But if we have not found any matching entry and the GLOB_NOCHECK
	 flag was set we must return the input pattern itself.  */
      if (pglob->gl_pathc + pglob->gl_offs == oldcount)
	{
	no_matches:
	  /* No matches.  */
	  if (flags & GLOB_NOCHECK)
	    {
	      size_t newcount = pglob->gl_pathc + pglob->gl_offs;
	      char **new_gl_pathv;

	      if (newcount > UINTPTR_MAX - 2
		  || newcount + 2 > ~((size_t) 0) / sizeof (char *))
		{
		nospace2:
		  globfree (&dirs);
		  return GLOB_NOSPACE;
		}

	      new_gl_pathv = (char **) realloc (pglob->gl_pathv,
						(newcount + 2)
						* sizeof (char *));
	      if (new_gl_pathv == NULL)
		goto nospace2;
	      pglob->gl_pathv = new_gl_pathv;

	      pglob->gl_pathv[newcount] = __strdup (pattern);
	      if (pglob->gl_pathv[newcount] == NULL)
		{
		  globfree (&dirs);
		  globfree (pglob);
		  pglob->gl_pathc = 0;
		  return GLOB_NOSPACE;
		}

	      ++pglob->gl_pathc;
	      ++newcount;

	      pglob->gl_pathv[newcount] = NULL;
	      pglob->gl_flags = flags;
	    }
	  else
	    {
	      globfree (&dirs);
	      return GLOB_NOMATCH;
	    }
	}

      globfree (&dirs);
    }
  else
    {
      size_t old_pathc = pglob->gl_pathc;
      int orig_flags = flags;

      if (meta & 2)
	{
	  char *p = strchr (dirname, '\\'), *q;
	  /* We need to unescape the dirname string.  It is certainly
	     allocated by alloca, as otherwise filename would be NULL
	     or dirname wouldn't contain backslashes.  */
	  q = p;
	  do
	    {
	      if (*p == '\\')
		{
		  *q = *++p;
		  --dirlen;
		}
	      else
		*q = *p;
	      ++q;
	    }
	  while (*p++ != '\0');
	  dirname_modified = 1;
	}
      if (dirname_modified)
	flags &= ~(GLOB_NOCHECK | GLOB_NOMAGIC);
      status = glob_in_dir (filename, dirname, flags, errfunc, pglob,
			    alloca_used);
      if (status != 0)
	{
	  if (status == GLOB_NOMATCH && flags != orig_flags
	      && pglob->gl_pathc + pglob->gl_offs == oldcount)
	    {
	      /* Make sure globfree (&dirs); is a nop.  */
	      dirs.gl_pathv = NULL;
	      flags = orig_flags;
	      goto no_matches;
	    }
	  return status;
	}

      if (dirlen > 0)
	{
	  /* Stick the directory on the front of each name.  */
	  if (prefix_array (dirname,
			    &pglob->gl_pathv[old_pathc + pglob->gl_offs],
			    pglob->gl_pathc - old_pathc))
	    {
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      return GLOB_NOSPACE;
	    }
	}
    }

  if (flags & GLOB_MARK)
    {
      /* Append slashes to directory names.  */
      size_t i;
      struct stat st;
      struct_stat64 st64;

      for (i = oldcount; i < pglob->gl_pathc + pglob->gl_offs; ++i)
	if ((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
	     ? ((*pglob->gl_stat) (pglob->gl_pathv[i], &st) == 0
		&& S_ISDIR (st.st_mode))
	     : (__stat64 (pglob->gl_pathv[i], &st64) == 0
		&& S_ISDIR (st64.st_mode))))
	  {
	    size_t len = strlen (pglob->gl_pathv[i]) + 2;
	    char *new = realloc (pglob->gl_pathv[i], len);
	    if (new == NULL)
	      {
		globfree (pglob);
		pglob->gl_pathc = 0;
		return GLOB_NOSPACE;
	      }
	    strcpy (&new[len - 2], "/");
	    pglob->gl_pathv[i] = new;
	  }
    }

  if (!(flags & GLOB_NOSORT))
    {
      /* Sort the vector.  */
      qsort (&pglob->gl_pathv[oldcount],
	     pglob->gl_pathc + pglob->gl_offs - oldcount,
	     sizeof (char *), collated_compare);
    }

 out:
  if (__builtin_expect (malloc_dirname, 0))
    free (dirname);

  return retval;
}
#if defined _LIBC && !defined glob
libc_hidden_def (glob)
#endif


#if !defined _LIBC || !defined GLOB_ONLY_P

/* Free storage allocated in PGLOB by a previous `glob' call.  */
void
globfree (pglob)
     register glob_t *pglob;
{
  if (pglob->gl_pathv != NULL)
    {
      size_t i;
      for (i = 0; i < pglob->gl_pathc; ++i)
	free (pglob->gl_pathv[pglob->gl_offs + i]);
      free (pglob->gl_pathv);
      pglob->gl_pathv = NULL;
    }
}
#if defined _LIBC && !defined globfree
libc_hidden_def (globfree)
#endif


/* Do a collated comparison of A and B.  */
static int
collated_compare (const void *a, const void *b)
{
  const char *const s1 = *(const char *const * const) a;
  const char *const s2 = *(const char *const * const) b;

  if (s1 == s2)
    return 0;
  if (s1 == NULL)
    return 1;
  if (s2 == NULL)
    return -1;
  return strcoll (s1, s2);
}


/* Prepend DIRNAME to each of N members of ARRAY, replacing ARRAY's
   elements in place.  Return nonzero if out of memory, zero if successful.
   A slash is inserted between DIRNAME and each elt of ARRAY,
   unless DIRNAME is just "/".  Each old element of ARRAY is freed.  */
static int
prefix_array (const char *dirname, char **array, size_t n)
{
  register size_t i;
  size_t dirlen = strlen (dirname);
#if defined __MSDOS__ || defined WINDOWS32
  int sep_char = '/';
# define DIRSEP_CHAR sep_char
#else
# define DIRSEP_CHAR '/'
#endif

  if (dirlen == 1 && dirname[0] == '/')
    /* DIRNAME is just "/", so normal prepending would get us "//foo".
       We want "/foo" instead, so don't prepend any chars from DIRNAME.  */
    dirlen = 0;
#if defined __MSDOS__ || defined WINDOWS32
  else if (dirlen > 1)
    {
      if (dirname[dirlen - 1] == '/' && dirname[dirlen - 2] == ':')
	/* DIRNAME is "d:/".  Don't prepend the slash from DIRNAME.  */
	--dirlen;
      else if (dirname[dirlen - 1] == ':')
	{
	  /* DIRNAME is "d:".  Use `:' instead of `/'.  */
	  --dirlen;
	  sep_char = ':';
	}
    }
#endif

  for (i = 0; i < n; ++i)
    {
      size_t eltlen = strlen (array[i]) + 1;
      char *new = (char *) malloc (dirlen + 1 + eltlen);
      if (new == NULL)
	{
	  while (i > 0)
	    free (array[--i]);
	  return 1;
	}

      {
	char *endp = mempcpy (new, dirname, dirlen);
	*endp++ = DIRSEP_CHAR;
	mempcpy (endp, array[i], eltlen);
      }
      free (array[i]);
      array[i] = new;
    }

  return 0;
}


/* We must not compile this function twice.  */
#if !defined _LIBC || !defined NO_GLOB_PATTERN_P
int
__glob_pattern_type (pattern, quote)
     const char *pattern;
     int quote;
{
  register const char *p;
  int ret = 0;

  for (p = pattern; *p != '\0'; ++p)
    switch (*p)
      {
      case '?':
      case '*':
	return 1;

      case '\\':
	if (quote)
	  {
	    if (p[1] != '\0')
	      ++p;
	    ret |= 2;
	  }
	break;

      case '[':
	ret |= 4;
	break;

      case ']':
	if (ret & 4)
	  return 1;
	break;
      }

  return ret;
}

/* Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.  */
int
__glob_pattern_p (pattern, quote)
     const char *pattern;
     int quote;
{
  return __glob_pattern_type (pattern, quote) == 1;
}
# ifdef _LIBC
weak_alias (__glob_pattern_p, glob_pattern_p)
# endif
#endif

#endif /* !GLOB_ONLY_P */


/* We put this in a separate function mainly to allow the memory
   allocated with alloca to be recycled.  */
#if !defined _LIBC || !defined GLOB_ONLY_P
static int
__attribute_noinline__
link_exists2_p (const char *dir, size_t dirlen, const char *fname,
	       glob_t *pglob
# ifndef _LIBC
		, int flags
# endif
		)
{
  size_t fnamelen = strlen (fname);
  char *fullname = (char *) __alloca (dirlen + 1 + fnamelen + 1);
  struct stat st;
# ifndef _LIBC
  struct_stat64 st64;
# endif

  mempcpy (mempcpy (mempcpy (fullname, dir, dirlen), "/", 1),
	   fname, fnamelen + 1);

# ifdef _LIBC
  return (*pglob->gl_stat) (fullname, &st) == 0;
# else
  return ((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
	   ? (*pglob->gl_stat) (fullname, &st)
	   : __stat64 (fullname, &st64)) == 0);
# endif
}
# ifdef _LIBC
#  define link_exists_p(dfd, dirname, dirnamelen, fname, pglob, flags) \
  (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)			      \
   ? link_exists2_p (dirname, dirnamelen, fname, pglob)			      \
   : ({ struct stat64 st64;						      \
       __fxstatat64 (_STAT_VER, dfd, fname, &st64, 0) == 0; }))
# else
#  define link_exists_p(dfd, dirname, dirnamelen, fname, pglob, flags) \
  link_exists2_p (dirname, dirnamelen, fname, pglob, flags)
# endif
#endif


/* Like `glob', but PATTERN is a final pathname component,
   and matches are searched for in DIRECTORY.
   The GLOB_NOSORT bit in FLAGS is ignored.  No sorting is ever done.
   The GLOB_APPEND flag is assumed to be set (always appends).  */
static int
glob_in_dir (const char *pattern, const char *directory, int flags,
	     int (*errfunc) (const char *, int),
	     glob_t *pglob, size_t alloca_used)
{
  size_t dirlen = strlen (directory);
  void *stream = NULL;
  struct globnames
    {
      struct globnames *next;
      size_t count;
      char *name[64];
    };
#define INITIAL_COUNT sizeof (init_names.name) / sizeof (init_names.name[0])
  struct globnames init_names;
  struct globnames *names = &init_names;
  struct globnames *names_alloca = &init_names;
  size_t nfound = 0;
  size_t cur = 0;
  int meta;
  int save;

  alloca_used += sizeof (init_names);

  init_names.next = NULL;
  init_names.count = INITIAL_COUNT;

  meta = __glob_pattern_type (pattern, !(flags & GLOB_NOESCAPE));
  if (meta == 0 && (flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
    {
      /* We need not do any tests.  The PATTERN contains no meta
	 characters and we must not return an error therefore the
	 result will always contain exactly one name.  */
      flags |= GLOB_NOCHECK;
    }
  else if (meta == 0)
    {
      /* Since we use the normal file functions we can also use stat()
	 to verify the file is there.  */
      union
      {
	struct stat st;
	struct_stat64 st64;
      } ust;
      size_t patlen = strlen (pattern);
      int alloca_fullname = __libc_use_alloca (alloca_used
					       + dirlen + 1 + patlen + 1);
      char *fullname;
      if (alloca_fullname)
	fullname = alloca_account (dirlen + 1 + patlen + 1, alloca_used);
      else
	{
	  fullname = malloc (dirlen + 1 + patlen + 1);
	  if (fullname == NULL)
	    return GLOB_NOSPACE;
	}

      mempcpy (mempcpy (mempcpy (fullname, directory, dirlen),
			"/", 1),
	       pattern, patlen + 1);
      if ((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
	   ? (*pglob->gl_stat) (fullname, &ust.st)
	   : __stat64 (fullname, &ust.st64)) == 0)
	/* We found this file to be existing.  Now tell the rest
	   of the function to copy this name into the result.  */
	flags |= GLOB_NOCHECK;

      if (__builtin_expect (!alloca_fullname, 0))
	free (fullname);
    }
  else
    {
      stream = (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
		? (*pglob->gl_opendir) (directory)
		: opendir (directory));
      if (stream == NULL)
	{
	  if (errno != ENOTDIR
	      && ((errfunc != NULL && (*errfunc) (directory, errno))
		  || (flags & GLOB_ERR)))
	    return GLOB_ABORTED;
	}
      else
	{
#ifdef _LIBC
	  int dfd = (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
		     ? -1 : dirfd ((DIR *) stream));
#endif
	  int fnm_flags = ((!(flags & GLOB_PERIOD) ? FNM_PERIOD : 0)
			   | ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0)
#if defined _AMIGA || defined VMS
			   | FNM_CASEFOLD
#endif
			   );
	  flags |= GLOB_MAGCHAR;

	  while (1)
	    {
	      const char *name;
	      size_t len;
#if defined _LIBC && !defined COMPILE_GLOB64
	      struct dirent64 *d;
	      union
		{
		  struct dirent64 d64;
		  char room [offsetof (struct dirent64, d_name[0])
			     + NAME_MAX + 1];
		}
	      d64buf;

	      if (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0))
		{
		  struct dirent *d32 = (*pglob->gl_readdir) (stream);
		  if (d32 != NULL)
		    {
		      CONVERT_DIRENT_DIRENT64 (&d64buf.d64, d32);
		      d = &d64buf.d64;
		    }
		  else
		    d = NULL;
		}
	      else
		d = __readdir64 (stream);
#else
	      struct dirent *d = (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
				  ? ((struct dirent *)
				     (*pglob->gl_readdir) (stream))
				  : __readdir (stream));
#endif
	      if (d == NULL)
		break;
	      if (! REAL_DIR_ENTRY (d))
		continue;

	      /* If we shall match only directories use the information
		 provided by the dirent call if possible.  */
	      if ((flags & GLOB_ONLYDIR) && !DIRENT_MIGHT_BE_DIR (d))
		continue;

	      name = d->d_name;

	      if (fnmatch (pattern, name, fnm_flags) == 0)
		{
		  /* If the file we found is a symlink we have to
		     make sure the target file exists.  */
		  if (!DIRENT_MIGHT_BE_SYMLINK (d)
		      || link_exists_p (dfd, directory, dirlen, name, pglob,
					flags))
		    {
		      if (cur == names->count)
			{
			  struct globnames *newnames;
			  size_t count = names->count * 2;
			  size_t size = (sizeof (struct globnames)
					 + ((count - INITIAL_COUNT)
					    * sizeof (char *)));
			  if (__libc_use_alloca (alloca_used + size))
			    newnames = names_alloca
			      = alloca_account (size, alloca_used);
			  else if ((newnames = malloc (size))
				   == NULL)
			    goto memory_error;
			  newnames->count = count;
			  newnames->next = names;
			  names = newnames;
			  cur = 0;
			}
		      len = NAMLEN (d);
		      names->name[cur] = (char *) malloc (len + 1);
		      if (names->name[cur] == NULL)
			goto memory_error;
		      *((char *) mempcpy (names->name[cur++], name, len))
			= '\0';
		      ++nfound;
		    }
		}
	    }
	}
    }

  if (nfound == 0 && (flags & GLOB_NOCHECK))
    {
      size_t len = strlen (pattern);
      nfound = 1;
      names->name[cur] = (char *) malloc (len + 1);
      if (names->name[cur] == NULL)
	goto memory_error;
      *((char *) mempcpy (names->name[cur++], pattern, len)) = '\0';
    }

  int result = GLOB_NOMATCH;
  if (nfound != 0)
    {
      result = 0;

      if (pglob->gl_pathc > UINTPTR_MAX - pglob->gl_offs
	  || pglob->gl_pathc + pglob->gl_offs > UINTPTR_MAX - nfound
	  || pglob->gl_pathc + pglob->gl_offs + nfound > UINTPTR_MAX - 1
	  || (pglob->gl_pathc + pglob->gl_offs + nfound + 1
	      > UINTPTR_MAX / sizeof (char *)))
	goto memory_error;

      char **new_gl_pathv;
      new_gl_pathv
	= (char **) realloc (pglob->gl_pathv,
			     (pglob->gl_pathc + pglob->gl_offs + nfound + 1)
			     * sizeof (char *));
      if (new_gl_pathv == NULL)
	{
	memory_error:
	  while (1)
	    {
	      struct globnames *old = names;
	      for (size_t i = 0; i < cur; ++i)
		free (names->name[i]);
	      names = names->next;
	      /* NB: we will not leak memory here if we exit without
		 freeing the current block assigned to OLD.  At least
		 the very first block is always allocated on the stack
		 and this is the block assigned to OLD here.  */
	      if (names == NULL)
		{
		  assert (old == &init_names);
		  break;
		}
	      cur = names->count;
	      if (old == names_alloca)
		names_alloca = names;
	      else
		free (old);
	    }
	  result = GLOB_NOSPACE;
	}
      else
	{
	  while (1)
	    {
	      struct globnames *old = names;
	      for (size_t i = 0; i < cur; ++i)
		new_gl_pathv[pglob->gl_offs + pglob->gl_pathc++]
		  = names->name[i];
	      names = names->next;
	      /* NB: we will not leak memory here if we exit without
		 freeing the current block assigned to OLD.  At least
		 the very first block is always allocated on the stack
		 and this is the block assigned to OLD here.  */
	      if (names == NULL)
		{
		  assert (old == &init_names);
		  break;
		}
	      cur = names->count;
	      if (old == names_alloca)
		names_alloca = names;
	      else
		free (old);
	    }

	  pglob->gl_pathv = new_gl_pathv;

	  pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

	  pglob->gl_flags = flags;
	}
    }

  if (stream != NULL)
    {
      save = errno;
      if (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0))
	(*pglob->gl_closedir) (stream);
      else
	closedir (stream);
      __set_errno (save);
    }

  return result;
}
