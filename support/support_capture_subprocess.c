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

#include <support/capture_subprocess.h>

#include <errno.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/xunistd.h>
#include <support/xsocket.h>

static void
transfer (const char *what, struct pollfd *pfd, struct xmemstream *stream)
{
  if (pfd->revents != 0)
    {
      char buf[1024];
      ssize_t ret = TEMP_FAILURE_RETRY (read (pfd->fd, buf, sizeof (buf)));
      if (ret < 0)
        {
          support_record_failure ();
          printf ("error: reading from subprocess %s: %m", what);
          pfd->events = 0;
          pfd->revents = 0;
        }
      else if (ret == 0)
        {
          /* EOF reached.  Stop listening.  */
          pfd->events = 0;
          pfd->revents = 0;
        }
      else
        /* Store the data just read.   */
        TEST_VERIFY (fwrite (buf, ret, 1, stream->out) == 1);
    }
}

struct support_capture_subprocess
support_capture_subprocess (void (*callback) (void *), void *closure)
{
  struct support_capture_subprocess result;
  xopen_memstream (&result.out);
  xopen_memstream (&result.err);

  int stdout_pipe[2];
  xpipe (stdout_pipe);
  int stderr_pipe[2];
  xpipe (stderr_pipe);

  TEST_VERIFY (fflush (stdout) == 0);
  TEST_VERIFY (fflush (stderr) == 0);

  pid_t pid = xfork ();
  if (pid == 0)
    {
      xclose (stdout_pipe[0]);
      xclose (stderr_pipe[0]);
      xdup2 (stdout_pipe[1], STDOUT_FILENO);
      xdup2 (stderr_pipe[1], STDERR_FILENO);
      callback (closure);
      _exit (0);
    }
  xclose (stdout_pipe[1]);
  xclose (stderr_pipe[1]);

  struct pollfd fds[2] =
    {
      { .fd = stdout_pipe[0], .events = POLLIN },
      { .fd = stderr_pipe[0], .events = POLLIN },
    };

  do
    {
      xpoll (fds, 2, -1);
      transfer ("stdout", &fds[0], &result.out);
      transfer ("stderr", &fds[1], &result.err);
    }
  while (fds[0].events != 0 || fds[1].events != 0);
  xclose (stdout_pipe[0]);
  xclose (stderr_pipe[0]);

  xfclose_memstream (&result.out);
  xfclose_memstream (&result.err);
  xwaitpid (pid, &result.status, 0);
  return result;
}

void
support_capture_subprocess_free (struct support_capture_subprocess *p)
{
  free (p->out.buffer);
  free (p->err.buffer);
}
