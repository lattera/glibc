/* Common extra functions.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

/* This header file should only contain definitions compatible with
   C90.  (Using __attribute__ is fine because <features.h> provides a
   fallback.)  */

#ifndef SUPPORT_H
#define SUPPORT_H

#include <stddef.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/* Write a message to standard output.  Can be used in signal
   handlers.  */
void write_message (const char *message) __attribute__ ((nonnull (1)));

/* Avoid all the buffer overflow messages on stderr.  */
void ignore_stderr (void);

/* Set fortification error handler.  Used when tests want to verify that bad
   code is caught by the library.  */
void set_fortify_handler (void (*handler) (int sig));

/* Report an out-of-memory error for the allocation of SIZE bytes in
   FUNCTION, terminating the process.  */
void oom_error (const char *function, size_t size)
  __attribute__ ((nonnull (1)));

/* Return a pointer to a memory region of SIZE bytes.  The memory is
   initialized to zero and will be shared with subprocesses (across
   fork).  The returned pointer must be freed using
   support_shared_free; it is not compatible with the malloc
   functions.  */
void *support_shared_allocate (size_t size);

/* Deallocate a pointer returned by support_shared_allocate.  */
void support_shared_free (void *);

/* Write CONTENTS to the file PATH.  Create or truncate the file as
   needed.  The file mode is 0666 masked by the umask.  Terminate the
   process on error.  */
void support_write_file_string (const char *path, const char *contents);

/* Error-checking wrapper functions which terminate the process on
   error.  */

void *xmalloc (size_t) __attribute__ ((malloc));
void *xcalloc (size_t n, size_t s) __attribute__ ((malloc));
void *xrealloc (void *p, size_t n);
char *xasprintf (const char *format, ...)
  __attribute__ ((format (printf, 1, 2), malloc));
char *xstrdup (const char *);
char *xstrndup (const char *, size_t);

__END_DECLS

#endif /* SUPPORT_H */
