/* Capture output from a subprocess.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#ifndef SUPPORT_CAPTURE_SUBPROCESS_H
#define SUPPORT_CAPTURE_SUBPROCESS_H

#include <support/xmemstream.h>

struct support_capture_subprocess
{
  struct xmemstream out;
  struct xmemstream err;
  int status;
};

/* Invoke CALLBACK (CLOSURE) in a subprocess and capture standard
   output, standard error, and the exit status.  The out.buffer and
   err.buffer members in the result are null-terminated strings which
   can be examined by the caller (out.out and err.out are NULL).  */
struct support_capture_subprocess support_capture_subprocess
  (void (*callback) (void *), void *closure);

/* Deallocate the subprocess data captured by
   support_capture_subprocess.  */
void support_capture_subprocess_free (struct support_capture_subprocess *);

enum support_capture_allow
{
  /* No output is allowed.  */
  sc_allow_none = 0x01,
  /* Output to stdout is permitted.  */
  sc_allow_stdout = 0x02,
  /* Output to standard error is permitted.  */
  sc_allow_stderr = 0x04,
};

/* Check that the subprocess exited with STATUS and that only the
   allowed outputs happened.  ALLOWED is a combination of
   support_capture_allow flags.  Report errors under the CONTEXT
   message.  */
void support_capture_subprocess_check (struct support_capture_subprocess *,
                                       const char *context, int status,
                                       int allowed)
  __attribute__ ((nonnull (1, 2)));

#endif /* SUPPORT_CAPTURE_SUBPROCESS_H */
