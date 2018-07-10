/* Test the __libc_readline_unlocked function.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

/* Exercise __libc_readline_unlocked with various combinations of line
   lengths, stdio buffer sizes, and line read buffer sizes.  */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/test-driver.h>
#include <support/xmemstream.h>
#include <support/xstdio.h>
#include <support/xunistd.h>

enum
  {
    maximum_line_length = 7,
    number_of_lines = 3,
  };

/* -1: Do not set buffer size.  0: unbuffered.  Otherwise, use this as
   the size of the buffer.  */
static int buffer_size;

/* These size of the buffer used for reading.  Must be at least 2.  */
static int read_size;

/* If a read files with ERANGE, increase the buffer size by this
   amount.  Must be positive.  */
static int read_size_increment;

/* If non-zero, do not reset the read size after an ERANGE error.  */
static int read_size_preserve;

/* If non-zero, no '\n' at the end of the file.  */
static int no_newline_at_eof;

/* Length of the line, or -1 if the line is not present.  */
static int line_lengths[number_of_lines];

/* The name of the test file.  */
static char *test_file_path;

/* The contents of the test file.  */
static char expected_contents[(maximum_line_length + 2) * number_of_lines + 1];
static size_t expected_length;

/* Returns a random byte which is not zero or the line terminator.  */
static char
random_char (void)
{
  static unsigned int rand_state = 1;
  while (true)
    {
      char result = rand_r (&rand_state) >> 16;
      if (result != 0 && result != '\n')
        return result;
    }
}

/* Create the test file.  */
static void
prepare (int argc, char **argv)
{
  int fd = create_temp_file ("tst-readline-", &test_file_path);
  TEST_VERIFY_EXIT (fd >= 0);
  xclose (fd);
}

/* Prepare the test file.  Return false if the test parameters are
   incongruent and the test should be skipped.  */
static bool
write_test_file (void)
{
  expected_length = 0;
  char *p = expected_contents;
  for (int lineno = 0; lineno < number_of_lines; ++lineno)
    for (int i = 0; i < line_lengths[lineno]; ++i)
      *p++ = random_char ();
  expected_length = p - &expected_contents[0];
  if (no_newline_at_eof)
    {
      if (expected_length == 0)
        return false;
      --expected_length;
      --p;
    }
  if (test_verbose > 0)
    {
      printf ("info: writing test file of %zu bytes:\n", expected_length);
      for (int i = 0; i < number_of_lines; ++i)
        printf (" line %d: %d\n", i, line_lengths[i]);
      if (no_newline_at_eof)
        puts ("  (no newline at EOF)");
    }
  TEST_VERIFY_EXIT (expected_length < sizeof (expected_contents));
  *p++ = '\0';
  support_write_file_string (test_file_path, expected_contents);
  return true;
}

/* Run a single test (a combination of a test file and read
   parameters).  */
static void
run_test (void)
{
  TEST_VERIFY_EXIT (read_size_increment > 0);
  if (test_verbose > 0)
    {
      printf ("info: running test: buffer_size=%d read_size=%d\n"
              "  read_size_increment=%d read_size_preserve=%d\n",
              buffer_size, read_size, read_size_increment, read_size_preserve);
    }

  struct xmemstream result;
  xopen_memstream (&result);

  FILE *fp = xfopen (test_file_path, "rce");
  char *fp_buffer = NULL;
  if (buffer_size == 0)
    TEST_VERIFY_EXIT (setvbuf (fp, NULL, _IONBF, 0) == 0);
  if (buffer_size > 0)
    {
      fp_buffer = xmalloc (buffer_size);
      TEST_VERIFY_EXIT (setvbuf (fp, fp_buffer, _IOFBF, buffer_size) == 0);
    }

  char *line_buffer = xmalloc (read_size);
  size_t line_buffer_size = read_size;

  while (true)
    {
      ssize_t ret = __libc_readline_unlocked
        (fp, line_buffer, line_buffer_size);
      if (ret < 0)
        {
          TEST_VERIFY (ret == -1);
          if (errno != ERANGE)
            FAIL_EXIT1 ("__libc_readline_unlocked: %m");
          line_buffer_size += read_size_increment;
          free (line_buffer);
          line_buffer = xmalloc (line_buffer_size);
          /* Try reading this line again.  */
        }
      else if (ret == 0)
        break;
      else
        {
          /* A line has been read.  Save it.  */
          TEST_VERIFY (ret == strlen (line_buffer));
          const char *pnl = strchr (line_buffer, '\n');
          /* If there is a \n, it must be at the end.  */
          TEST_VERIFY (pnl == NULL || pnl == line_buffer + ret - 1);
          fputs (line_buffer, result.out);

          /* Restore the original read size if required.  */
          if (line_buffer_size > read_size && !read_size_preserve)
            {
              line_buffer_size = read_size;
              free (line_buffer);
              line_buffer = xmalloc (line_buffer_size);
            }
        }
    }

  xfclose (fp);
  free (fp_buffer);
  free (line_buffer);

  xfclose_memstream (&result);
  TEST_VERIFY (result.length == expected_length);
  TEST_VERIFY (strcmp (result.buffer, expected_contents) == 0);
  if (test_verbose > 0)
    {
      printf ("info: expected (%zu): [[%s]]\n",
              expected_length, expected_contents);
      printf ("info:   actual (%zu): [[%s]]\n", result.length, result.buffer);
    }
  free (result.buffer);
}

/* Test one test file with multiple read parameters.  */
static void
test_one_file (void)
{
  for (buffer_size = -1; buffer_size <= maximum_line_length + 1; ++buffer_size)
    for (read_size = 2; read_size <= maximum_line_length + 2; ++read_size)
      for (read_size_increment = 1; read_size_increment <= 4;
           ++read_size_increment)
        for (read_size_preserve = 0; read_size_preserve < 2;
             ++read_size_preserve)
          run_test ();
}


static int
do_test (void)
{
  /* Set up the test file contents.  */
  for (line_lengths[0] = -1; line_lengths[0] <= maximum_line_length;
       ++line_lengths[0])
    for (line_lengths[1] = -1; line_lengths[1] <= maximum_line_length;
         ++line_lengths[1])
      for (line_lengths[2] = -1; line_lengths[2] <= maximum_line_length;
           ++line_lengths[2])
        for (no_newline_at_eof = 0; no_newline_at_eof < 2; ++no_newline_at_eof)
          {
            if (!write_test_file ())
              continue;
            test_one_file ();
          }
  free (test_file_path);
  return 0;
}

#define PREPARE prepare
#include <support/test-driver.c>
