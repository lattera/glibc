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

#ifndef _NSCD_H
#define _NSCD_H 1

#include <grp.h>
#include <pwd.h>

/* Version number of the daemon interface */
#define NSCD_VERSION 1

/* How many threads do we spawn maximal ? */
#define MAX_NUM_CONNECTIONS 16

/* Services provided */
typedef enum
{
  GETPWBYNAME,
  GETPWBYUID,
  GETGRBYNAME,
  GETGRBYGID,
  GETHOSTBYNAME,
  GETHOSTBYADDR,
  SHUTDOWN,		/* Shut the server down */
  GETSTAT               /* Get the server statistic */
} request_type;

/* Header common to all requests */
typedef struct
{
  /* Version number of the daemon interface */
  int version;
  /* Service requested */
  request_type type;
  /* key len */
  ssize_t key_len;
} request_header;

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

typedef struct
{
  int version;
  int found;
  ssize_t gr_name_len;
  ssize_t gr_passwd_len;
  gid_t gr_gid;
  ssize_t gr_mem_len;
} gr_response_header;

typedef struct
{
  int debug_level;
  int pw_enabled;
  unsigned long pw_poshit;
  unsigned long pw_posmiss;
  unsigned long pw_neghit;
  unsigned long pw_negmiss;
  unsigned long pw_size;
  unsigned long pw_posttl;
  unsigned long pw_negttl;
  int gr_enabled;
  unsigned long gr_poshit;
  unsigned long gr_posmiss;
  unsigned long gr_neghit;
  unsigned long gr_negmiss;
  unsigned long gr_size;
  unsigned long gr_posttl;
  unsigned long gr_negttl;
} stat_response_header;

#define _PATH_NSCDPID	 "/var/run/nscd.pid"
#define _PATH_NSCDSOCKET "/var/run/.nscd_socket"
#define _PATH_NSCDCONF	 "/etc/nscd.conf"

typedef struct
{
  char *key;
  int conn;
} param_t;

extern int  do_shutdown; /* 1 if we should quit the programm.  */
extern int  disabled_passwd;
extern int  disabled_group;

extern int  nscd_parse_file __P ((const char *fname));
extern int  set_logfile __P ((const char *logfile));
extern void set_pos_pwd_ttl __P ((unsigned long));
extern void set_neg_pwd_ttl __P ((unsigned long));
extern void set_pos_grp_ttl __P ((unsigned long));
extern void set_neg_grp_ttl __P ((unsigned long));
extern void set_pwd_modulo __P ((unsigned long));
extern void set_grp_modulo __P ((unsigned long));

extern void init_sockets __P ((void));
extern void close_socket __P ((int conn));
extern void close_sockets __P ((void));
extern void get_request __P ((int *conn, request_header *req, char **key));
extern void pw_send_answer __P ((int conn, struct passwd *pwd));
extern void pw_send_disabled __P ((int conn));
extern void gr_send_answer __P ((int conn, struct group *grp));
extern void gr_send_disabled __P ((int conn));

extern int  cache_pwdinit __P ((void));
extern void *cache_getpwnam __P ((void *param));
extern void *cache_getpwuid __P ((void *param));
extern void *cache_pw_disabled __P ((void *param));

extern int  cache_grpinit __P ((void));
extern void *cache_getgrnam __P ((void *param));
extern void *cache_getgrgid __P ((void *param));
extern void *cache_gr_disabled __P ((void *param));

extern int __nscd_open_socket __P ((void));

extern void get_pw_stat __P ((stat_response_header *resp));
extern void get_gr_stat __P ((stat_response_header *resp));
extern void print_stat __P ((void));
extern void stat_send __P ((int conn, stat_response_header *resp));

#endif
