/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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
   Boston, MA 02111-1307, USA. */

/* This file defines everything that client code should need to
   know to talk to the nscd daemon.  */

#ifndef _NSCD_CLIENT_H
#define _NSCD_CLIENT_H	1

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
  LASTREQ,
} request_type;


/* Header common to all requests */
typedef struct
{
  int version;		/* Version number of the daemon interface.  */
  request_type type;	/* Service requested.  */
  ssize_t key_len;	/* Key length.  */
} request_header;


/* Structure sent in reply to password query.  Note that this struct is
   sent also if the service is disabled or there is no record found.  */
typedef struct
{
  int version;
  int found;
  ssize_t pw_name_len;
  ssize_t pw_passwd_len;
  uid_t pw_uid;
  gid_t pw_gid;
  ssize_t pw_gecos_len;
  ssize_t pw_dir_len;
  ssize_t pw_shell_len;
} pw_response_header;


/* Structure sent in reply to group query.  Note that this struct is
   sent also if the service is disabled or there is no record found.  */
typedef struct
{
  int version;
  int found;
  ssize_t gr_name_len;
  ssize_t gr_passwd_len;
  gid_t gr_gid;
  ssize_t gr_mem_cnt;
} gr_response_header;


/* Structure sent in reply to host query.  Note that this struct is
   sent also if the service is disabled or there is no record found.  */
typedef struct
{
  int version;
  int found;
  ssize_t h_name_len;
  ssize_t h_aliases_cnt;
  int h_addrtype;
  int h_length;
  ssize_t h_addr_list_cnt;
  int error;
} hst_response_header;


#endif /* nscd.h */
