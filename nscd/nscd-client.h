/* Copyright (c) 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1998.

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

/* This file defines everything that client code should need to
   know to talk to the nscd daemon.  */

#ifndef _NSCD_CLIENT_H
#define _NSCD_CLIENT_H	1

#include <nscd-types.h>

/* Version number of the daemon interface */
#define NSCD_VERSION 2

/* Path of the file where the PID of the running system is stored.  */
#define _PATH_NSCDPID	 "/var/run/nscd.pid"

/* Path for the Unix domain socket.  */
#define _PATH_NSCDSOCKET "/var/run/.nscd_socket"

/* Path for the configuration file.  */
#define _PATH_NSCDCONF	 "/etc/nscd.conf"


/* Available services.  */
typedef enum
{
  GETPWBYNAME,
  GETPWBYUID,
  GETGRBYNAME,
  GETGRBYGID,
  GETHOSTBYNAME,
  GETHOSTBYNAMEv6,
  GETHOSTBYADDR,
  GETHOSTBYADDRv6,
  LASTDBREQ = GETHOSTBYADDRv6,
  SHUTDOWN,		/* Shut the server down.  */
  GETSTAT,		/* Get the server statistic.  */
  INVALIDATE,           /* Invalidate one special cache.  */
  LASTREQ
} request_type;


/* Header common to all requests */
typedef struct
{
  int32_t version;	/* Version number of the daemon interface.  */
  request_type type;	/* Service requested.  */
  int32_t key_len;	/* Key length.  */
} request_header;


/* Structure sent in reply to password query.  Note that this struct is
   sent also if the service is disabled or there is no record found.  */
typedef struct
{
  int32_t version;
  int32_t found;
  nscd_ssize_t pw_name_len;
  nscd_ssize_t pw_passwd_len;
  uid_t pw_uid;
  gid_t pw_gid;
  nscd_ssize_t pw_gecos_len;
  nscd_ssize_t pw_dir_len;
  nscd_ssize_t pw_shell_len;
} pw_response_header;


/* Structure sent in reply to group query.  Note that this struct is
   sent also if the service is disabled or there is no record found.  */
typedef struct
{
  int32_t version;
  int32_t found;
  nscd_ssize_t gr_name_len;
  nscd_ssize_t gr_passwd_len;
  gid_t gr_gid;
  nscd_ssize_t gr_mem_cnt;
} gr_response_header;


/* Structure sent in reply to host query.  Note that this struct is
   sent also if the service is disabled or there is no record found.  */
typedef struct
{
  int32_t version;
  int32_t found;
  nscd_ssize_t h_name_len;
  nscd_ssize_t h_aliases_cnt;
  int32_t h_addrtype;
  int32_t h_length;
  nscd_ssize_t h_addr_list_cnt;
  int32_t error;
} hst_response_header;


#endif /* nscd.h */
