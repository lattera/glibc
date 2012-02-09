/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <rpcsvc/nis.h>

#include "nss-nisplus.h"
#include "nsswitch.h"


/* Convert NIS+ error number to NSS error number.  */
const enum nss_status __niserr2nss_tab[] =
{
  [NIS_SUCCESS] = NSS_STATUS_SUCCESS,
  [NIS_S_SUCCESS] = NSS_STATUS_SUCCESS,
  [NIS_NOTFOUND] = NSS_STATUS_NOTFOUND,
  [NIS_S_NOTFOUND] = NSS_STATUS_NOTFOUND,
  [NIS_CACHEEXPIRED] = NSS_STATUS_UNAVAIL,
  [NIS_NAMEUNREACHABLE] = NSS_STATUS_UNAVAIL,
  [NIS_UNKNOWNOBJ] = NSS_STATUS_NOTFOUND,
  [NIS_TRYAGAIN] = NSS_STATUS_TRYAGAIN,
  [NIS_SYSTEMERROR] = NSS_STATUS_UNAVAIL,
  [NIS_CHAINBROKEN] = NSS_STATUS_UNAVAIL,
  [NIS_PERMISSION] = NSS_STATUS_UNAVAIL,
  [NIS_NOTOWNER] = NSS_STATUS_UNAVAIL,
  [NIS_NOT_ME] = NSS_STATUS_UNAVAIL,
  [NIS_NOMEMORY] = NSS_STATUS_TRYAGAIN,
  [NIS_NAMEEXISTS] = NSS_STATUS_UNAVAIL,
  [NIS_NOTMASTER] = NSS_STATUS_UNAVAIL,
  [NIS_INVALIDOBJ] = NSS_STATUS_UNAVAIL,
  [NIS_BADNAME] = NSS_STATUS_UNAVAIL,
  [NIS_NOCALLBACK] = NSS_STATUS_UNAVAIL,
  [NIS_CBRESULTS] = NSS_STATUS_UNAVAIL,
  [NIS_NOSUCHNAME] = NSS_STATUS_NOTFOUND,
  [NIS_NOTUNIQUE] = NSS_STATUS_UNAVAIL,
  [NIS_IBMODERROR] = NSS_STATUS_UNAVAIL,
  [NIS_NOSUCHTABLE] = NSS_STATUS_UNAVAIL,
  [NIS_TYPEMISMATCH] = NSS_STATUS_UNAVAIL,
  [NIS_LINKNAMEERROR] = NSS_STATUS_UNAVAIL,
  [NIS_PARTIAL] = NSS_STATUS_NOTFOUND,
  [NIS_TOOMANYATTRS] = NSS_STATUS_UNAVAIL,
  [NIS_RPCERROR] = NSS_STATUS_UNAVAIL,
  [NIS_BADATTRIBUTE] = NSS_STATUS_UNAVAIL,
  [NIS_NOTSEARCHABLE] = NSS_STATUS_UNAVAIL,
  [NIS_CBERROR] = NSS_STATUS_UNAVAIL,
  [NIS_FOREIGNNS] = NSS_STATUS_UNAVAIL,
  [NIS_BADOBJECT] = NSS_STATUS_UNAVAIL,
  [NIS_NOTSAMEOBJ] = NSS_STATUS_UNAVAIL,
  [NIS_MODFAIL] = NSS_STATUS_UNAVAIL,
  [NIS_BADREQUEST] = NSS_STATUS_UNAVAIL,
  [NIS_NOTEMPTY] = NSS_STATUS_UNAVAIL,
  [NIS_COLDSTART_ERR] = NSS_STATUS_UNAVAIL,
  [NIS_RESYNC] = NSS_STATUS_UNAVAIL,
  [NIS_FAIL] = NSS_STATUS_UNAVAIL,
  [NIS_UNAVAIL] = NSS_STATUS_UNAVAIL,
  [NIS_RES2BIG] = NSS_STATUS_UNAVAIL,
  [NIS_SRVAUTH] = NSS_STATUS_UNAVAIL,
  [NIS_CLNTAUTH] = NSS_STATUS_UNAVAIL,
  [NIS_NOFILESPACE] = NSS_STATUS_UNAVAIL,
  [NIS_NOPROC] = NSS_STATUS_UNAVAIL,
  [NIS_DUMPLATER] = NSS_STATUS_UNAVAIL
};
const unsigned int __niserr2nss_count = (sizeof (__niserr2nss_tab)
					 / sizeof (__niserr2nss_tab[0]));
