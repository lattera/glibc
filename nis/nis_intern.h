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

struct dir_binding
{
  CLIENT *clnt;                  /* RPC CLIENT handle */
  nis_server *server_val;        /* List of servers */
  u_int server_len;              /* # of servers */
  u_int server_used;             /* Which server we are bind in the moment ? */
  u_int current_ep;              /* Which endpoint of the server are in use? */
  u_int trys;                    /* How many server have we tried ? */
  bool_t master_only;            /* Is only binded to the master */
  bool_t use_auth;               /* Do we use AUTH ? */
  bool_t use_udp;                /* Do we use UDP ? */
  time_t create;                 /* Binding creation time */
  struct sockaddr_in addr;       /* Server's IP address */
  int socket;                    /* Server's local socket */
  unsigned short port;           /* Local port */
};
typedef struct dir_binding dir_binding;

extern unsigned long inetstr2int __P ((const char *str));
extern long __nis_findfastest __P ((dir_binding *bind));
extern nis_error __do_niscall2 __P ((const nis_server *serv, u_int serv_len,
				     u_long prog, xdrproc_t xargs, caddr_t req,
				     xdrproc_t xres, caddr_t resp,
				     u_long flags));
extern nis_error __do_niscall __P ((const_nis_name name, u_long prog,
				    xdrproc_t xargs, caddr_t req,
				    xdrproc_t xres, caddr_t resp,
				    u_long flags));
extern AUTH *authdes_pk_create __P ((const char *, const netobj *, u_int,
				     struct sockaddr *, des_block *));

/* NIS+ cache */
extern directory_obj *__cache_search __P ((const_nis_name name));
extern nis_error __cache_add __P ((fd_result *));

__END_DECLS

#endif
