/* Miscellaneous support functions for dynamic linker
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <assert.h>
#include <fcntl.h>
#include <ldsodefs.h>
#include <link.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio-common/_itoa.h>

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
   protections.  *SIZEP gets the size of the file.  */

void *
internal_function
_dl_sysdep_read_whole_file (const char *file, size_t *sizep, int prot)
{
  void *result;
  struct stat64 st;
  int fd = __open (file, O_RDONLY);
  if (fd < 0)
    return NULL;
  if (__fxstat64 (_STAT_VER, fd, &st) < 0
      /* No need to map the file if it is empty.  */
      || st.st_size == 0)
    result = NULL;
  else
    {
      /* Map a copy of the file contents.  */
      result = __mmap (0, st.st_size, prot,
#ifdef MAP_COPY
                       MAP_COPY
#else
                       MAP_PRIVATE
#endif
#ifdef MAP_FILE
                       | MAP_FILE
#endif
                       , fd, 0);
      if (result == MAP_FAILED)
        result = NULL;
      else
        *sizep = st.st_size;
    }
  __close (fd);
  return result;
}


/* Descriptor to write debug messages to.  */
int _dl_debug_fd = 2;


void
_dl_sysdep_output (int fd, const char *msg, ...)
{
  va_list ap;

  va_start (ap, msg);
  do
    {
      size_t len = strlen (msg);
      __libc_write (fd, msg, len);
      msg = va_arg (ap, const char *);
    }
  while (msg != NULL);
  va_end (ap);
}


void
_dl_debug_message (int new_line, const char *msg, ...)
{
  /* We print the strings we get passed one after the other but start all
     lines using the current PID.  */
  int pid = 0;
  va_list ap;

  va_start (ap, msg);
  do
    if (msg[0] == '\0')
      /* Get the next argument.  */
      msg = va_arg (ap, const char *);
    else
      {
	const char *endp;

	/* We actually will print something in this line.  So print the
	   PID now if needed.  */
	if (new_line)
	  {
	    char buf[7];
	    char *p;
	    if (pid == 0)
	      pid = __getpid ();
	    assert (pid >= 0 && pid < 100000);
	    p = _itoa_word (pid, &buf[5], 10, 0);
	    while (p > buf)
	      *--p = '0';
	    buf[5] = ':';
	    buf[6] = '\t';
	    __libc_write (_dl_debug_fd, buf, 7);
	    new_line = 0;
	  }

	endp = msg + strcspn (msg, "\n");
	if (*endp == '\0')
	  {
	    __libc_write (_dl_debug_fd, msg, endp - msg);
	    msg = va_arg (ap, const char *);
	  }
	else
	  {
	    __libc_write (_dl_debug_fd, msg, endp - msg + 1);
	    msg = endp + 1;
	    new_line = 1;
	  }
      }
  while (msg != NULL);
  va_end (ap);
}
