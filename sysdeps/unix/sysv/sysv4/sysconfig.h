/* `__sysconfig' NAME values.
   Copyright (C) 1993, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H

#define _CONFIG_OPEN_FILES 4	/* process limit on open files */
#define _CONFIG_PAGESIZE 6	/* MMU page size */
#define _CONFIG_CLK_TCK 7	/* all times are in CLK_TCKths of a second */

#endif
