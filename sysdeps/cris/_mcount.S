/* Machine-specific calling sequence for `mcount' profiling function for CRIS.
   Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* FIXME: This isn't implemented yet.  This is just a machine-specific
   stub.  Perhaps a real implementation can make use of it.  */

#include <sysdep.h>

#undef CALL_MCOUNT
#define CALL_MCOUNT

ENTRY (_mcount)
	SETUP_PIC
	/* Sorry, isn't implemented yet.
	   Can't call abort; that will recurse.  Force SEGV instead.  */
	test.d [99]
1:
	ba 1b
	nop
END (_mcount)
