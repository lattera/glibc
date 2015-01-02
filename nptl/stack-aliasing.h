/* Define macros for stack address aliasing issues for NPTL.  Stub version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

/* This is a number of bytes (less than a page) by which to "color" the
   starting stack address of new threads.  This number is multiplied by the
   number of threads created so far and then truncated modulo page size,
   to get a roughly even distribution of values for different threads.  */
#define COLORING_INCREMENT      0

/* This is a number of bytes that is an alignment that should be avoided
   when choosing the exact size of a new thread's stack.  If the size
   chosen is aligned to this, an extra page will be added to render the
   size off-aligned.  */
#define MULTI_PAGE_ALIASING     0
