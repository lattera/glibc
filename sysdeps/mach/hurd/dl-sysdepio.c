/* Operating system support for run-time dynamic linker.  Hurd version.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <hurd.h>
#include <link.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <assert.h>
#include <sysdep.h>
#include <mach/mig_support.h>
#include "hurdstartup.h"
#include <mach/host_info.h>
#include "../stdio-common/_itoa.h"
#include <hurd/auth.h>
#include <hurd/term.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>

void
_dl_sysdep_fatal (const char *msg, ...)
{
  va_list ap;

  va_start (ap, msg);
  do
    {
      size_t len = strlen (msg);
      mach_msg_type_number_t nwrote;
      do
	{
	  if (__io_write (_hurd_init_dtable[2], msg, len, -1, &nwrote))
	    break;
	  len -= nwrote;
	  msg += nwrote;
	} while (nwrote > 0);
      msg = va_arg (ap, const char *);
    } while (msg);
  va_end (ap);

  _exit (127);
}


void
_dl_sysdep_error (const char *msg, ...)
{
  va_list ap;

  va_start (ap, msg);
  do
    {
      size_t len = strlen (msg);
      mach_msg_type_number_t nwrote;
      do
	{
	  if (__io_write (_hurd_init_dtable[2], msg, len, -1, &nwrote))
	    break;
	  len -= nwrote;
	  msg += nwrote;
	} while (nwrote > 0);
      msg = va_arg (ap, const char *);
    } while (msg);
  va_end (ap);
}


void
_dl_sysdep_message (const char *msg, ...)
{
  va_list ap;

  va_start (ap, msg);
  do
    {
      size_t len = strlen (msg);
      mach_msg_type_number_t nwrote;
      do
	{
	  if (__io_write (_hurd_init_dtable[1], msg, len, -1, &nwrote))
	    break;
	  len -= nwrote;
	  msg += nwrote;
	} while (nwrote > 0);
      msg = va_arg (ap, const char *);
    } while (msg);
  va_end (ap);
}
