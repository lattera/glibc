/* Optional code to distinguish library flavours.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

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

#ifndef _DL_LIBRECON_H
#define _DL_LIBRECON_H	1

/* Recognizing extra environment variables.  */
#define EXTRA_LD_ENVVARS \
  case 13:								      \
    if (memcmp (&envline[3], "ASSUME_KERNEL", 13) == 0)			      \
      {									      \
	unsigned long int i, j, osversion = 0;				      \
	char *p = &envline[17], *q;					      \
									      \
	for (i = 0; i < 3; i++, p = q + 1)				      \
	  {								      \
	    j = __strtoul_internal (p, &q, 0, 0);			      \
	    if (j >= 255 || p == q || (i < 2 && *q && *q != '.'))	      \
	      {								      \
		osversion = 0;						      \
		break;							      \
	      }								      \
	    osversion |= j << (16 - 8 * i);				      \
	    if (!*q)							      \
	      break;							      \
	  }								      \
	if (osversion)							      \
	  _dl_osversion = osversion;					      \
	break;								      \
      }

#endif /* dl-librecon.h */
