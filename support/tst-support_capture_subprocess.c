/* Test capturing output from a subprocess.
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>
#include <sys/wait.h>
#include <unistd.h>

/* Write one byte at *P to FD and advance *P.  Do nothing if *P is
   '\0'.  */
static void
transfer (const unsigned char **p, int fd)
{
  if (**p != '\0')
    {
      TEST_VERIFY (write (fd, *p, 1) == 1);
      ++*p;
    }
}

/* Determine the order in which stdout and stderr are written.  */
enum write_mode { out_first, err_first, interleave,
                  write_mode_last =  interleave };

/* Describe what to write in the subprocess.  */
struct test
{
  char *out;
  char *err;
  enum write_mode write_mode;
  int signal;
  int status;
};

/* For use with support_capture_subprocess.  */
static void
callback (void *closure)
{
  const struct test *test = closure;
  bool mode_ok = false;
  switch (test->write_mode)
    {
    case out_first:
      TEST_VERIFY (fputs (test->out, stdout) >= 0);
      TEST_VERIFY (fflush (stdout) == 0);
      TEST_VERIFY (fputs (test->err, stderr) >= 0);
      TEST_VERIFY (fflush (stderr) == 0);
      mode_ok = true;
      break;
    case err_first:
      TEST_VERIFY (fputs (test->err, stderr) >= 0);
      TEST_VERIFY (fflush (stderr) == 0);
      TEST_VERIFY (fputs (test->out, stdout) >= 0);
      TEST_VERIFY (fflush (stdout) == 0);
      mode_ok = true;
      break;
    case interleave:
      {
        const unsigned char *pout = (const unsigned char *) test->out;
        const unsigned char *perr = (const unsigned char *) test->err;
        do
          {
            transfer (&pout, STDOUT_FILENO);
            transfer (&perr, STDERR_FILENO);
          }
        while (*pout != '\0' || *perr != '\0');
      }
      mode_ok = true;
      break;
    }
  TEST_VERIFY (mode_ok);

  if (test->signal != 0)
    raise (test->signal);
  exit (test->status);
}

/* Create a heap-allocated random string of letters.  */
static char *
random_string (size_t length)
{
  char *result = xmalloc (length + 1);
  for (size_t i = 0; i < length; ++i)
    result[i] = 'a' + (rand () % 26);
  result[length] = '\0';
  return result;
}

/* Check that the specific stream from the captured subprocess matches
   expectations.  */
static void
check_stream (const char *what, const struct xmemstream *stream,
              const char *expected)
{
  if (strcmp (stream->buffer, expected) != 0)
    {
      support_record_failure ();
      printf ("error: captured %s data incorrect\n"
              "  expected: %s\n"
              "  actual:   %s\n",
              what, expected, stream->buffer);
    }
  if (stream->length != strlen (expected))
    {
      support_record_failure ();
      printf ("error: captured %s data length incorrect\n"
              "  expected: %zu\n"
              "  actual:   %zu\n",
              what, strlen (expected), stream->length);
    }
}

static int
do_test (void)
{
  const int lengths[] = {0, 1, 17, 512, 20000, -1};

  /* Test multiple combinations of support_capture_subprocess.

     length_idx_stdout: Index into the lengths array above,
       controls how many bytes are written by the subprocess to
       standard output.
     length_idx_stderr: Same for standard error.
     write_mode: How standard output and standard error writes are
       ordered.
     signal: Exit with no signal if zero, with SIGTERM if one.
     status: Process exit status: 0 if zero, 3 if one.  */
  for (int length_idx_stdout = 0; lengths[length_idx_stdout] >= 0;
       ++length_idx_stdout)
    for (int length_idx_stderr = 0; lengths[length_idx_stderr] >= 0;
         ++length_idx_stderr)
      for (int write_mode = 0; write_mode < write_mode_last; ++write_mode)
        for (int signal = 0; signal < 2; ++signal)
          for (int status = 0; status < 2; ++status)
            {
              struct test test =
                {
                  .out = random_string (lengths[length_idx_stdout]),
                  .err = random_string (lengths[length_idx_stderr]),
                  .write_mode = write_mode,
                  .signal = signal * SIGTERM, /* 0 or SIGTERM.  */
                  .status = status * 3,       /* 0 or 3.  */
                };
              TEST_VERIFY (strlen (test.out) == lengths[length_idx_stdout]);
              TEST_VERIFY (strlen (test.err) == lengths[length_idx_stderr]);

              struct support_capture_subprocess result
                = support_capture_subprocess (callback, &test);
              check_stream ("stdout", &result.out, test.out);
              check_stream ("stderr", &result.err, test.err);
              if (test.signal != 0)
                {
                  TEST_VERIFY (WIFSIGNALED (result.status));
                  TEST_VERIFY (WTERMSIG (result.status) == test.signal);
                }
              else
                {
                  TEST_VERIFY (WIFEXITED (result.status));
                  TEST_VERIFY (WEXITSTATUS (result.status) == test.status);
                }
              support_capture_subprocess_free (&result);
              free (test.out);
              free (test.err);
            }
  return 0;
}

#include <support/test-driver.c>
