/* Constant values for the uname function to return.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

/* This file must define these macros with string values:
        UNAME_SYSNAME
        UNAME_RELEASE
        UNAME_VERSION
        UNAME_MACHINE
*/

#ifndef UNAME_MACHINE
# error "sysdeps/MACHINE/nacl/uname-values.h should define UNAME_MACHINE"
#endif

#define UNAME_SYSNAME "NaCl"
#define UNAME_RELEASE "unknown"
#define UNAME_VERSION "unknown"
