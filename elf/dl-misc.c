/* Miscellaneous support functions for dynamic linker
   Copyright (C) 1997-2002, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <fcntl.h>
#include <ldsodefs.h>
#include <limits.h>
#include <link.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sysdep.h>
#include <stdio-common/_itoa.h>
#include <bits/libc-lock.h>

#ifndef MAP_ANON
/* This is the only dl-sysdep.c function that is actually needed at run-time
   by _dl_map_object.  */

int
_dl_sysdep_open_zero_fill (void)
{
  return __open ("/dev/zero", O_RDONLY);
}
#endif

/* Read the whole contents of FILE into new mmap'd space with given
   protections.  *SIZEP gets the size of the file.  On error MAP_FAILED
   is returned.  */

void *
internal_function
_dl_sysdep_read_whole_file (const char *file, size_t *sizep, int prot)
{
  void *result = MAP_FAILED;
  struct stat64 st;
  int fd = __open (file, O_RDONLY);
  if (fd >= 0)
    {
      if (__fxstat64 (_STAT_VER, fd, &st) >= 0)
	{
	  *sizep = st.st_size;

	  /* No need to map the file if it is empty.  */
	  if (*sizep != 0)
	    /* Map a copy of the file contents.  */
	    result = __mmap (NULL, *sizep, prot,
#ifdef MAP_COPY
			     MAP_COPY
#else
			     MAP_PRIVATE
#endif
#ifdef MAP_FILE
			     | MAP_FILE
#endif
			     , fd, 0);
	}
      __close (fd);
    }
  return result;
}


/* Bare-bones printf implementation.  This function only knows about
   the formats and flags needed and can handle only up to 64 stripes in
   the output.  */
static void
_dl_debug_vdprintf (int fd, int tag_p, const char *fmt, va_list arg)
{
# define NIOVMAX 64
  struct iovec iov[NIOVMAX];
  int niov = 0;
  pid_t pid = 0;
  char pidbuf[12];

  while (*fmt != '\0')
    {
      const char *startp = fmt;

      if (tag_p > 0)
	{
	  /* Generate the tag line once.  It consists of the PID and a
	     colon followed by a tab.  */
	  if (pid == 0)
	    {
	      char *p;
	      pid = __getpid ();
	      assert (pid >= 0 && sizeof (pid_t) <= 4);
	      p = _itoa (pid, &pidbuf[10], 10, 0);
	      while (p > pidbuf)
		*--p = ' ';
	      pidbuf[10] = ':';
	      pidbuf[11] = '\t';
	    }

	  /* Append to the output.  */
	  assert (niov < NIOVMAX);
	  iov[niov].iov_len = 12;
	  iov[niov++].iov_base = pidbuf;

	  /* No more tags until we see the next newline.  */
	  tag_p = -1;
	}

      /* Skip everything except % and \n (if tags are needed).  */
      while (*fmt != '\0' && *fmt != '%' && (! tag_p || *fmt != '\n'))
	++fmt;

      /* Append constant string.  */
      assert (niov < NIOVMAX);
      if ((iov[niov].iov_len = fmt - startp) != 0)
	iov[niov++].iov_base = (char *) startp;

      if (*fmt == '%')
	{
	  /* It is a format specifier.  */
	  char fill = ' ';
	  int width = -1;
	  int prec = -1;
#if LONG_MAX != INT_MAX
	  int long_mod = 0;
#endif

	  /* Recognize zero-digit fill flag.  */
	  if (*++fmt == '0')
	    {
	      fill = '0';
	      ++fmt;
	    }

	  /* See whether with comes from a parameter.  Note that no other
	     way to specify the width is implemented.  */
	  if (*fmt == '*')
	    {
	      width = va_arg (arg, int);
	      ++fmt;
	    }

	  /* Handle precision.  */
	  if (*fmt == '.' && fmt[1] == '*')
	    {
	      prec = va_arg (arg, int);
	      fmt += 2;
	    }

	  /* Recognize the l modifier.  It is only important on some
	     platforms where long and int have a different size.  We
	     can use the same code for size_t.  */
	  if (*fmt == 'l' || *fmt == 'Z')
	    {
#if LONG_MAX != INT_MAX
	      long_mod = 1;
#endif
	      ++fmt;
	    }

	  switch (*fmt)
	    {
	      /* Integer formatting.  */
	    case 'u':
	    case 'x':
	      {
		/* We have to make a difference if long and int have a
		   different size.  */
#if LONG_MAX != INT_MAX
		unsigned long int num = (long_mod
					 ? va_arg (arg, unsigned long int)
					 : va_arg (arg, unsigned int));
#else
		unsigned long int num = va_arg (arg, unsigned int);
#endif
		/* We use alloca() to allocate the buffer with the most
		   pessimistic guess for the size.  Using alloca() allows
		   having more than one integer formatting in a call.  */
		char *buf = (char *) alloca (3 * sizeof (unsigned long int));
		char *endp = &buf[3 * sizeof (unsigned long int)];
		char *cp = _itoa (num, endp, *fmt == 'x' ? 16 : 10, 0);

		/* Pad to the width the user specified.  */
		if (width != -1)
		  while (endp - cp < width)
		    *--cp = fill;

		iov[niov].iov_base = cp;
		iov[niov].iov_len = endp - cp;
		++niov;
	      }
	      break;

	    case 's':
	      /* Get the string argument.  */
	      iov[niov].iov_base = va_arg (arg, char *);
	      iov[niov].iov_len = strlen (iov[niov].iov_base);
	      if (prec != -1)
		iov[niov].iov_len = MIN ((size_t) prec, iov[niov].iov_len);
	      ++niov;
	      break;

	    case '%':
	      iov[niov].iov_base = (void *) fmt;
	      iov[niov].iov_len = 1;
	      ++niov;
	      break;

	    default:
	      assert (! "invalid format specifier");
	    }
	  ++fmt;
	}
      else if (*fmt == '\n')
	{
	  /* See whether we have to print a single newline character.  */
	  if (fmt == startp)
	    {
	      iov[niov].iov_base = (char *) startp;
	      iov[niov++].iov_len = 1;
	    }
	  else
	    /* No, just add it to the rest of the string.  */
	    ++iov[niov - 1].iov_len;

	  /* Next line, print a tag again.  */
	  tag_p = 1;
	  ++fmt;
	}
    }

  /* Finally write the result.  */
#ifdef HAVE_INLINED_SYSCALLS
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (writev, err, 3, fd, &iov, niov);
#elif RTLD_PRIVATE_ERRNO
  /* We have to take this lock just to be sure we don't clobber the private
     errno when it's being used by another thread that cares about it.
     Yet we must be sure not to try calling the lock functions before
     the thread library is fully initialized.  */
  if (__builtin_expect (INTUSE (_dl_starting_up), 0))
    __writev (fd, iov, niov);
  else
    {
      __rtld_lock_lock_recursive (GL(dl_load_lock));
      __writev (fd, iov, niov);
      __rtld_lock_unlock_recursive (GL(dl_load_lock));
    }
#else
  __writev (fd, iov, niov);
#endif
}


/* Write to debug file.  */
void
_dl_debug_printf (const char *fmt, ...)
{
  va_list arg;

  va_start (arg, fmt);
  _dl_debug_vdprintf (GLRO(dl_debug_fd), 1, fmt, arg);
  va_end (arg);
}


/* Write to debug file but don't start with a tag.  */
void
_dl_debug_printf_c (const char *fmt, ...)
{
  va_list arg;

  va_start (arg, fmt);
  _dl_debug_vdprintf (GLRO(dl_debug_fd), -1, fmt, arg);
  va_end (arg);
}


/* Write the given file descriptor.  */
void
_dl_dprintf (int fd, const char *fmt, ...)
{
  va_list arg;

  va_start (arg, fmt);
  _dl_debug_vdprintf (fd, 0, fmt, arg);
  va_end (arg);
}


/* Test whether given NAME matches any of the names of the given object.  */
int
internal_function
_dl_name_match_p (const char *name, struct link_map *map)
{
  if (strcmp (name, map->l_name) == 0)
    return 1;

  struct libname_list *runp = map->l_libname;

  while (runp != NULL)
    if (strcmp (name, runp->name) == 0)
      return 1;
    else
      runp = runp->next;

  return 0;
}
