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
#define _NSCD_H	1

#include <pthread.h>
#include <time.h>
#include <sys/uio.h>


/* Version number of the daemon interface */
#define NSCD_VERSION 2

/* Path of the file where the PID of the running system is stored.  */
#define _PATH_NSCDPID	 "/var/run/nscd.pid"

/* Path for the Unix domain socket.  */
#define _PATH_NSCDSOCKET "/var/run/.nscd_socket"

/* Path for the configuration file.  */
#define _PATH_NSCDCONF	 "/etc/nscd.conf"


/* Handle databases.  */
typedef enum
{
  pwddb,
  grpdb,
  hstdb,
  lastdb
} dbtype;


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


/* Structure for one hash table entry.  */
struct hashentry
{
  request_type type;		/* Which type of dataset.  */
  size_t len;			/* Length of key.  */
  void *key;			/* Pointer to key.  */
  struct hashentry *next;	/* Next entry in this hash bucket list.  */
  time_t timeout;		/* Time when this entry becomes invalid.  */
  ssize_t total;		/* Number of bytes in PACKET.  */
  const void *packet;		/* Records for the result.  */
  void *data;			/* The malloc()ed respond record.  */
  int last;			/* Nonzero if DATA should be free()d.  */
  struct hashentry *dellist;	/* Next record to be deleted.  */
};

/* Structure describing one database.  */
struct database
{
  pthread_rwlock_t lock;

  int enabled;
  int check_file;
  const char *filename;
  time_t file_mtime;
  size_t module;

  const struct iovec *disabled_iov;

  unsigned long int postimeout;
  unsigned long int negtimeout;

  unsigned long int poshit;
  unsigned long int neghit;
  unsigned long int posmiss;
  unsigned long int negmiss;

  struct hashentry **array;
};


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

/* Global variables.  */
extern const char *dbnames[lastdb];
extern const char *serv2str[LASTREQ];

extern const struct iovec pwd_iov_disabled;
extern const struct iovec grp_iov_disabled;
extern const struct iovec hst_iov_disabled;

/* Number of threads to run.  */
extern int nthreads;


/* Prototypes for global functions.  */

/* nscd.c */
extern void termination_handler (int signum);
extern int nscd_open_socket (void);

/* connections.c */
extern void nscd_init (const char *conffile);
extern void close_sockets (void);
extern void start_threads (void);

/* nscd_conf.c */
extern int nscd_parse_file (const char *fname, struct database dbs[lastdb]);

/* nscd_stat.c */
extern void send_stats (int fd, struct database dbs[lastdb]);
extern int receive_print_stats (void);

/* cache.c */
extern struct hashentry *cache_search (int type, void *key, size_t len,
				       struct database *table);
extern void cache_add (int type, void *key, size_t len,
		       const void *packet, size_t iovtotal, void *data,
		       int last, time_t t, struct database *table);
extern void prune_cache (struct database *table, time_t now);

/* pwdcache.c */
extern void addpwbyname (struct database *db, int fd, request_header *req,
			 void *key);
extern void addpwbyuid (struct database *db, int fd, request_header *req,
			void *key);

/* grpcache.c */
extern void addgrbyname (struct database *db, int fd, request_header *req,
			 void *key);
extern void addgrbygid (struct database *db, int fd, request_header *req,
			void *key);

/* hstcache.c */
extern void addhstbyname (struct database *db, int fd, request_header *req,
			  void *key);
extern void addhstbyaddr (struct database *db, int fd, request_header *req,
			  void *key);
extern void addhstbynamev6 (struct database *db, int fd, request_header *req,
			    void *key);
extern void addhstbyaddrv6 (struct database *db, int fd, request_header *req,
			    void *key);

#endif /* nscd.h */
