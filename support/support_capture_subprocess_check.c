/* Verify capture output from a subprocess.
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
#include <support/capture_subprocess.h>
#include <support/check.h>

static void
print_context (const char *context, bool *failed)
{
  if (*failed)
    /* Do not duplicate message.  */
    return;
  support_record_failure ();
  printf ("error: subprocess failed: %s\n", context);
}

void
support_capture_subprocess_check (struct support_capture_subprocess *proc,
                                  const char *context, int status,
                                  int allowed)
{
  TEST_VERIFY ((allowed & sc_allow_none)
               || (allowed & sc_allow_stdout)
               || (allowed & sc_allow_stderr));
  TEST_VERIFY (!((allowed & sc_allow_none)
                 && ((allowed & sc_allow_stdout)
                     || (allowed & sc_allow_stderr))));

  bool failed = false;
  if (proc->status != status)
    {
      print_context (context, &failed);
      printf ("error:   expected exit status: %d\n", status);
      printf ("error:   actual exit status:   %d\n", proc->status);
    }
  if (!(allowed & sc_allow_stdout) && proc->out.length != 0)
    {
      print_context (context, &failed);
      printf ("error:   unexpected output from subprocess\n");
      fwrite (proc->out.buffer, proc->out.length, 1, stdout);
      puts ("\n");
    }
  if (!(allowed & sc_allow_stderr) && proc->err.length != 0)
    {
      print_context (context, &failed);
      printf ("error:   unexpected error output from subprocess\n");
      fwrite (proc->err.buffer, proc->err.length, 1, stdout);
      puts ("\n");
    }
}
