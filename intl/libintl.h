/* Message catalogs for internationalization.
   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1995.
   This file is derived from the file libgettext.h in the GNU gettext package.

   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.

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

#ifndef _LIBINTL_H
#define _LIBINTL_H	1

#include <features.h>

/* We define an additional symbol to signal that we use the GNU
   implementation of gettext.  */
#define __USE_GNU_GETTEXT 1

__BEGIN_DECLS

/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
extern char *gettext (__const char *__msgid) __THROW;

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
extern char *dgettext (__const char *__domainname,
		       __const char *__msgid) __THROW;
extern char *__dgettext (__const char *__domainname,
			 __const char *__msgid) __THROW;

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
extern char *dcgettext (__const char *__domainname,
			__const char *__msgid, int __category) __THROW;
extern char *__dcgettext (__const char *__domainname,
			  __const char *__msgid, int __category) __THROW;


/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
extern char *textdomain (__const char *__domainname) __THROW;

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
extern char *bindtextdomain (__const char *__domainname,
			     __const char *__dirname) __THROW;


/* Optimized version of the function above.  */
#if defined __OPTIMIZE__

/* We need NULL for `gettext'.  */
# define __need_NULL
# include <stddef.h>

/* We need LC_MESSAGES for `dgettext'.  */
# include <locale.h>

/* These must be macros.  Inlined functions are useless because the
   `__builtin_constant_p' predicate in dcgettext would always return
   false.  */

# define gettext(msgid) dgettext (NULL, msgid)

# define dgettext(domainname, msgid)					      \
  dcgettext (domainname, msgid, LC_MESSAGES)

# if __GNUC_PREREQ (2,7)
/* Variable defined in loadmsgcat.c which gets incremented every time a
   new catalog is loaded.  */
extern int _nl_msg_cat_cntr;

#  define dcgettext(domainname, msgid, category)			      \
  (__extension__							      \
   ({									      \
     char *__result;							      \
     if (__builtin_constant_p (msgid))					      \
       {								      \
	 static char *__translation__;					      \
	 static int __catalog_counter__;				      \
	 if (! __translation__ || __catalog_counter__ != _nl_msg_cat_cntr)    \
	   {								      \
	     __translation__ =						      \
	       __dcgettext ((domainname), (msgid), (category));		      \
	     __catalog_counter__ = _nl_msg_cat_cntr;			      \
	   }								      \
	 __result = __translation__;					      \
       }								      \
     else								      \
       __result = __dcgettext ((domainname), (msgid), (category));	      \
     __result;								      \
    }))
# endif
#endif /* Optimizing. */


__END_DECLS

#endif /* libintl.h */
