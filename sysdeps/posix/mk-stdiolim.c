/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <posix1_lim.h>

int
main()
{
  /* These values correspond to the code in sysdeps/posix/tempname.c.
     Change the values here if you change that code.  */
  printf("#define L_tmpnam %u\n", sizeof("/usr/tmp/") + 8);
  printf("#define TMP_MAX %u\n", 62 * 62 * 62);

  puts  ("#ifdef __USE_POSIX");
  printf("#define L_ctermid %u\n", sizeof("/dev/tty"));
  printf("#define L_cuserid 9\n");
  puts  ("#endif");

  /* POSIX does not require that OPEN_MAX and PATH_MAX be defined, so
     <local_lim.h> will not define them if they are run-time variant (which
     is the case in the Hurd).  ANSI still requires that FOPEN_MAX and
     FILENAME_MAX be defined, however.  */

  printf("#define FOPEN_MAX %u\n", 
#ifdef	OPEN_MAX

	 OPEN_MAX
#else
	 /* This is the minimum number of files that the implementation
	    guarantees can be open simultaneously.  OPEN_MAX not being
	    defined means the maximum is run-time variant; but POSIX.1
	    requires that it never be less than _POSIX_OPEN_MAX, so that is
	    a good minimum to use.  */
	 _POSIX_OPEN_MAX
#endif

	 );

  printf("#define FILENAME_MAX %u\n", 
#ifdef	PATH_MAX
	 PATH_MAX
#else
	 /* This is supposed to be the size needed to hold the longest file
	    name string the implementation guarantees can be opened.
	    PATH_MAX not being defined means the actual limit on the length
	    of a file name is runtime-variant (or it is unlimited).  ANSI
	    says in such a case FILENAME_MAX should be a good size to
	    allocate for a file name string.  POSIX.1 guarantees that a
	    file name up to _POSIX_PATH_MAX chars long can be opened, so
	    this value must be at least that.  */
	    1024		/* _POSIX_PATH_MAX is 255.  */
#endif
	 );

  exit(0);
}
