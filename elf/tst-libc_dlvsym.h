/* Compare dlvsym and __libc_dlvsym results.  Common code.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

/* compare_vsyms is the main entry point for these tests.

   Indirectly, It calls __libc_dlvsym (from libc.so; internal
   interface) and dlvsym (from libdl.so; public interface) to compare
   the results for a selected set of symbols in libc.so which
   typically have more than one symbol version.  The two functions are
   implemented by somewhat different code, and this test checks that
   their results are the same.

   The versions are generated to range from GLIBC_2.0 to GLIBC_2.Y,
   with Y being the current __GLIBC_MINOR__ version plus two.  In
   addition, there is a list of special symbol versions of the form
   GLIBC_2.Y.Z, which were used for some releases.

   Comparing the two dlvsym results at versions which do not actually
   exist does not test much, but it will not contribute to false test
   failures, either.  */

#include <array_length.h>
#include <gnu/lib-names.h>
#include <stdbool.h>
#include <stdio.h>
#include <support/check.h>
#include <support/xdlfcn.h>

/* Run consistency check for versioned symbol NAME@VERSION.  NB: We
   may execute in a shared object, so exit on error for proper error
   reporting.  */
static void
compare_vsyms_0 (void *libc_handle, const char *name, const char *version,
                 bool *pfound)
{
  void *dlvsym_address = dlvsym (libc_handle, name, version);
  void *libc_dlvsym_address
    = __libc_dlvsym (libc_handle, name, version);
  if (dlvsym_address != libc_dlvsym_address)
    FAIL_EXIT1 ("%s@%s mismatch: %p != %p",
                name, version, dlvsym_address, libc_dlvsym_address);
  if (dlvsym_address != NULL)
    *pfound = true;
}


/* Run consistency check for versioned symbol NAME at multiple symbol
   version.  */
static void
compare_vsyms_1 (void *libc_handle, const char *name)
{
  bool found = false;

  /* Historic versions which do not follow the usual GLIBC_2.Y
     pattern, to increase test coverage.  Not all architectures have
     those, but probing additional versions does not hurt.  */
  static const char special_versions[][12] =
    {
      "GLIBC_2.1.1",
      "GLIBC_2.1.2",
      "GLIBC_2.1.3",
      "GLIBC_2.1.4",
      "GLIBC_2.2.1",
      "GLIBC_2.2.2",
      "GLIBC_2.2.3",
      "GLIBC_2.2.4",
      "GLIBC_2.2.5",
      "GLIBC_2.2.6",
      "GLIBC_2.3.2",
      "GLIBC_2.3.3",
      "GLIBC_2.3.4",
    };
  for (int i = 0; i < array_length (special_versions); ++i)
    compare_vsyms_0 (libc_handle, name, special_versions[i], &found);

  /* Iterate to an out-of-range version, to cover some unused symbols
     as well.  */
  for (int minor_version = 0; minor_version <= __GLIBC_MINOR__ + 2;
       ++minor_version)
    {
      char version[30];
      snprintf (version, sizeof (version), "GLIBC_%d.%d",
                __GLIBC__, minor_version);
      compare_vsyms_0 (libc_handle, name, version, &found);
    }

  if (!found)
    FAIL_EXIT1 ("symbol %s not found at any version", name);
}

/* Run consistency checks for various symbols which usually have
   multiple versions.  */
static void
compare_vsyms (void)
{
  /* The minor version loop in compare_vsyms_1 needs updating in case
     we ever switch to glibc 3.0.  */
  if (__GLIBC__ != 2)
    FAIL_EXIT1 ("unexpected glibc major version: %d", __GLIBC__);

  /* __libc_dlvsym does not recognize the special RTLD_* handles, so
     obtain an explicit handle for libc.so.  */
  void *libc_handle = xdlopen (LIBC_SO, RTLD_LAZY | RTLD_NOLOAD);

  compare_vsyms_1 (libc_handle, "_sys_errlist");
  compare_vsyms_1 (libc_handle, "_sys_siglist");
  compare_vsyms_1 (libc_handle, "quick_exit");

  xdlclose (libc_handle);
}
