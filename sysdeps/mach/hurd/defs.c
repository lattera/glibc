/* Definitions of global stdio data structures.
   Copyright (C) 1991,92,93,94,95,97,2000 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <hurd/fd.h>
#include <unistd.h>

FILE *stdin, *stdout, *stderr;

/* Pointer to the first stream in the list.  */
FILE *__stdio_head = NULL;

/* XXX should be __init_stdio? */
static void
init_stdio (void)
{
  inline void init (FILE **streamptr, int fd)
    {
      /* We want to use the existing FILE object if one has been allocated.
	 (This will only be the case if our image came from something like
	 Emacs's unexec, where we were called in the first run.)  */
      FILE *s = *streamptr ?: __newstream ();
      struct hurd_fd *d = _hurd_fd_get (fd);
      if (d == NULL)
	{
	  /* There is no file descriptor allocated.  We want the standard
	     streams to always refer to their standard file descriptors, even
	     if those descriptors are not set up until later.  So allocate
	     the descriptor structure with no ports and store it in the
	     stream.  Operations will fail until ports are installed in the
	     file descriptor.  */
	  if (d = _hurd_alloc_fd (NULL, fd))
	    __spin_unlock (&d->port.lock);
	}
      if (s)
	s->__cookie = d;
      *streamptr = s;
    }
#define S(NAME, FD, MODE) \
  init (&NAME, FD); if (NAME) NAME->__mode.__##MODE = 1;

  S (stdin, STDIN_FILENO, read);
  S (stdout, STDOUT_FILENO, write);
  S (stderr, STDERR_FILENO, write);

#undef S

  if (stderr)
    stderr->__userbuf = 1;	/* stderr is always unbuffered.  */

  (void) &init_stdio;		/* Avoid "defined but not used" warning.  */
}
/* This initializer will be run along with other vanilla libc initializers
   in a normal Posixoid environment.  The earlier Hurd-speciifc initializer
   phases cannot use normal facilities like malloc (which stdio uses).  */
text_set_element (__libc_subinit, init_stdio);

/* This function MUST be in this file!
   This is because we want _cleanup to go into the __libc_atexit set
   when any stdio code is used (and to use any stdio code, one must reference
   something defined in this file), and since only local symbols can be made
   set elements, having the set element stab entry here and _cleanup elsewhere
   loses; and having them both elsewhere loses because there is no reference
   to cause _cleanup to be linked in.  */

void
_cleanup (void)
{
  __fcloseall ();
}
text_set_element (__libc_atexit, _cleanup);
