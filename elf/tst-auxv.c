/* Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <elf.h>
#include <link.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <misc/sys/auxv.h>

static int
do_test (int argc, char *argv[])
{
  const char *execfn = (const char *) getauxval (AT_EXECFN);

  if (execfn == NULL)
    {
      printf ("No AT_EXECFN found, test skipped\n");
      return 0;
    }

  if (strcmp (argv[0], execfn) != 0)
    {
      printf ("Mismatch: argv[0]: %s vs. AT_EXECFN: %s\n", argv[0], execfn);
      return 1;
    }

  return 0;
}

#include "../test-skeleton.c"
