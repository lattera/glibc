/* Test CPU feature data.
   This file is part of the GNU C Library.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.

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

#include <cpu-features.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *cpu_flags;

/* Search for flags in /proc/cpuinfo and store line
   in cpu_flags.  */
void
get_cpuinfo (void)
{
  FILE *f;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  f = fopen ("/proc/cpuinfo", "r");
  if (f == NULL)
    {
      printf ("cannot open /proc/cpuinfo\n");
      exit (1);
    }

  while ((read = getline (&line, &len, f)) != -1)
    {
      if (strncmp (line, "flags", 5) == 0)
       {
         cpu_flags = strdup (line);
         break;
       }
    }
  fclose (f);
  free (line);
}

int
check_proc (const char *proc_name, int flag, const char *name)
{
  int found = 0;

  printf ("Checking %s:\n", name);
  printf ("  init-arch %d\n", flag);
  if (strstr (cpu_flags, proc_name) != NULL)
    found = 1;
  printf ("  cpuinfo (%s) %d\n", proc_name, found);

  if (found != flag)
    printf (" *** failure ***\n");

  return (found != flag);
}

static int
do_test (int argc, char **argv)
{
  int fails;

  get_cpuinfo ();
  fails = check_proc ("avx", HAS_ARCH_FEATURE (AVX_Usable),
		      "HAS_ARCH_FEATURE (AVX_Usable)");
  fails += check_proc ("fma4", HAS_ARCH_FEATURE (FMA4_Usable),
		       "HAS_ARCH_FEATURE (FMA4_Usable)");
  fails += check_proc ("sse4_2", HAS_CPU_FEATURE (SSE4_2),
		       "HAS_CPU_FEATURE (SSE4_2)");
  fails += check_proc ("sse4_1", HAS_CPU_FEATURE (SSE4_1)
		       , "HAS_CPU_FEATURE (SSE4_1)");
  fails += check_proc ("ssse3", HAS_CPU_FEATURE (SSSE3),
		       "HAS_CPU_FEATURE (SSSE3)");
  fails += check_proc ("popcnt", HAS_CPU_FEATURE (POPCOUNT),
		       "HAS_CPU_FEATURE (POPCOUNT)");

  printf ("%d differences between /proc/cpuinfo and glibc code.\n", fails);

  return (fails != 0);
}

#include "../../../test-skeleton.c"
