/* Copyright (c) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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

#ifndef _NSCD_H
#define _NSCD_H	1

#include <pthread.h>
#include <time.h>
#include <sys/uio.h>

/* The declarations for the request and response types are in the file
   "nscd-client.h", which should contain everything needed by client
   functions.  */
#include "nscd-client.h"


/* Handle databases.  */
typedef enum
{
  pwddb,
  grpdb,
  hstdb,
  lastdb
} dbtype;


/* Structure for one hash table entry.  */
struct hashentry
{
  request_type type;		/* Which type of dataset.  */
  size_t len;			/* Length of key.  */
  void *key;			/* Pointer to key.  */
  uid_t owner;                  /* If secure table, this is the owner.  */
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


/* Global variables.  */
extern const char *dbnames[lastdb];
extern const char *serv2str[LASTREQ];

extern const struct iovec pwd_iov_disabled;
extern const struct iovec grp_iov_disabled;
extern const struct iovec hst_iov_disabled;

/* Number of threads to run.  */
extern int nthreads;

/* Tables for which we cache data with uid */
extern int secure[lastdb];
extern int secure_in_use; /* Is one of the above 1 ? */

/* User name to run server processes as */
extern const char *server_user;

/* Prototypes for global functions.  */

/* nscd.c */
extern void termination_handler (int signum) __attribute__ ((__noreturn__));
extern int nscd_open_socket (void);

/* connections.c */
extern void nscd_init (const char *conffile);
extern void close_sockets (void);
extern void start_threads (void) __attribute__ ((__noreturn__));

/* nscd_conf.c */
extern int nscd_parse_file (const char *fname, struct database dbs[lastdb]);

/* nscd_stat.c */
extern void send_stats (int fd, struct database dbs[lastdb]);
extern int receive_print_stats (void) __attribute__ ((__noreturn__));

/* cache.c */
extern struct hashentry *cache_search (int type, void *key, size_t len,
				       struct database *table, uid_t owner);
extern void cache_add (int type, void *key, size_t len,
		       const void *packet, size_t iovtotal, void *data,
		       int last, time_t t, struct database *table,
		       uid_t owner);
extern void prune_cache (struct database *table, time_t now);

/* pwdcache.c */
extern void addpwbyname (struct database *db, int fd, request_header *req,
			 void *key, uid_t uid);
extern void addpwbyuid (struct database *db, int fd, request_header *req,
			void *key, uid_t uid);

/* grpcache.c */
extern void addgrbyname (struct database *db, int fd, request_header *req,
			 void *key, uid_t uid);
extern void addgrbygid (struct database *db, int fd, request_header *req,
			void *key, uid_t uid);

/* hstcache.c */
extern void addhstbyname (struct database *db, int fd, request_header *req,
			  void *key, uid_t uid);
extern void addhstbyaddr (struct database *db, int fd, request_header *req,
			  void *key, uid_t uid);
extern void addhstbynamev6 (struct database *db, int fd, request_header *req,
			    void *key, uid_t uid);
extern void addhstbyaddrv6 (struct database *db, int fd, request_header *req,
			    void *key, uid_t uid);


#endif /* nscd.h */
