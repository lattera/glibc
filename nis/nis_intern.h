/* Copyright (c) 1997 Free Software Foundation, Inc.
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

#ifndef __NIS_INTERN_H

#define __NIS_INTERN_H
#include <features.h>

__BEGIN_DECLS

extern nis_error __do_niscall (__const nis_server *server, int server_len,
			       u_long prog, xdrproc_t xargs, caddr_t req,
			       xdrproc_t xres, caddr_t resp, u_long flags);
#if defined (HAVE_SECURE_RPC)
extern AUTH *authdes_pk_create (const char *, const netobj *, u_int,
				struct sockaddr *, des_block *);
#endif
extern nis_name *__nis_expandname (__const nis_name);

__END_DECLS

#endif
