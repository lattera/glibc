/* libintl.h -- Message catalogs for internationalization.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _LIBINTL_H
#define	_LIBINTL_H	1

#include <locale.h>


/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
extern char *gettext __P ((const char *__msgid));

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
extern char *dgettext __P ((const char *__domainname, const char *__msgid));

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
extern char *__dcgettext __P ((const char *__domainname, const char *__msgid,
			       int __category));
extern char *dcgettext __P ((const char *__domainname, const char *__msgid,
			     int __category));


/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
extern char *textdomain __P ((const char *__domainname));

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
extern char *bindtextdomain __P ((const char *__domainname,
				  const char *__dirname));


#if 1 /* XXX stub for the moment */
#define gettext(msgid) (msgid)
#define textdomain(domain) (void)(domain)
#else
#define gettext(msgid)			__gettext (msgid)
#define __gettext(msgid)		__dgettext (NULL, (msgid))

#define	dgettext(domainname, msgid)	__dgettext (domainname, msgid)
#define	__dgettext(domainname, msgid)	\
  __dcgettext (NULL, (msgid), LC_MESSAGES)

#ifdef __GNUC__
#define	__dcgettext(domainname, msgid, category)			      \
  (__extension__							      \
   ({									      \
     static char *__translation__;					      \
     if (! __translation__)						      \
       __translation__ = (__dcgettext) ((domainname), (msgid), (category));   \
     __translation__;							      \
    }))
#endif
#endif

#endif	/* libintl.h */
