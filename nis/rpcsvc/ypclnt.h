/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1996.

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


#ifndef	__RPCSVC_YPCLNT_H__
#define	__RPCSVC_YPCLNT_H__

#include <features.h>

/* some defines */
#define YPERR_SUCCESS 0                 /* There is no error */
#define	YPERR_BADARGS 1			/* Args to function are bad */
#define	YPERR_RPC 2			/* RPC failure */
#define	YPERR_DOMAIN 3			/* Can't bind to a server with this domain */
#define	YPERR_MAP 4			/* No such map in server's domain */
#define	YPERR_KEY 5			/* No such key in map */
#define	YPERR_YPERR 6			/* Internal yp server or client error */
#define	YPERR_RESRC 7			/* Local resource allocation failure */
#define	YPERR_NOMORE 8			/* No more records in map database */
#define	YPERR_PMAP 9			/* Can't communicate with portmapper */
#define	YPERR_YPBIND 10			/* Can't communicate with ypbind */
#define	YPERR_YPSERV 11			/* Can't communicate with ypserv */
#define	YPERR_NODOM 12			/* Local domain name not set */
#define	YPERR_BADDB 13			/* yp data base is bad */
#define	YPERR_VERS 14			/* YP version mismatch */
#define	YPERR_ACCESS 15			/* Access violation */
#define	YPERR_BUSY 16			/* Database is busy */

/* Types of update operations */
#define	YPOP_CHANGE 1			/* change, do not add */
#define	YPOP_INSERT 2			/* add, do not change */
#define	YPOP_DELETE 3			/* delete this entry */
#define	YPOP_STORE  4			/* add, or change */

__BEGIN_DECLS

/* struct ypall_callback * is the arg which must be passed to yp_all */
struct ypall_callback
  {
    int (*foreach) __PMT ((int __status, char *__key, int __keylen,
			 char *__val, int __vallen, char *__data));
    char *data;
  };

/* External NIS client function references. */
extern int yp_bind __P ((__const char *));
extern void yp_unbind __P ((__const char *));
extern int yp_get_default_domain __P ((char **));
extern int yp_match __P ((__const char *, __const char *, __const char *,
			  __const int, char **, int *));
extern int yp_first __P ((__const char *, __const char *, char **,
			  int *, char **, int *));
extern int yp_next __P ((__const char *, __const char *, __const char *,
			 __const int, char **, int *, char **, int *));
extern int yp_master __P ((__const char *, __const char *, char **));
extern int yp_order __P ((__const char *, __const char *, unsigned int *));
extern int yp_all __P ((__const char *, __const char *,
			__const struct ypall_callback *));
extern __const char *yperr_string __P ((__const int));
extern __const char *ypbinderr_string __P ((__const int));
extern int ypprot_err __P ((__const int));
extern int yp_update __P ((char *, char *, unsigned,  char *,
			   int, char *, int));
#if 0
extern int yp_maplist __P ((__const char *, struct ypmaplist **));
#endif

/* Exist only under BSD and Linux systems */
extern int __yp_check __P ((char **));

__END_DECLS

#endif	/* __RPCSVC_YPCLNT_H__ */
