/* Dummy nop functions to elicit link-time warnings.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <sys/cdefs.h>

void __nop (void)
{
}

/* Don't insert any other #include's before this #undef!  */

#undef __warndecl
#define __warndecl(name, msg) \
  strong_alias (__nop, name) link_warning (name, msg)

#undef	__USE_FORTIFY_LEVEL
#define __USE_FORTIFY_LEVEL 99

/* Following here we need an #include for each public header file
   that uses __warndecl.  */

#include <string.h>
