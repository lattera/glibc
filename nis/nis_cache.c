/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>
#include <rpcsvc/nis_cache.h>
#include <bits/libc-lock.h>

#include "nis_intern.h"

/* XXX Only dummy functions in the moment. The real implementation
       will follow, if we have a working nis_cachemgr */
directory_obj *
__cache_search (const_nis_name name)
{
  return NULL;
}

nis_error 
__cache_add (fd_result *fd)
{
  return NIS_FAIL;
}
