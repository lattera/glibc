/* Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2004.

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

#include <fcntl.h>
#include <paths.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *temp_filename;
static void do_prepare (void);
static int do_test (void);
#define PREPARE(argc, argv) do_prepare ()
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"

static void
do_prepare (void)
{
  int temp_fd = create_temp_file ("tst-chk1.", &temp_filename);
  if (temp_fd == -1)
    {
      printf ("cannot create temporary file: %m\n");
      exit (1);
    }

  const char *strs = "abcdefgh\nABCDEFGHI\nabcdefghij\nABCDEFGHIJ";
  if (write (temp_fd, strs, strlen (strs)) != strlen (strs))
    {
      puts ("could not write test strings into file");
      unlink (temp_filename);
      exit (1);
    }
}

volatile int chk_fail_ok;
volatile int ret;
jmp_buf chk_fail_buf;

static void
handler (int sig)
{
  if (chk_fail_ok)
    {
      chk_fail_ok = 0;
      longjmp (chk_fail_buf, 1);
    }
  else
    _exit (127);
}

char buf[10];
volatile size_t l0;
volatile char *p;
const char *str1 = "JIHGFEDCBA";
const char *str2 = "F";
const char *str3 = "%s%n%s%n";
const char *str4 = "Hello, ";
const char *str5 = "World!\n";
char buf2[10] = "%s";
int num1 = 67;
int num2 = 987654;

#define FAIL() \
  do { printf ("Failure on line %d\n", __LINE__); ret = 1; } while (0)
#define CHK_FAIL_START \
  chk_fail_ok = 1;				\
  if (! setjmp (chk_fail_buf))			\
    {
#define CHK_FAIL_END \
      chk_fail_ok = 0;				\
      FAIL ();					\
    }
#if __USE_FORTIFY_LEVEL >= 2
#define CHK_FAIL2_START CHK_FAIL_START
#define CHK_FAIL2_END CHK_FAIL_END
#else
#define CHK_FAIL2_START
#define CHK_FAIL2_END
#endif

static int
do_test (void)
{
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);

  sigaction (SIGABRT, &sa, NULL);

  /* Avoid all the buffer overflow messages on stderr.  */
  int fd = open (_PATH_DEVNULL, O_WRONLY);
  if (fd == -1)
    close (STDERR_FILENO);
  else
    {
      dup2 (fd, STDERR_FILENO);
      close (fd);
    }
  setenv ("LIBC_FATAL_STDERR_", "1", 1);

  struct A { char buf1[9]; char buf2[1]; } a;

  printf ("Test checking routines at fortify level %d\n",
#ifdef __USE_FORTIFY_LEVEL
	  (int) __USE_FORTIFY_LEVEL
#else
	  0
#endif
	  );

  /* These ops can be done without runtime checking of object size.  */
  memcpy (buf, "abcdefghij", 10);
  memmove (buf + 1, buf, 9);
  if (memcmp (buf, "aabcdefghi", 10))
    FAIL ();

  if (mempcpy (buf + 5, "abcde", 5) != buf + 10 || memcmp (buf, "aabcdabcde", 10))
    FAIL ();

  memset (buf + 8, 'j', 2);
  if (memcmp (buf, "aabcdabcjj", 10))
    FAIL ();

  strcpy (buf + 4, "EDCBA");
  if (memcmp (buf, "aabcEDCBA", 10))
    FAIL ();

  if (stpcpy (buf + 8, "F") != buf + 9 || memcmp (buf, "aabcEDCBF", 10))
    FAIL ();

  strncpy (buf + 6, "X", 4);
  if (memcmp (buf, "aabcEDX\0\0", 10))
    FAIL ();

  if (sprintf (buf + 7, "%s", "67") != 2 || memcmp (buf, "aabcEDX67", 10))
    FAIL ();

  if (snprintf (buf + 7, 3, "%s", "987654") != 6
      || memcmp (buf, "aabcEDX98", 10))
    FAIL ();

  /* These ops need runtime checking, but shouldn't __chk_fail.  */
  memcpy (buf, "abcdefghij", l0 + 10);
  memmove (buf + 1, buf, l0 + 9);
  if (memcmp (buf, "aabcdefghi", 10))
    FAIL ();

  if (mempcpy (buf + 5, "abcde", l0 + 5) != buf + 10 || memcmp (buf, "aabcdabcde", 10))
    FAIL ();

  memset (buf + 8, 'j', l0 + 2);
  if (memcmp (buf, "aabcdabcjj", 10))
    FAIL ();

  strcpy (buf + 4, str1 + 5);
  if (memcmp (buf, "aabcEDCBA", 10))
    FAIL ();

  if (stpcpy (buf + 8, str2) != buf + 9 || memcmp (buf, "aabcEDCBF", 10))
    FAIL ();

  strncpy (buf + 6, "X", l0 + 4);
  if (memcmp (buf, "aabcEDX\0\0", 10))
    FAIL ();

  if (sprintf (buf + 7, "%d", num1) != 2 || memcmp (buf, "aabcEDX67", 10))
    FAIL ();

  if (snprintf (buf + 7, 3, "%d", num2) != 6 || memcmp (buf, "aabcEDX98", 10))
    FAIL ();

  buf[l0 + 8] = '\0';
  strcat (buf, "A");
  if (memcmp (buf, "aabcEDX9A", 10))
    FAIL ();

  buf[l0 + 7] = '\0';
  strncat (buf, "ZYXWV", l0 + 2);
  if (memcmp (buf, "aabcEDXZY", 10))
    FAIL ();

  memcpy (a.buf1, "abcdefghij", l0 + 10);
  memmove (a.buf1 + 1, a.buf1, l0 + 9);
  if (memcmp (a.buf1, "aabcdefghi", 10))
    FAIL ();

  if (mempcpy (a.buf1 + 5, "abcde", l0 + 5) != a.buf1 + 10
      || memcmp (a.buf1, "aabcdabcde", 10))
    FAIL ();

  memset (a.buf1 + 8, 'j', l0 + 2);
  if (memcmp (a.buf1, "aabcdabcjj", 10))
    FAIL ();

#if __USE_FORTIFY_LEVEL < 2
  /* The following tests are supposed to crash with -D_FORTIFY_SOURCE=2
     and sufficient GCC support, as the string operations overflow
     from a.buf1 into a.buf2.  */
  strcpy (a.buf1 + 4, str1 + 5);
  if (memcmp (a.buf1, "aabcEDCBA", 10))
    FAIL ();

  if (stpcpy (a.buf1 + 8, str2) != a.buf1 + 9 || memcmp (a.buf1, "aabcEDCBF", 10))
    FAIL ();

  strncpy (a.buf1 + 6, "X", l0 + 4);
  if (memcmp (a.buf1, "aabcEDX\0\0", 10))
    FAIL ();

  if (sprintf (a.buf1 + 7, "%d", num1) != 2 || memcmp (a.buf1, "aabcEDX67", 10))
    FAIL ();

  if (snprintf (a.buf1 + 7, 3, "%d", num2) != 6
      || memcmp (a.buf1, "aabcEDX98", 10))
    FAIL ();

  a.buf1[l0 + 8] = '\0';
  strcat (a.buf1, "A");
  if (memcmp (a.buf1, "aabcEDX9A", 10))
    FAIL ();

  a.buf1[l0 + 7] = '\0';
  strncat (a.buf1, "ZYXWV", l0 + 2);
  if (memcmp (a.buf1, "aabcEDXZY", 10))
    FAIL ();

#endif

#if __USE_FORTIFY_LEVEL >= 1
  /* Now check if all buffer overflows are caught at runtime.  */

  CHK_FAIL_START
  memcpy (buf + 1, "abcdefghij", l0 + 10);
  CHK_FAIL_END

  CHK_FAIL_START
  memmove (buf + 2, buf + 1, l0 + 9);
  CHK_FAIL_END

  CHK_FAIL_START
  p = mempcpy (buf + 6, "abcde", l0 + 5);
  CHK_FAIL_END

  CHK_FAIL_START
  memset (buf + 9, 'j', l0 + 2);
  CHK_FAIL_END

  CHK_FAIL_START
  strcpy (buf + 5, str1 + 5);
  CHK_FAIL_END

  CHK_FAIL_START
  p = stpcpy (buf + 9, str2);
  CHK_FAIL_END

  CHK_FAIL_START
  strncpy (buf + 7, "X", l0 + 4);
  CHK_FAIL_END

  CHK_FAIL_START
  sprintf (buf + 8, "%d", num1);
  CHK_FAIL_END

  CHK_FAIL_START
  snprintf (buf + 8, l0 + 3, "%d", num2);
  CHK_FAIL_END

  memcpy (buf, str1 + 2, l0 + 9);
  CHK_FAIL_START
  strcat (buf, "AB");
  CHK_FAIL_END

  memcpy (buf, str1 + 3, l0 + 8);
  CHK_FAIL_START
  strncat (buf, "ZYXWV", l0 + 3);
  CHK_FAIL_END

  CHK_FAIL_START
  memcpy (a.buf1 + 1, "abcdefghij", l0 + 10);
  CHK_FAIL_END

  CHK_FAIL_START
  memmove (a.buf1 + 2, a.buf1 + 1, l0 + 9);
  CHK_FAIL_END

  CHK_FAIL_START
  p = mempcpy (a.buf1 + 6, "abcde", l0 + 5);
  CHK_FAIL_END

  CHK_FAIL_START
  memset (a.buf1 + 9, 'j', l0 + 2);
  CHK_FAIL_END

#if __USE_FORTIFY_LEVEL >= 2
# define O 0
#else
# define O 1
#endif

  CHK_FAIL_START
  strcpy (a.buf1 + (O + 4), str1 + 5);
  CHK_FAIL_END

  CHK_FAIL_START
  p = stpcpy (a.buf1 + (O + 8), str2);
  CHK_FAIL_END

  CHK_FAIL_START
  strncpy (a.buf1 + (O + 6), "X", l0 + 4);
  CHK_FAIL_END

  CHK_FAIL_START
  sprintf (a.buf1 + (O + 7), "%d", num1);
  CHK_FAIL_END

  CHK_FAIL_START
  snprintf (a.buf1 + (O + 7), l0 + 3, "%d", num2);
  CHK_FAIL_END

  memcpy (a.buf1, str1 + (3 - O), l0 + 8 + O);
  CHK_FAIL_START
  strcat (a.buf1, "AB");
  CHK_FAIL_END

  memcpy (a.buf1, str1 + (4 - O), l0 + 7 + O);
  CHK_FAIL_START
  strncat (a.buf1, "ZYXWV", l0 + 3);
  CHK_FAIL_END
#endif

  /* Now checks for %n protection.  */

  /* Constant literals passed directly are always ok
     (even with warnings about possible bugs from GCC).  */
  int n1, n2;
  if (sprintf (buf, "%s%n%s%n", str2, &n1, str2, &n2) != 2
      || n1 != 1 || n2 != 2)
    FAIL ();

  /* In this case the format string is not known at compile time,
     but resides in read-only memory, so is ok.  */
  if (snprintf (buf, 4, str3, str2, &n1, str2, &n2) != 2
      || n1 != 1 || n2 != 2)
    FAIL ();

  strcpy (buf2 + 2, "%n%s%n");
  /* When the format string is writable and contains %n,
     with -D_FORTIFY_SOURCE=2 it causes __chk_fail.  */
  CHK_FAIL2_START
  if (sprintf (buf, buf2, str2, &n1, str2, &n1) != 2)
    FAIL ();
  CHK_FAIL2_END

  CHK_FAIL2_START
  if (snprintf (buf, 3, buf2, str2, &n1, str2, &n1) != 2)
    FAIL ();
  CHK_FAIL2_END

  /* But if there is no %n, even writable format string
     should work.  */
  buf2[6] = '\0';
  if (sprintf (buf, buf2 + 4, str2) != 1)
    FAIL ();

  /* Constant literals passed directly are always ok
     (even with warnings about possible bugs from GCC).  */
  if (printf ("%s%n%s%n", str4, &n1, str5, &n2) != 14
      || n1 != 7 || n2 != 14)
    FAIL ();

  /* In this case the format string is not known at compile time,
     but resides in read-only memory, so is ok.  */
  if (printf (str3, str4, &n1, str5, &n2) != 14
      || n1 != 7 || n2 != 14)
    FAIL ();

  strcpy (buf2 + 2, "%n%s%n");
  /* When the format string is writable and contains %n,
     with -D_FORTIFY_SOURCE=2 it causes __chk_fail.  */
  CHK_FAIL2_START
  if (printf (buf2, str4, &n1, str5, &n1) != 14)
    FAIL ();
  CHK_FAIL2_END

  /* But if there is no %n, even writable format string
     should work.  */
  buf2[6] = '\0';
  if (printf (buf2 + 4, str5) != 7)
    FAIL ();

  FILE *fp = stdout;

  /* Constant literals passed directly are always ok
     (even with warnings about possible bugs from GCC).  */
  if (fprintf (fp, "%s%n%s%n", str4, &n1, str5, &n2) != 14
      || n1 != 7 || n2 != 14)
    FAIL ();

  /* In this case the format string is not known at compile time,
     but resides in read-only memory, so is ok.  */
  if (fprintf (fp, str3, str4, &n1, str5, &n2) != 14
      || n1 != 7 || n2 != 14)
    FAIL ();

  strcpy (buf2 + 2, "%n%s%n");
  /* When the format string is writable and contains %n,
     with -D_FORTIFY_SOURCE=2 it causes __chk_fail.  */
  CHK_FAIL2_START
  if (fprintf (fp, buf2, str4, &n1, str5, &n1) != 14)
    FAIL ();
  CHK_FAIL2_END

  /* But if there is no %n, even writable format string
     should work.  */
  buf2[6] = '\0';
  if (fprintf (fp, buf2 + 4, str5) != 7)
    FAIL ();

  if (freopen (temp_filename, "r", stdin) == NULL)
    {
      puts ("could not open temporary file");
      exit (1);
    }

  if (gets (buf) != buf || memcmp (buf, "abcdefgh", 9))
    FAIL ();
  if (gets (buf) != buf || memcmp (buf, "ABCDEFGHI", 10))
    FAIL ();

#if __USE_FORTIFY_LEVEL >= 1
  CHK_FAIL_START
  if (gets (buf) != buf)
    FAIL ();
  CHK_FAIL_END
#endif

  if (freopen (temp_filename, "r", stdin) == NULL)
    {
      puts ("could not open temporary file");
      exit (1);
    }

  if (fseek (stdin, 9 + 10 + 11, SEEK_SET))
    {
      puts ("could not seek in test file");
      exit (1);
    }

#if __USE_FORTIFY_LEVEL >= 1
  CHK_FAIL_START
  if (gets (buf) != buf)
    FAIL ();
  CHK_FAIL_END
#endif

  /* Check whether missing N$ formats are detected.  */
  CHK_FAIL2_START
  printf ("%3$d\n", 1, 2, 3, 4);
  CHK_FAIL2_END

  CHK_FAIL2_START
  fprintf (stdout, "%3$d\n", 1, 2, 3, 4);
  CHK_FAIL2_END

  CHK_FAIL2_START
  sprintf (buf, "%3$d\n", 1, 2, 3, 4);
  CHK_FAIL2_END

  CHK_FAIL2_START
  snprintf (buf, sizeof (buf), "%3$d\n", 1, 2, 3, 4);
  CHK_FAIL2_END

  return ret;
}
