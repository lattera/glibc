/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@uni-paderborn.de>, 1997.

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

#ifndef _NIS_NSS_NISPLUS_H
#define _NIS_NSS_NISPLUS_H	1

#include <rpcsvc/nis.h>

#include "nsswitch.h"


/* Convert NIS+ error number to NSS error number.  */
extern const enum nss_status __niserr2nss_tab[] attribute_hidden;
extern const unsigned int __niserr2nss_count attribute_hidden;

static enum nss_status
__attribute__ ((unused))
niserr2nss (int errval)
{
  if ((unsigned int) errval >= __niserr2nss_count)
    return NSS_STATUS_UNAVAIL;
  return __niserr2nss_tab[(unsigned int) errval];
}

#endif /* nis/nss-nisplus.h */
