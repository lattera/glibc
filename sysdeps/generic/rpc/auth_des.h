/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _RPC_AUTH_DES_H

#define _RPC_AUTH_DES_H	1
#include <sys/cdefs.h>

__BEGIN_DECLS

/* This is no complete version of this header.  More definitions with
   the real authentication stuff will come in 1997.  For now we only
   need to define the function for handling public keys.  */


/* Get the public key for NAME and place it in KEY.  NAME can only be
   up to MAXNETNAMELEN bytes long and the destination buffer KEY should
   have HEXKEYBATES + 1 bytes long to fit all characters from the key.  */
extern int getpublickey __P ((__const char *__name, char *__key));

/* Get the secret key for NAME and place it in KEY.  PASSWD is used to
   decrypt the encrypted key stored in the database.  NAME can only be
   up to MAXNETNAMELEN bytes long and the destination buffer KEY
   should have HEXKEYBATES + 1 bytes long to fit all characters from
   the key.  */
extern int getsecretkey __P ((__const char *__name, char *__key,
			      __const char *__passwd));

__END_DECLS

#endif /* rpc/auth_des.h */
