/* Initialization code run first thing by the ELF startup code.  For i386/Unix.
   Copyright (C) 1995-2012 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <unistd.h>

extern void __libc_init (int, char **, char **);
#ifdef USE_NONOPTION_FLAGS
extern void __getopt_clean_environment (char **);
#endif
extern void __libc_global_ctors (void);

int __libc_multiple_libcs attribute_hidden = 1;

static void
init (int *data)
{
  int argc = *data;
  char **argv = (void *) (data + 1);
  char **envp = &argv[argc + 1];

  __environ = envp;
  __libc_init (argc, argv, envp);

#ifdef USE_NONOPTION_FLAGS
  /* This is a hack to make the special getopt in GNU libc working.  */
  __getopt_clean_environment (envp);
#endif

  /* Initialize ctype data.  */
  __ctype_init ();
}

#ifdef SHARED
/* This function is called to initialize the shared C library.
   It is called just before the user _start code from i386/elf/start.S,
   with the stack set up as that code gets it.  */

/* NOTE!  The linker notices the magical name `_init' and sets the DT_INIT
   pointer in the dynamic section based solely on that.  It is convention
   for this function to be in the `.init' section, but the symbol name is
   the only thing that really matters!!  */
/*void _init (int argc, ...) __attribute__ ((unused, section (".init")));*/

void
_init (int argc, ...)
{
  init (&argc);

#ifndef NO_CTORS_DTORS_SECTIONS
  __libc_global_ctors ();
#endif
}
#endif


void
__libc_init_first (int argc __attribute__ ((unused)), ...)
{
#ifndef SHARED
  init (&argc);
#endif
}
