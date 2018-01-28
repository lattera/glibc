/* Tests for copy_file_range.
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

#include <array_length.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <poll.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/test-driver.h>
#include <support/xunistd.h>
#ifdef CLONE_NEWNS
# include <sys/mount.h>
#endif

/* Boolean flags which indicate whether to use pointers with explicit
   output flags.  */
static int do_inoff;
static int do_outoff;

/* Name and descriptors of the input files.  Files are truncated and
   reopened (with O_RDWR) between tests.  */
static char *infile;
static int infd;
static char *outfile;
static int outfd;

/* Like the above, but on a different file system.  xdevfile can be
   NULL if no suitable file system has been found.  */
static char *xdevfile;

/* Input and output offsets.  Set according to do_inoff and do_outoff
   before the test.  The offsets themselves are always set to
   zero.  */
static off64_t inoff;
static off64_t *pinoff;
static off64_t outoff;
static off64_t *poutoff;

/* These are a collection of copy sizes used in tests.  The selection
   takes into account that the fallback implementation uses an
   internal buffer of 8192 bytes.  */
enum { maximum_size = 99999 };
static const int typical_sizes[] =
  { 0, 1, 2, 3, 1024, 2048, 4096, 8191, 8192, 8193, 16383, 16384, 16385,
    maximum_size };

/* The random contents of this array can be used as a pattern to check
   for correct write operations.  */
static unsigned char random_data[maximum_size];

/* The size chosen by the test harness.  */
static int current_size;

/* Maximum writable file offset.  Updated by find_maximum_offset
   below.  */
static off64_t maximum_offset;

/* Error code when crossing the offset.  */
static int maximum_offset_errno;

/* If true: Writes which cross the limit will fail.  If false: Writes
   which cross the limit will result in a partial write.  */
static bool maximum_offset_hard_limit;

/* Fills maximum_offset etc. above.  Truncates outfd as a side
   effect.  */
static void
find_maximum_offset (void)
{
  xftruncate (outfd, 0);
  if (maximum_offset != 0)
    return;

  uint64_t upper = -1;
  upper >>= 1;                  /* Maximum of off64_t.  */
  TEST_VERIFY ((off64_t) upper > 0);
  TEST_VERIFY ((off64_t) (upper + 1) < 0);
  if (lseek64 (outfd, upper, SEEK_SET) >= 0)
    {
      if (write (outfd, "", 1) == 1)
        FAIL_EXIT1 ("created a file larger than the off64_t range");
    }

  uint64_t lower = 1024 * 1024; /* A reasonable minimum file size.  */
  /* Loop invariant: writing at lower succeeds, writing at upper fails.  */
  while (lower + 1 < upper)
    {
      uint64_t middle = (lower + upper) / 2;
      if (test_verbose > 0)
        printf ("info: %s: remaining test range %" PRIu64 " .. %" PRIu64
                ", probe at %" PRIu64 "\n", __func__, lower, upper, middle);
      xftruncate (outfd, 0);
      if (lseek64 (outfd, middle, SEEK_SET) >= 0
          && write (outfd, "", 1) == 1)
        lower = middle;
      else
        upper = middle;
    }
  TEST_VERIFY (lower + 1 == upper);
  maximum_offset = lower;
  printf ("info: maximum writable file offset: %" PRIu64 " (%" PRIx64 ")\n",
          lower, lower);

  /* Check that writing at the valid offset actually works.  */
  xftruncate (outfd, 0);
  xlseek (outfd, lower, SEEK_SET);
  TEST_COMPARE (write (outfd, "", 1), 1);

  /* Cross the boundary with a two-byte write.  This can either result
     in a short write, or a failure.  */
  xlseek (outfd, lower, SEEK_SET);
  ssize_t ret = write (outfd, " ", 2);
  if (ret < 0)
    {
      maximum_offset_errno = errno;
      maximum_offset_hard_limit = true;
    }
  else
    maximum_offset_hard_limit = false;

  /* Check that writing at the next offset actually fails.  This also
     obtains the expected errno value.  */
  xftruncate (outfd, 0);
  const char *action;
  if (lseek64 (outfd, lower + 1, SEEK_SET) != 0)
    {
      if (write (outfd, "", 1) != -1)
        FAIL_EXIT1 ("write to impossible offset %" PRIu64 " succeeded",
                    lower + 1);
      action = "writing";
      int errno_copy = errno;
      if (maximum_offset_hard_limit)
        TEST_COMPARE (errno_copy, maximum_offset_errno);
      else
        maximum_offset_errno = errno_copy;
    }
  else
    {
      action = "seeking";
      maximum_offset_errno = errno;
    }
  printf ("info: %s out of range fails with %m (%d)\n",
          action, maximum_offset_errno);

  xftruncate (outfd, 0);
  xlseek (outfd, 0, SEEK_SET);
}

/* Perform a copy of a file.  */
static void
simple_file_copy (void)
{
  xwrite (infd, random_data, current_size);

  int length;
  int in_skipped; /* Expected skipped bytes in input.  */
  if (do_inoff)
    {
      xlseek (infd, 1, SEEK_SET);
      inoff = 2;
      length = current_size - 3;
      in_skipped = 2;
    }
  else
    {
      xlseek (infd, 3, SEEK_SET);
      length = current_size - 5;
      in_skipped = 3;
    }
  int out_skipped; /* Expected skipped bytes before the written data.  */
  if (do_outoff)
    {
      xlseek (outfd, 4, SEEK_SET);
      outoff = 5;
      out_skipped = 5;
    }
  else
    {
      xlseek (outfd, 6, SEEK_SET);
      length = current_size - 6;
      out_skipped = 6;
    }
  if (length < 0)
    length = 0;

  TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                 length, 0), length);
  if (do_inoff)
    {
      TEST_COMPARE (inoff, 2 + length);
      TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 1);
    }
  else
    TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 3 + length);
  if (do_outoff)
    {
      TEST_COMPARE (outoff, 5 + length);
      TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 4);
    }
  else
    TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 6 + length);

  struct stat64 st;
  xfstat (outfd, &st);
  if (length > 0)
    TEST_COMPARE (st.st_size, out_skipped + length);
  else
    {
      /* If we did not write anything, we also did not add any
         padding.  */
      TEST_COMPARE (st.st_size, 0);
      return;
    }

  xlseek (outfd, 0, SEEK_SET);
  char *bytes = xmalloc (st.st_size);
  TEST_COMPARE (read (outfd, bytes, st.st_size), st.st_size);
  for (int i = 0; i < out_skipped; ++i)
    TEST_COMPARE (bytes[i], 0);
  TEST_VERIFY (memcmp (bytes + out_skipped, random_data + in_skipped,
                       length) == 0);
  free (bytes);
}

/* Test that reading from a pipe willfails.  */
static void
pipe_as_source (void)
{
  int pipefds[2];
  xpipe (pipefds);

  for (int length = 0; length < 2; ++length)
    {
      if (test_verbose > 0)
        printf ("info: %s: length=%d\n", __func__, length);

      /* Make sure that there is something to copy in the pipe.  */
      xwrite (pipefds[1], "@", 1);

      TEST_COMPARE (copy_file_range (pipefds[0], pinoff, outfd, poutoff,
                                     length, 0), -1);
      /* Linux 4.10 and later return EINVAL.  Older kernels return
         EXDEV.  */
      TEST_VERIFY (errno == EINVAL || errno == EXDEV);
      TEST_COMPARE (inoff, 0);
      TEST_COMPARE (outoff, 0);
      TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 0);

      /* Make sure that nothing was read.  */
      char buf = 'A';
      TEST_COMPARE (read (pipefds[0], &buf, 1), 1);
      TEST_COMPARE (buf, '@');
    }

  xclose (pipefds[0]);
  xclose (pipefds[1]);
}

/* Test that writing to a pipe fails.  */
static void
pipe_as_destination (void)
{
  /* Make sure that there is something to read in the input file.  */
  xwrite (infd, "abc", 3);
  xlseek (infd, 0, SEEK_SET);

  int pipefds[2];
  xpipe (pipefds);

  for (int length = 0; length < 2; ++length)
    {
      if (test_verbose > 0)
        printf ("info: %s: length=%d\n", __func__, length);

      TEST_COMPARE (copy_file_range (infd, pinoff, pipefds[1], poutoff,
                                     length, 0), -1);
      /* Linux 4.10 and later return EINVAL.  Older kernels return
         EXDEV.  */
      TEST_VERIFY (errno == EINVAL || errno == EXDEV);
      TEST_COMPARE (inoff, 0);
      TEST_COMPARE (outoff, 0);
      TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 0);

      /* Make sure that nothing was written.  */
      struct pollfd pollfd = { .fd = pipefds[0], .events = POLLIN, };
      TEST_COMPARE (poll (&pollfd, 1, 0), 0);
    }

  xclose (pipefds[0]);
  xclose (pipefds[1]);
}

/* Test a write failure after (potentially) writing some bytes.
   Failure occurs near the start of the buffer.  */
static void
delayed_write_failure_beginning (void)
{
  /* We need to write something to provoke the error.  */
  if (current_size == 0)
    return;
  xwrite (infd, random_data, sizeof (random_data));
  xlseek (infd, 0, SEEK_SET);

  /* Write failure near the start.  The actual error code varies among
     file systems.  */
  find_maximum_offset ();
  off64_t where = maximum_offset;

  if (current_size == 1)
    ++where;
  outoff = where;
  if (do_outoff)
    xlseek (outfd, 1, SEEK_SET);
  else
    xlseek (outfd, where, SEEK_SET);
  if (maximum_offset_hard_limit || where > maximum_offset)
    {
      TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                     sizeof (random_data), 0), -1);
      TEST_COMPARE (errno, maximum_offset_errno);
      TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 0);
      TEST_COMPARE (inoff, 0);
      if (do_outoff)
        TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 1);
      else
        TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), where);
      TEST_COMPARE (outoff, where);
      struct stat64 st;
      xfstat (outfd, &st);
      TEST_COMPARE (st.st_size, 0);
    }
  else
    {
      /* The offset is not a hard limit.  This means we write one
         byte.  */
      TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                     sizeof (random_data), 0), 1);
      if (do_inoff)
        {
          TEST_COMPARE (inoff, 1);
          TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 0);
        }
      else
        {
          TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 1);
          TEST_COMPARE (inoff, 0);
        }
      if (do_outoff)
        {
          TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 1);
          TEST_COMPARE (outoff, where + 1);
        }
      else
        {
          TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), where + 1);
          TEST_COMPARE (outoff, where);
        }
      struct stat64 st;
      xfstat (outfd, &st);
      TEST_COMPARE (st.st_size, where + 1);
    }
}

/* Test a write failure after (potentially) writing some bytes.
   Failure occurs near the end of the buffer.  */
static void
delayed_write_failure_end (void)
{
  if (current_size <= 1)
    /* This would be same as the first test because there is not
       enough data to write to make a difference.  */
    return;
  xwrite (infd, random_data, sizeof (random_data));
  xlseek (infd, 0, SEEK_SET);

  find_maximum_offset ();
  off64_t where = maximum_offset - current_size + 1;
  if (current_size == sizeof (random_data))
    /* Otherwise we do not reach the non-writable byte.  */
    ++where;
  outoff = where;
  if (do_outoff)
    xlseek (outfd, 1, SEEK_SET);
  else
    xlseek (outfd, where, SEEK_SET);
  ssize_t ret = copy_file_range (infd, pinoff, outfd, poutoff,
                                 sizeof (random_data), 0);
  if (ret < 0)
    {
      TEST_COMPARE (ret, -1);
      TEST_COMPARE (errno, maximum_offset_errno);
      struct stat64 st;
      xfstat (outfd, &st);
      TEST_COMPARE (st.st_size, 0);
    }
  else
    {
      /* The first copy succeeded.  This happens in the emulation
         because the internal buffer of limited size does not
         necessarily cross the off64_t boundary on the first write
         operation.  */
      if (test_verbose > 0)
        printf ("info:   copy_file_range (%zu) returned %zd\n",
                sizeof (random_data), ret);
      TEST_VERIFY (ret > 0);
      TEST_VERIFY (ret < maximum_size);
      struct stat64 st;
      xfstat (outfd, &st);
      TEST_COMPARE (st.st_size, where + ret);
      if (do_inoff)
        {
          TEST_COMPARE (inoff, ret);
          TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 0);
        }
      else
          TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), ret);

      char *buffer = xmalloc (ret);
      TEST_COMPARE (pread64 (outfd, buffer, ret, where), ret);
      TEST_VERIFY (memcmp (buffer, random_data, ret) == 0);
      free (buffer);

      /* The second copy fails.  */
      TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                     sizeof (random_data), 0), -1);
      TEST_COMPARE (errno, maximum_offset_errno);
    }
}

/* Test a write failure across devices.  */
static void
cross_device_failure (void)
{
  if (xdevfile == NULL)
    /* Subtest not supported due to missing cross-device file.  */
    return;

  /* We need something to write.  */
  xwrite (infd, random_data, sizeof (random_data));
  xlseek (infd, 0, SEEK_SET);

  int xdevfd = xopen (xdevfile, O_RDWR | O_LARGEFILE, 0);
  TEST_COMPARE (copy_file_range (infd, pinoff, xdevfd, poutoff,
                                 current_size, 0), -1);
  TEST_COMPARE (errno, EXDEV);
  TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 0);
  struct stat64 st;
  xfstat (xdevfd, &st);
  TEST_COMPARE (st.st_size, 0);

  xclose (xdevfd);
}

/* Try to exercise ENOSPC behavior with a tempfs file system (so that
   we do not have to fill up a regular file system to get the error).
   This function runs in a subprocess, so that we do not change the
   mount namespace of the actual test process.  */
static void
enospc_failure_1 (void *closure)
{
#ifdef CLONE_NEWNS
  support_become_root ();

  /* Make sure that we do not alter the file system mounts of the
     parents.  */
  if (! support_enter_mount_namespace ())
    {
      printf ("warning: ENOSPC test skipped\n");
      return;
    }

  char *mountpoint = closure;
  if (mount ("none", mountpoint, "tmpfs", MS_NODEV | MS_NOEXEC,
             "size=500k") != 0)
    {
      printf ("warning: could not mount tmpfs at %s: %m\n", mountpoint);
      return;
    }

  /* The source file must reside on the same file system.  */
  char *intmpfsfile = xasprintf ("%s/%s", mountpoint, "in");
  int intmpfsfd = xopen (intmpfsfile, O_RDWR | O_CREAT | O_LARGEFILE, 0600);
  xwrite (intmpfsfd, random_data, sizeof (random_data));
  xlseek (intmpfsfd, 1, SEEK_SET);
  inoff = 1;

  char *outtmpfsfile = xasprintf ("%s/%s", mountpoint, "out");
  int outtmpfsfd = xopen (outtmpfsfile, O_RDWR | O_CREAT | O_LARGEFILE, 0600);

  /* Fill the file with data until ENOSPC is reached.  */
  while (true)
    {
      ssize_t ret = write (outtmpfsfd, random_data, sizeof (random_data));
      if (ret < 0 && errno != ENOSPC)
        FAIL_EXIT1 ("write to %s: %m", outtmpfsfile);
      if (ret < sizeof (random_data))
        break;
    }
  TEST_COMPARE (write (outtmpfsfd, "", 1), -1);
  TEST_COMPARE (errno, ENOSPC);
  off64_t maxsize = xlseek (outtmpfsfd, 0, SEEK_CUR);
  TEST_VERIFY_EXIT (maxsize > sizeof (random_data));

  /* Constructed the expected file contents.  */
  char *expected = xmalloc (maxsize);
  TEST_COMPARE (pread64 (outtmpfsfd, expected, maxsize, 0), maxsize);
  /* Go back a little, so some bytes can be written.  */
  enum { offset = 20000 };
  TEST_VERIFY_EXIT (offset < maxsize);
  TEST_VERIFY_EXIT (offset < sizeof (random_data));
  memcpy (expected + maxsize - offset, random_data + 1, offset);

  if (do_outoff)
    {
      outoff = maxsize - offset;
      xlseek (outtmpfsfd, 2, SEEK_SET);
    }
  else
    xlseek (outtmpfsfd, -offset, SEEK_CUR);

  /* First call is expected to succeed because we made room for some
     bytes.  */
  TEST_COMPARE (copy_file_range (intmpfsfd, pinoff, outtmpfsfd, poutoff,
                                 maximum_size, 0), offset);
  if (do_inoff)
    {
      TEST_COMPARE (inoff, 1 + offset);
      TEST_COMPARE (xlseek (intmpfsfd, 0, SEEK_CUR), 1);
    }
  else
      TEST_COMPARE (xlseek (intmpfsfd, 0, SEEK_CUR), 1 + offset);
  if (do_outoff)
    {
      TEST_COMPARE (outoff, maxsize);
      TEST_COMPARE (xlseek (outtmpfsfd, 0, SEEK_CUR), 2);
    }
  else
    TEST_COMPARE (xlseek (outtmpfsfd, 0, SEEK_CUR), maxsize);
  struct stat64 st;
  xfstat (outtmpfsfd, &st);
  TEST_COMPARE (st.st_size, maxsize);
  char *actual = xmalloc (st.st_size);
  TEST_COMPARE (pread64 (outtmpfsfd, actual, st.st_size, 0), st.st_size);
  TEST_VERIFY (memcmp (expected, actual, maxsize) == 0);

  /* Second call should fail with ENOSPC.  */
  TEST_COMPARE (copy_file_range (intmpfsfd, pinoff, outtmpfsfd, poutoff,
                                 maximum_size, 0), -1);
  TEST_COMPARE (errno, ENOSPC);

  /* Offsets should be unchanged.  */
  if (do_inoff)
    {
      TEST_COMPARE (inoff, 1 + offset);
      TEST_COMPARE (xlseek (intmpfsfd, 0, SEEK_CUR), 1);
    }
  else
    TEST_COMPARE (xlseek (intmpfsfd, 0, SEEK_CUR), 1 + offset);
  if (do_outoff)
    {
      TEST_COMPARE (outoff, maxsize);
      TEST_COMPARE (xlseek (outtmpfsfd, 0, SEEK_CUR), 2);
    }
  else
    TEST_COMPARE (xlseek (outtmpfsfd, 0, SEEK_CUR), maxsize);
  TEST_COMPARE (xlseek (outtmpfsfd, 0, SEEK_END), maxsize);
  TEST_COMPARE (pread64 (outtmpfsfd, actual, maxsize, 0), maxsize);
  TEST_VERIFY (memcmp (expected, actual, maxsize) == 0);

  free (actual);
  free (expected);

  xclose (intmpfsfd);
  xclose (outtmpfsfd);
  free (intmpfsfile);
  free (outtmpfsfile);

#else /* !CLONE_NEWNS */
  puts ("warning: ENOSPC test skipped (no mount namespaces)");
#endif
}

/* Call enospc_failure_1 in a subprocess.  */
static void
enospc_failure (void)
{
  char *mountpoint
    = support_create_temp_directory ("tst-copy_file_range-enospc-");
  support_isolate_in_subprocess (enospc_failure_1, mountpoint);
  free (mountpoint);
}

/* The target file descriptor must have O_APPEND enabled.  */
static void
oappend_failure (void)
{
  /* Add data, to make sure we do not fail because there is
     insufficient input data.  */
  xwrite (infd, random_data, current_size);
  xlseek (infd, 0, SEEK_SET);

  xclose (outfd);
  outfd = xopen (outfile, O_RDWR | O_APPEND, 0);
  TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                 current_size, 0), -1);
  TEST_COMPARE (errno, EBADF);
}

/* Test that a short input file results in a shortened copy.  */
static void
short_copy (void)
{
  if (current_size == 0)
    /* Nothing to shorten.  */
    return;

  /* Two subtests, one with offset 0 and current_size - 1 bytes, and
     another one with current_size bytes, but offset 1.  */
  for (int shift = 0; shift < 2; ++shift)
    {
      if (test_verbose > 0)
        printf ("info:   shift=%d\n", shift);
      xftruncate (infd, 0);
      xlseek (infd, 0, SEEK_SET);
      xwrite (infd, random_data, current_size - !shift);

      if (do_inoff)
        {
          inoff = shift;
          xlseek (infd, 2, SEEK_SET);
        }
      else
        {
          inoff = 3;
          xlseek (infd, shift, SEEK_SET);
        }
      ftruncate (outfd, 0);
      xlseek (outfd, 0, SEEK_SET);
      outoff = 0;

      /* First call copies current_size - 1 bytes.  */
      TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                     current_size, 0), current_size - 1);
      char *buffer = xmalloc (current_size);
      TEST_COMPARE (pread64 (outfd, buffer, current_size, 0),
                    current_size - 1);
      TEST_VERIFY (memcmp (buffer, random_data + shift, current_size - 1)
                   == 0);
      free (buffer);

      if (do_inoff)
        {
          TEST_COMPARE (inoff, current_size - 1 + shift);
          TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 2);
        }
      else
        TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), current_size - 1 + shift);
      if (do_outoff)
        {
          TEST_COMPARE (outoff, current_size - 1);
          TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 0);
        }
      else
        TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), current_size - 1);

      /* First call copies zero bytes.  */
      TEST_COMPARE (copy_file_range (infd, pinoff, outfd, poutoff,
                                     current_size, 0), 0);
      /* And the offsets are unchanged.  */
      if (do_inoff)
        {
          TEST_COMPARE (inoff, current_size - 1 + shift);
          TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), 2);
        }
      else
        TEST_COMPARE (xlseek (infd, 0, SEEK_CUR), current_size - 1 + shift);
      if (do_outoff)
        {
          TEST_COMPARE (outoff, current_size - 1);
          TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), 0);
        }
      else
        TEST_COMPARE (xlseek (outfd, 0, SEEK_CUR), current_size - 1);
    }
}

/* A named test function.  */
struct test_case
{
  const char *name;
  void (*func) (void);
  bool sizes; /* If true, call the test with different current_size values.  */
};

/* The available test cases.  */
static struct test_case tests[] =
  {
    { "simple_file_copy", simple_file_copy, .sizes = true },
    { "pipe_as_source", pipe_as_source, },
    { "pipe_as_destination", pipe_as_destination, },
    { "delayed_write_failure_beginning", delayed_write_failure_beginning,
      .sizes = true },
    { "delayed_write_failure_end", delayed_write_failure_end, .sizes = true },
    { "cross_device_failure", cross_device_failure, .sizes = true },
    { "enospc_failure", enospc_failure, },
    { "oappend_failure", oappend_failure, .sizes = true },
    { "short_copy", short_copy, .sizes = true },
  };

static int
do_test (void)
{
  for (unsigned char *p = random_data; p < array_end (random_data); ++p)
    *p = rand () >> 24;

  infd = create_temp_file ("tst-copy_file_range-in-", &infile);
  xclose (create_temp_file ("tst-copy_file_range-out-", &outfile));

  /* Try to find a different directory from the default input/output
     file.  */
  {
    struct stat64 instat;
    xfstat (infd, &instat);
    static const char *const candidates[] =
      { NULL, "/var/tmp", "/dev/shm" };
    for (const char *const *c = candidates; c < array_end (candidates); ++c)
      {
        const char *path = *c;
        char *to_free = NULL;
        if (path == NULL)
          {
            to_free = xreadlink ("/proc/self/exe");
            path = dirname (to_free);
          }

        struct stat64 cstat;
        xstat (path, &cstat);
        if (cstat.st_dev == instat.st_dev)
          {
            free (to_free);
            continue;
          }

        printf ("info: using alternate temporary files directory: %s\n", path);
        xdevfile = xasprintf ("%s/tst-copy_file_range-xdev-XXXXXX", path);
        free (to_free);
        break;
      }
    if (xdevfile != NULL)
      {
        int xdevfd = mkstemp (xdevfile);
        if (xdevfd < 0)
          FAIL_EXIT1 ("mkstemp (\"%s\"): %m", xdevfile);
        struct stat64 xdevst;
        xfstat (xdevfd, &xdevst);
        TEST_VERIFY (xdevst.st_dev != instat.st_dev);
        add_temp_file (xdevfile);
        xclose (xdevfd);
      }
    else
      puts ("warning: no alternate directory on different file system found");
  }
  xclose (infd);

  for (do_inoff = 0; do_inoff < 2; ++do_inoff)
    for (do_outoff = 0; do_outoff < 2; ++do_outoff)
      for (struct test_case *test = tests; test < array_end (tests); ++test)
        for (const int *size = typical_sizes;
             size < array_end (typical_sizes); ++size)
          {
            current_size = *size;
            if (test_verbose > 0)
              printf ("info: %s do_inoff=%d do_outoff=%d current_size=%d\n",
                      test->name, do_inoff, do_outoff, current_size);

            inoff = 0;
            if (do_inoff)
              pinoff = &inoff;
            else
              pinoff = NULL;
            outoff = 0;
            if (do_outoff)
              poutoff = &outoff;
            else
              poutoff = NULL;

            infd = xopen (infile, O_RDWR | O_LARGEFILE, 0);
            xftruncate (infd, 0);
            outfd = xopen (outfile, O_RDWR | O_LARGEFILE, 0);
            xftruncate (outfd, 0);

            test->func ();

            xclose (infd);
            xclose (outfd);

            if (!test->sizes)
              /* Skip the other sizes unless they have been
                 requested.  */
              break;
          }

  free (infile);
  free (outfile);
  free (xdevfile);

  return 0;
}

#include <support/test-driver.c>
