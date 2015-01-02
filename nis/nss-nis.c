/* Copyright (C) 1996-2015 Free Software Foundation, Inc.
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

#include "nss-nis.h"
#include "nsswitch.h"


/* Convert YP error number to NSS error number.  */
const enum nss_status __yperr2nss_tab[] =
{
  [YPERR_SUCCESS] = NSS_STATUS_SUCCESS,
  [YPERR_BADARGS] = NSS_STATUS_UNAVAIL,
  [YPERR_RPC]     = NSS_STATUS_UNAVAIL,
  [YPERR_DOMAIN]  = NSS_STATUS_UNAVAIL,
  [YPERR_MAP]     = NSS_STATUS_UNAVAIL,
  [YPERR_KEY]     = NSS_STATUS_NOTFOUND,
  [YPERR_YPERR]   = NSS_STATUS_UNAVAIL,
  [YPERR_RESRC]   = NSS_STATUS_TRYAGAIN,
  [YPERR_NOMORE]  = NSS_STATUS_NOTFOUND,
  [YPERR_PMAP]    = NSS_STATUS_UNAVAIL,
  [YPERR_YPBIND]  = NSS_STATUS_UNAVAIL,
  [YPERR_YPSERV]  = NSS_STATUS_UNAVAIL,
  [YPERR_NODOM]   = NSS_STATUS_UNAVAIL,
  [YPERR_BADDB]   = NSS_STATUS_UNAVAIL,
  [YPERR_VERS]    = NSS_STATUS_UNAVAIL,
  [YPERR_ACCESS]  = NSS_STATUS_UNAVAIL,
  [YPERR_BUSY]    = NSS_STATUS_TRYAGAIN
};
const unsigned int __yperr2nss_count = (sizeof (__yperr2nss_tab)
				        / sizeof (__yperr2nss_tab[0]));
