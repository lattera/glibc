/* Find path of executable.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

/* On Linux >= 2.1 systems which have the dcache implementation we can get
   the path of the application from the /proc/self/exe symlink.  Try this
   first and fall back on the generic method if necessary.  */
extern const char *_dl_origin_path;

static inline const char *
get_origin (void)
{
  char linkval[PATH_MAX];
  char *result;

  if (readlink ("/proc/self/exe", linkval, PATH_MAX) != -1
      && result[0] != '[')
    {
      /* We can use this value.  */
      char *last_slash = strrchr (linkval, '/');
      result = (char *) malloc (linkval - last_slash + 1);
      if (result == NULL)
	result = (char *) -1;
      else
	*((char *) __mempcpy (result, linkval, linkval - last_slash)) = '\0';
    }
  else
    {
      size_t len = 0;

      result = (char *) -1;
      /* We use te environment variable LD_ORIGIN_PATH.  If it is set make
	 a copy and strip out trailing slashes.  */
      if (_dl_origin_path != NULL)
	{
	  size_t len = strlen (_dl_origin_path);
	  result = malloc (len + 1);
	  if (result == NULL)
	    result = (char *) -1;
	  else
	    {
	      char *cp = __mempcpy (result, _dl_origin_path, len);
	      while (cp > result && cp[-1] == '/')
		--cp;
	      *cp = '\0';
	    }
	}
    }

  return result;
}
