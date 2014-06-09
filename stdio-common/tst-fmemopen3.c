/* fmemopen tests for append and read mode.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static void
print_buffer (const char *s, size_t n)
{
  size_t i;
  for (i=0; i<n; ++i)
    printf ("0x%02X (%c), ", s[i], s[i]);
}

/* This test check append mode initial position (a/a+) based on POSIX defition
   (BZ#6544 and BZ#13151).  */
static int
do_test_write_append (const char *mode)
{
  char buf[32] = "testing buffer";
  char exp[32] = "testing bufferXX";

  FILE *fp = fmemopen (buf, sizeof (buf), mode);

  fflush (fp);
  fprintf (fp, "X");
  fseek (fp, 0, SEEK_SET);
  fprintf (fp, "X");
  fclose (fp);

  if (strcmp (buf, exp) != 0)
    {
      printf ("%s: check failed: %s != %s\n", __FUNCTION__, buf, exp);
      return 1;
    }

  return 0;
}

/* This test check append mode initial position (a/a+) based on POSIX defition
   (BZ#6544 and BZ#13151) for buffer without null byte end.  */
static int
do_test_write_append_without_null (const char *mode)
{
  char buf[] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
  char exp[] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };

  /* If '\0' is not found in buffer, POSIX states that SEEK_SET should be
     the size argument.  */
  FILE *fp = fmemopen (buf, sizeof (buf) - 2, "a");

  fflush (fp);
  fputc (0x70, fp);
  fseek (fp, 0, SEEK_SET);
  fputc (0x70, fp);
  fputc (0x70, fp);
  fclose (fp);

  /* POSIX also states that a write operation on the stream shall not advance
     the current buffer size beyond the size given in fmemopen, so the string
     should be same.  */
  if (memcmp (buf, exp, sizeof (buf)) != 0)
    {
      printf ("%s: check failed: ", __FUNCTION__);
      print_buffer (buf, sizeof (buf));
      printf ("!= ");
      print_buffer (exp, sizeof (exp));
      printf ("\n");
      return 1;
    }

  return 0;
}

/* This test check for initial position and feek value for fmemopen objects
   opened with append mode.  */
static int
do_test_read_append (void)
{
  char buf[32] = "testing buffer";
  size_t buflen = strlen (buf);
  long fpos;

  /* POSIX defines for 'a+' the initial position is the first null byte.  */
  FILE *fp = fmemopen (buf, sizeof (buf), "a+");

  fpos = ftell (fp);
  if (fpos != buflen)
    {
      printf ("%s: ftell|SEEK_SET (fp) %li != strlen (%s) %zu\n",
	      __FUNCTION__, fpos, buf, buflen);
      fclose (fp);
      return 1;
    }

  fseek (fp, 0, SEEK_END);

  if (fpos != buflen)
    {
      printf ("%s: ftell|SEEK_END (fp) %li != strlen (%s) %zu\n",
	      __FUNCTION__, fpos, buf, buflen);
      fclose (fp);
      return 1;
    }
  fclose (fp);

  /* Check if attempting to read past the current size, defined as strlen (buf)
     yield an EOF.  */
  fp = fmemopen (buf, sizeof (buf), "a+");
  if (getc(fp) != EOF)
    {
      printf ("%s: getc(fp) != EOF\n", __FUNCTION__);
      fclose (fp);
      return -1;
    }

  fclose (fp);

  return 0;
}

/* This test check for fseek (SEEK_END) using negative offsets (BZ#14292).  The
   starting position of descriptor is different base on the opening mode.  */
static int
do_test_read_seek_neg (const char *mode, const char *expected)
{
  char buf[] = "abcdefghijklmnopqrstuvxz0123456789";
  char tmp[10];
  size_t tmps = sizeof (tmps);
  long offset = -11;

  FILE *fp = fmemopen (buf, sizeof (buf), mode);
  fseek (fp, offset, SEEK_END);
  fread (tmp, tmps, 1, fp);

  if (memcmp (tmp, expected, tmps) != 0)
    {
      printf ("%s: fmemopen(%s) - fseek (fp, %li, SEEK_END):\n",
	      __FUNCTION__, mode, offset);
      printf ("  returned: ");
      print_buffer (tmp, tmps);
      printf ("\n");
      printf ("  expected: ");
      print_buffer (expected, tmps);
      printf ("\n");
      return 1;
    }

  fclose (fp);

  return 0;
}

static int
do_test_read_seek_negative (void)
{
  int ret = 0;

  /* 'r' and 'w' modes defines the initial position at the buffer start and
     seek with SEEK_END shall seek relative to its size give in fmemopen
     call.  The expected tmp result is 0 to 9 *without* the ending null  */
  ret += do_test_read_seek_neg ("r", "0123456789");
  /* 'a+' mode sets the initial position at the first null byte in buffer and
    SEEK_END shall seek relative to its size as well.  The expected result is
    z012345678, since SEEK_END plus a+ start at '\0', not size.  */
  ret += do_test_read_seek_neg ("a+", "z012345678");

  return ret;
}

static int
do_test (void)
{
  int ret = 0;

  ret += do_test_write_append ("a");
  ret += do_test_write_append_without_null ("a");
  ret += do_test_write_append ("a+");
  ret += do_test_write_append_without_null ("a+");

  ret += do_test_read_append ();

  ret += do_test_read_seek_negative ();

  return ret;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
