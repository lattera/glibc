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

#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <rpcsvc/nis.h>
#include <sys/types.h>

#include "dbg_log.h"
#include "nscd.h"

static unsigned long int modulo = 211;
static unsigned long int postimeout = 600;
static unsigned long int negtimeout = 20;

static unsigned long int poshit = 0;
static unsigned long int posmiss = 0;
static unsigned long int neghit = 0;
static unsigned long int negmiss = 0;

struct pwdhash
{
  time_t create;
  struct pwdhash *next;
  struct passwd *pwd;
};
typedef struct pwdhash pwdhash;

struct uidhash
{
  struct uidhash *next;
  struct passwd *pwptr;
};
typedef struct uidhash uidhash;

struct neghash
{
  time_t create;
  struct neghash *next;
  char *key;
};
typedef struct neghash neghash;

static pwdhash *pwdtbl;
static uidhash *uidtbl;
static neghash *negtbl;

static pthread_rwlock_t pwdlock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t neglock = PTHREAD_RWLOCK_INITIALIZER;

static void *pwdtable_update (void *);
static void *negtable_update (void *);

void
get_pw_stat (stat_response_header *stat)
{
  stat->pw_poshit = poshit;
  stat->pw_posmiss = posmiss;
  stat->pw_neghit = neghit;
  stat->pw_negmiss = negmiss;
  stat->pw_size = modulo;
  stat->pw_posttl = postimeout;
  stat->pw_negttl = negtimeout;
}

void
set_pwd_modulo (unsigned long int mod)
{
  modulo = mod;
}

void
set_pos_pwd_ttl (unsigned long int ttl)
{
  postimeout = ttl;
}

void
set_neg_pwd_ttl (unsigned long int ttl)
{
  negtimeout = ttl;
}

int
cache_pwdinit ()
{
  pthread_attr_t attr;
  pthread_t thread;

  pwdtbl = calloc (modulo, sizeof (pwdhash));
  if (pwdtbl == NULL)
    return -1;
  uidtbl = calloc (modulo, sizeof (uidhash));
  if (uidtbl == NULL)
    return -1;
  negtbl = calloc (modulo, sizeof (neghash));
  if (negtbl == NULL)
    return -1;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  pthread_create (&thread, NULL, pwdtable_update, &attr);
  pthread_create (&thread, NULL, negtable_update, &attr);

  pthread_attr_destroy (&attr);

  return 0;
}

static struct passwd *
save_pwd (struct passwd *src)
{
  struct passwd *dest;

  dest = calloc (1, sizeof (struct passwd));
  dest->pw_name = strdup (src->pw_name);
  dest->pw_passwd = strdup (src->pw_passwd);
  dest->pw_uid = src->pw_uid;
  dest->pw_gid = src->pw_gid;
  dest->pw_gecos = strdup (src->pw_gecos);
  dest->pw_dir = strdup (src->pw_dir);
  dest->pw_shell = strdup (src->pw_shell);

  return dest;
}

static void
free_pwd (struct passwd *src)
{
  free (src->pw_name);
  free (src->pw_passwd);
  free (src->pw_gecos);
  free (src->pw_dir);
  free (src->pw_shell);
  free (src);
}

static int
add_cache (struct passwd *pwd)
{
  pwdhash *work;
  uidhash *uidwork;
  unsigned long int hash = __nis_hash (pwd->pw_name,
				       strlen (pwd->pw_name)) % modulo;

  if (debug_flag)
    dbg_log (_("pwd_add_cache (%s)"), pwd->pw_name);

  work = &pwdtbl[hash];

  if (pwdtbl[hash].pwd == NULL)
    pwdtbl[hash].pwd = save_pwd (pwd);
  else
    {
      while (work->next != NULL)
	work = work->next;

      work->next = calloc (1, sizeof (pwdhash));
      work->next->pwd = save_pwd (pwd);
      work = work->next;
    }
  /* Set a pointer from the pwuid hash table to the pwname hash table */
  time (&work->create);
  uidwork = &uidtbl[pwd->pw_uid % modulo];
  if (uidwork->pwptr == NULL)
    uidwork->pwptr = work->pwd;
  else
   {
      while (uidwork->next != NULL)
	uidwork = uidwork->next;

      uidwork->next = calloc (1, sizeof (uidhash));
      uidwork->next->pwptr = work->pwd;
    }
  return 0;
}

static struct passwd *
cache_search_name (const char *name)
{
  pwdhash *work;
  unsigned long int hash = __nis_hash (name, strlen (name)) % modulo;

  work = &pwdtbl[hash];

  while (work->pwd != NULL)
    {
      if (strcmp (work->pwd->pw_name, name) == 0)
	return work->pwd;
      if (work->next != NULL)
	work = work->next;
      else
	return NULL;
    }
  return NULL;
}

static struct passwd *
cache_search_uid (uid_t uid)
{
  uidhash *work;

  work = &uidtbl[uid % modulo];

  while (work->pwptr != NULL)
    {
      if (work->pwptr->pw_uid == uid)
	return work->pwptr;
      if (work->next != NULL)
	work = work->next;
      else
	return NULL;
    }
  return NULL;
}

static int
add_negcache (char *key)
{
  neghash *work;
  unsigned long int hash = __nis_hash (key, strlen (key)) % modulo;

  if (debug_flag)
    dbg_log (_("pwd_add_netgache (%s|%ld)"), key, hash);

  work = &negtbl[hash];

  if (negtbl[hash].key == NULL)
    {
      negtbl[hash].key = strdup (key);
      negtbl[hash].next = NULL;
    }
  else
    {
      while (work->next != NULL)
	work = work->next;

      work->next = calloc (1, sizeof (neghash));
      work->next->key = strdup (key);
      work = work->next;
    }

  time (&work->create);

  return 0;
}

static int
cache_search_neg (const char *key)
{
  neghash *work;
  unsigned long int hash = __nis_hash (key, strlen (key)) % modulo;

  if (debug_flag)
    dbg_log (_("pwd_cache_search_neg (%s|%ld)"), key, hash);

  work = &negtbl[hash];

  while (work->key != NULL)
    {
      if (strcmp (work->key, key) == 0)
	return 1;
      if (work->next != NULL)
	work = work->next;
      else
	return 0;
    }
  return 0;
}

void *
cache_getpwnam (void *v_param)
{
  struct passwd *pwd;
  param_t *param = (param_t *)v_param;

  pthread_rwlock_rdlock (&pwdlock);
  pwd = cache_search_name (param->key);

  /* I don't like it to hold the read only lock longer, but it is
     necessary to avoid to much malloc/free/strcpy.  */

  if (pwd != NULL)
    {
      if (debug_flag)
	dbg_log (_("Found \"%s\" in cache !"), param->key);

      ++poshit;
      pw_send_answer (param->conn, pwd);
      close_socket (param->conn);

      pthread_rwlock_unlock (&pwdlock);
    }
  else
    {
      int status;
      int buflen = 1024;
      char *buffer = calloc (1, buflen);
      struct passwd resultbuf;

      if (debug_flag)
	dbg_log (_("Doesn't found \"%s\" in cache !"), param->key);

      pthread_rwlock_unlock (&pwdlock);

      pthread_rwlock_rdlock (&neglock);
      status = cache_search_neg (param->key);
      pthread_rwlock_unlock (&neglock);

      if (status == 0)
	{
	  while (buffer != NULL
		 && (getpwnam_r (param->key, &resultbuf, buffer, buflen, &pwd)
		     != 0)
		 && errno == ERANGE)
	    {
	      errno = 0;
	      buflen += 1024;
	      buffer = realloc (buffer, buflen);
	    }

	  if (buffer != NULL && pwd != NULL)
	    {
	      struct passwd *tmp;

	      ++posmiss;
	      pthread_rwlock_wrlock (&pwdlock);
	      /* While we are waiting on the lock, somebody else could
		 add this entry.  */
	      tmp = cache_search_name (param->key);
	      if (tmp == NULL)
		add_cache (pwd);
	      pthread_rwlock_unlock (&pwdlock);
	    }
	  else
	    {
	      ++negmiss;
	      pthread_rwlock_wrlock (&neglock);
	      add_negcache (param->key);
	      pthread_rwlock_unlock (&neglock);
	    }
	}
      else
	++neghit;
      pw_send_answer (param->conn, pwd);
      close_socket (param->conn);
      if (buffer != NULL)
	free (buffer);
    }
  free (param->key);
  free (param);
  return NULL;
}

void *
cache_pw_disabled (void *v_param)
{
  param_t *param = (param_t *)v_param;

  if (debug_flag)
    dbg_log (_("\tpasswd cache is disabled\n"));

  pw_send_disabled (param->conn);
  return NULL;
}

void *
cache_getpwuid (void *v_param)
{
  param_t *param = (param_t *)v_param;
  struct passwd *pwd, resultbuf;
  uid_t uid = strtol (param->key, NULL, 10);

  pthread_rwlock_rdlock (&pwdlock);
  pwd = cache_search_uid (uid);

  /* I don't like it to hold the read only lock longer, but it is
     necessary to avoid to much malloc/free/strcpy.  */

  if (pwd != NULL)
    {
      if (debug_flag)
	dbg_log (_("Found \"%d\" in cache !"), uid);

      ++poshit;
      pw_send_answer (param->conn, pwd);
      close_socket (param->conn);

      pthread_rwlock_unlock (&pwdlock);
    }
  else
    {
      int buflen = 1024;
      char *buffer = malloc (buflen);
      int status;

      if (debug_flag)
	dbg_log (_("Doesn't found \"%d\" in cache !"), uid);

      pthread_rwlock_unlock (&pwdlock);

      pthread_rwlock_rdlock (&neglock);
      status = cache_search_neg (param->key);
      pthread_rwlock_unlock (&neglock);

      if (status == 0)
        {
	  while (buffer != NULL
		 && (getpwuid_r (uid, &resultbuf, buffer, buflen, &pwd) != 0)
		 && errno == ERANGE)
	    {
	      errno = 0;
	      buflen += 1024;
	      buffer = realloc (buffer, buflen);
	    }

	  if (buffer != NULL && pwd != NULL)
	    {
	      struct passwd *tmp;

	      ++posmiss;
	      pthread_rwlock_wrlock (&pwdlock);
	      /* While we are waiting on the lock, somebody else could
		 add this entry.  */
	      tmp = cache_search_uid (uid);
	      if (tmp == NULL)
		add_cache (pwd);
	      pthread_rwlock_unlock (&pwdlock);
	    }
	  else
	    {
	      ++negmiss;
	      pthread_rwlock_wrlock (&neglock);
	      add_negcache (param->key);
	      pthread_rwlock_unlock (&neglock);
	    }
	}
      else
	++neghit;

      pw_send_answer (param->conn, pwd);
      close_socket (param->conn);
      if (buffer != NULL)
	free (buffer);
    }
  free (param->key);
  free (param);
  return NULL;
}

static void *
pwdtable_update (void *v)
{
  time_t now;
  int i;

  sleep (20);

  while (!do_shutdown)
    {
      if (debug_flag > 2)
	dbg_log (_("(pwdtable_update) Wait for write lock!"));

      pthread_rwlock_wrlock (&pwdlock);

      if (debug_flag > 2)
	dbg_log (_("(pwdtable_update) Have write lock"));

      time (&now);
      for (i = 0; i < modulo; ++i)
	{
	  pwdhash *work = &pwdtbl[i];

	  while (work && work->pwd)
	    {
	      if ((now - work->create) >= postimeout)
		{
		  uidhash *uh = &uidtbl[work->pwd->pw_uid % modulo];

		  if (debug_flag)
		    dbg_log (_("Give \"%s\" free"), work->pwd->pw_name);

		  while (uh != NULL && uh->pwptr)
		    {
		      if (uh->pwptr->pw_uid == work->pwd->pw_uid)
			{
			  if (debug_flag)
			    dbg_log (_("Give uid for \"%s\" free"),
				     work->pwd->pw_name);
			  if (uh->next != NULL)
			    {
			      uidhash *tmp = uh->next;
			      uh->pwptr = tmp->pwptr;
			      uh->next = tmp->next;
			      free (tmp);
			    }
			  else
			    uh->pwptr = NULL;
			}
		      uh = uh->next;
		    }

		  free_pwd (work->pwd);
		  if (work->next != NULL)
		    {
		      pwdhash *tmp = work->next;
		      work->create = tmp->create;
		      work->next = tmp->next;
		      work->pwd = tmp->pwd;
		      free (tmp);
		    }
		  else
		    work->pwd = NULL;
		}
	      work = work->next;
	    }
	}
      if (debug_flag > 2)
	dbg_log (_("(pwdtable_update) Release wait lock"));
      pthread_rwlock_unlock (&pwdlock);
      sleep (20);
    }
  return NULL;
}

static void *
negtable_update (void *v)
{
  time_t now;
  int i;

  sleep (30);

  while (!do_shutdown)
    {
      if (debug_flag > 2)
	dbg_log (_("(negpwdtable_update) Wait for write lock!"));

      pthread_rwlock_wrlock (&neglock);

      if (debug_flag > 2)
	dbg_log (_("(negpwdtable_update) Have write lock"));

      time (&now);
      for (i = 0; i < modulo; ++i)
	{
	  neghash *work = &negtbl[i];

	  while (work && work->key)
	    {
	      if ((now - work->create) >= negtimeout)
		{
		  if (debug_flag)
		    dbg_log (_("Give \"%s\" free"), work->key);

		  free (work->key);

		  if (work->next != NULL)
		    {
		      neghash *tmp = work->next;
		      work->create = tmp->create;
		      work->next = tmp->next;
		      work->key = tmp->key;
		      free (tmp);
		    }
		  else
		    work->key = NULL;
		}
	      work = work->next;
	    }
	}
      if (debug_flag > 2)
	dbg_log (_("(negpwdtable_update) Release wait lock"));

      pthread_rwlock_unlock (&neglock);
      sleep (10);
    }
  return NULL;
}
