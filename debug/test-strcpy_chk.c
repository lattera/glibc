/* Test and measure __strcpy_chk functions.
   Copyright (C) 1999, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jakub Jelinek <jakub@redhat.com>, 1999.

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

#ifndef STRCPY_RESULT
# define STRCPY_RESULT(dst, len) dst
# define TEST_MAIN
# include "../string/test-string.h"

extern void __attribute__ ((noreturn)) __chk_fail (void);
char *simple_strcpy_chk (char *, const char *, size_t);
extern char *normal_strcpy (char *, const char *, size_t)
  __asm ("strcpy");
extern char *__strcpy_chk (char *, const char *, size_t);

IMPL (simple_strcpy_chk, 0)
IMPL (normal_strcpy, 1)
IMPL (__strcpy_chk, 2)

char *
simple_strcpy_chk (char *dst, const char *src, size_t len)
{
  char *ret = dst;
  if (! len)
    __chk_fail ();
  while ((*dst++ = *src++) != '\0')
    if (--len == 0)
      __chk_fail ();
  return ret;
}
#endif

#include <fcntl.h>
#include <paths.h>
#include <setjmp.h>
#include <signal.h>

volatile int chk_fail_ok;
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

typedef char *(*proto_t) (char *, const char *, size_t);

static void
do_one_test (impl_t *impl, char *dst, const char *src,
	     size_t len, size_t dlen)
{
  char *res;
  if (dlen <= len)
    {
      if (impl->test == 1)
	return;

      chk_fail_ok = 1;
      if (setjmp (chk_fail_buf) == 0)
	{
	  res = CALL (impl, dst, src, dlen);
	  printf ("*** Function %s (%zd; %zd) did not __chk_fail\n",
		  impl->name, len, dlen);
	  chk_fail_ok = 0;
	  ret = 1;
	}
      return;
    }
  else
    res = CALL (impl, dst, src, dlen);

  if (res != STRCPY_RESULT (dst, len))
    {
      printf ("Wrong result in function %s %p %p\n", impl->name,
	      res, STRCPY_RESULT (dst, len));
      ret = 1;
      return;
    }

  if (strcmp (dst, src) != 0)
    {
      printf ("Wrong result in function %s dst \"%s\" src \"%s\"\n",
	      impl->name, dst, src);
      ret = 1;
      return;
    }

  if (HP_TIMING_AVAIL)
    {
      hp_timing_t start __attribute ((unused));
      hp_timing_t stop __attribute ((unused));;
      hp_timing_t best_time = ~ (hp_timing_t) 0;
      size_t i;

      for (i = 0; i < 32; ++i)
	{
	  HP_TIMING_NOW (start);
	  CALL (impl, dst, src, dlen);
	  HP_TIMING_NOW (stop);
	  HP_TIMING_BEST (best_time, start, stop);
	}

      printf ("\t%zd", (size_t) best_time);
    }
}

static void
do_test (size_t align1, size_t align2, size_t len, size_t dlen, int max_char)
{
  size_t i;
  char *s1, *s2;

  align1 &= 7;
  if (align1 + len >= page_size)
    return;

  align2 &= 7;
  if (align2 + len >= page_size)
    return;

  s1 = buf1 + align1;
  s2 = buf2 + align2;

  for (i = 0; i < len; i++)
    s1[i] = 32 + 23 * i % (max_char - 32);
  s1[len] = 0;

  if (HP_TIMING_AVAIL && dlen > len)
    printf ("Length %4zd, alignment %2zd/%2zd:", len, align1, align2);

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s2, s1, len, dlen);

  if (HP_TIMING_AVAIL && dlen > len)
    putchar ('\n');
}

static void
do_random_tests (void)
{
  size_t i, j, n, align1, align2, len, dlen;
  unsigned char *p1 = buf1 + page_size - 512;
  unsigned char *p2 = buf2 + page_size - 512;
  unsigned char *res;

  for (n = 0; n < ITERATIONS; n++)
    {
      align1 = random () & 31;
      if (random () & 1)
	align2 = random () & 31;
      else
	align2 = align1 + (random () & 24);
      len = random () & 511;
      j = align1;
      if (align2 > j)
	j = align2;
      if (len + j >= 511)
	len = 510 - j - (random () & 7);
      j = len + align1 + 64;
      if (j > 512)
	j = 512;
      for (i = 0; i < j; i++)
	{
	  if (i == len + align1)
	    p1[i] = 0;
	  else
	    {
	      p1[i] = random () & 255;
	      if (i >= align1 && i < len + align1 && !p1[i])
		p1[i] = (random () & 127) + 3;
	    }
	}

      switch (random () & 7)
	{
	case 0:
	  dlen = len - (random () & 31);
	  if (dlen > len)
	    dlen = len;
	  break;
	case 1:
	  dlen = (size_t) -1;
	  break;
	case 2:
	  dlen = len + 1 + (random () & 65535);
	  break;
	case 3:
	  dlen = len + 1 + (random () & 255);
	  break;
	case 4:
	  dlen = len + 1 + (random () & 31);
	  break;
	case 5:
	  dlen = len + 1 + (random () & 7);
	  break;
	case 6:
	  dlen = len + 1 + (random () & 3);
	  break;
	default:
	  dlen = len + 1;
	  break;
	}

      FOR_EACH_IMPL (impl, 1)
	{
	  if (dlen <= len)
	    {
	      if (impl->test != 1)
		{
		  chk_fail_ok = 1;
		  if (setjmp (chk_fail_buf) == 0)
		    {
		      res = CALL (impl, p2 + align2, p1 + align1, dlen);
		      printf ("Iteration %zd - did not __chk_fail\n", n);
		      chk_fail_ok = 0;
		      ret = 1;
		    }
		}
	      continue;
	    }
	  memset (p2 - 64, '\1', 512 + 64);
	  res = CALL (impl, p2 + align2, p1 + align1, dlen);
	  if (res != STRCPY_RESULT (p2 + align2, len))
	    {
	      printf ("\
Iteration %zd - wrong result in function %s (%zd, %zd, %zd) %p != %p\n",
		      n, impl->name, align1, align2, len, res,
		      STRCPY_RESULT (p2 + align2, len));
	      ret = 1;
	    }
	  for (j = 0; j < align2 + 64; ++j)
	    {
	      if (p2[j - 64] != '\1')
		{
		  printf ("\
Iteration %zd - garbage before, %s (%zd, %zd, %zd)\n",
			  n, impl->name, align1, align2, len);
		  ret = 1;
		  break;
		}
	    }
	  for (j = align2 + len + 1; j < 512; ++j)
	    {
	      if (p2[j] != '\1')
		{
		  printf ("\
Iteration %zd - garbage after, %s (%zd, %zd, %zd)\n",
			  n, impl->name, align1, align2, len);
		  ret = 1;
		  break;
		}
	    }
	  if (memcmp (p1 + align1, p2 + align2, len + 1))
	    {
	      printf ("\
Iteration %zd - different strings, %s (%zd, %zd, %zd)\n",
		      n, impl->name, align1, align2, len);
	      ret = 1;
	    }
	}
    }
}

int
test_main (void)
{
  size_t i;

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

  test_init ();

  printf ("%23s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 0; i < 16; ++i)
    {
      do_test (0, 0, i, i + 1, 127);
      do_test (0, 0, i, i + 1, 255);
      do_test (0, i, i, i + 1, 127);
      do_test (i, 0, i, i + 1, 255);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 0, 8 << i, (8 << i) + 1, 127);
      do_test (8 - i, 2 * i, (8 << i), (8 << i) + 1, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 2 * i, (8 << i), (8 << i) + 1, 127);
      do_test (2 * i, i, (8 << i), (8 << i) + 1, 255);
      do_test (i, i, (8 << i), (8 << i) + 1, 127);
      do_test (i, i, (8 << i), (8 << i) + 1, 255);
    }

  for (i = 0; i < 16; ++i)
    {
      do_test (0, 0, i, i + 256, 127);
      do_test (0, 0, i, i + 256, 255);
      do_test (0, i, i, i + 256, 127);
      do_test (i, 0, i, i + 256, 255);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 0, 8 << i, (8 << i) + 256, 127);
      do_test (8 - i, 2 * i, (8 << i), (8 << i) + 256, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 2 * i, (8 << i), (8 << i) + 256, 127);
      do_test (2 * i, i, (8 << i), (8 << i) + 256, 255);
      do_test (i, i, (8 << i), (8 << i) + 256, 127);
      do_test (i, i, (8 << i), (8 << i) + 256, 255);
    }

  for (i = 0; i < 16; ++i)
    {
      do_test (0, 0, i, i, 127);
      do_test (0, 0, i, i + 2, 255);
      do_test (0, i, i, i + 3, 127);
      do_test (i, 0, i, i + 4, 255);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 0, 8 << i, (8 << i) - 15, 127);
      do_test (8 - i, 2 * i, (8 << i), (8 << i) + 5, 127);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 2 * i, (8 << i), (8 << i) + i, 127);
      do_test (2 * i, i, (8 << i), (8 << i) + (i - 1), 255);
      do_test (i, i, (8 << i), (8 << i) + i + 2, 127);
      do_test (i, i, (8 << i), (8 << i) + i + 3, 255);
    }

  do_random_tests ();
  return ret;
}

#include "../test-skeleton.c"
