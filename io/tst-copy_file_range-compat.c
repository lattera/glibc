/* Test the fallback implementation of copy_file_range.
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

/* Get the declaration of the official copy_of_range function.  */
#include <unistd.h>

/* Compile a local version of copy_file_range.  */
#define COPY_FILE_RANGE_DECL static
#define COPY_FILE_RANGE copy_file_range_compat
#include <io/copy_file_range-compat.c>

/* Re-use the test, but run it against copy_file_range_compat defined
   above.  */
#define copy_file_range copy_file_range_compat
#include "tst-copy_file_range.c"
