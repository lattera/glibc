/* utmpdump - dump utmp-like files.
   Copyright (C) 1997, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

static void
print_entry (struct utmp *up)
{
  /* Mixed 32-/64-bit systems may have timeval structs of different sixe
     but need struct utmp to be the same size.  So in 64-bit up->ut_tv may
     not be a timeval but a struct of __int32_t's.  This would cause a compile
     time warning and a formating error when 32-bit int is passed where
     a 64-bit long is expected. So copy up->up_tv to a temporary timeval.
     This is 32-/64-bit agnostic and expands the timeval fields to the
     expected size as needed. */
  struct timeval temp_tv;
  temp_tv.tv_sec = up->ut_tv.tv_sec;
  temp_tv.tv_usec = up->ut_tv.tv_usec;

  (printf) (
	    /* The format string.  */
#if _HAVE_UT_TYPE
	    "[%d] "
#endif
#if _HAVE_UT_PID
	    "[%05d] "
#endif
#if _HAVE_UT_ID
	    "[%-4.4s] "
#endif
	    "[%-8.8s] [%-12.12s]"
#if _HAVE_UT_HOST
	    " [%-16.16s]"
#endif
	    " [%-15.15s]"
#if _HAVE_UT_TV
	    " [%ld]"
#endif
	    "\n"
	    /* The arguments.  */
#if _HAVE_UT_TYPE
	    , up->ut_type
#endif
#if _HAVE_UT_PID
	    , up->ut_pid
#endif
#if _HAVE_UT_ID
	    , up->ut_id
#endif
	    , up->ut_user, up->ut_line
#if _HAVE_UT_HOST
	    , up->ut_host
#endif
#if _HAVE_UT_TV
	    , 4 + ctime (&temp_tv.tv_sec)
	    , (long int) temp_tv.tv_usec
#else
	    , 4 + ctime (&up->ut_time)
#endif
	   );
}

int
main (int argc, char *argv[])
{
  struct utmp *up;

  if (argc > 1)
    utmpname (argv[1]);

  setutent ();

  while ((up = getutent ()))
    print_entry (up);

  endutent ();

  return EXIT_SUCCESS;
}
