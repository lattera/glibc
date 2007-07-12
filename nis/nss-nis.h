/* Copyright (C) 1996, 2004, 2006 Free Software Foundation, Inc.
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

#ifndef _NIS_NSS_NIS_H
#define _NIS_NSS_NIS_H	1

#include <rpcsvc/ypclnt.h>

#include "nsswitch.h"


/* Convert YP error number to NSS error number.  */
extern const enum nss_status __yperr2nss_tab[] attribute_hidden;
extern const unsigned int __yperr2nss_count attribute_hidden;

static inline enum nss_status
yperr2nss (int errval)
{
  if ((unsigned int) errval >= __yperr2nss_count)
    return NSS_STATUS_UNAVAIL;
  return __yperr2nss_tab[(unsigned int) errval];
}


struct response_t
{
  struct response_t *next;
  size_t size;
  char mem[0];
};

typedef struct intern_t
{
  struct response_t *start;
  struct response_t *next;
  size_t offset;
} intern_t;


extern int _nis_saveit (int instatus, char *inkey, int inkeylen, char *inval,
			int invallen, char *indata) attribute_hidden;


#endif /* nis/nss-nis.h */
