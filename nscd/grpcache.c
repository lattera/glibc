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
#include <grp.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpcsvc/nis.h>
#include <sys/types.h>

#include "dbg_log.h"
#include "nscd.h"

static unsigned long modulo = 211;
static unsigned long postimeout = 3600;
static unsigned long negtimeout = 60;

static unsigned long poshit = 0;
static unsigned long posmiss = 0;
static unsigned long neghit = 0;
static unsigned long negmiss = 0;

struct grphash
{
  time_t create;
  struct grphash *next;
  struct group *grp;
};
typedef struct grphash grphash;

struct gidhash
{
  struct gidhash *next;
  struct group *grptr;
};
typedef struct gidhash gidhash;

struct neghash
{
  time_t create;
  struct neghash *next;
  char *key;
};
typedef struct neghash neghash;

static grphash *grptbl;
static gidhash *gidtbl;
static neghash *negtbl;

static pthread_rwlock_t grplock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_rwlock_t neglock = PTHREAD_RWLOCK_INITIALIZER;

static void *grptable_update (void *);
static void *negtable_update (void *);

void
get_gr_stat (stat_response_header *stat)
{
  stat->gr_poshit = poshit;
  stat->gr_posmiss = posmiss;
  stat->gr_neghit = neghit;
  stat->gr_negmiss = negmiss;
  stat->gr_size = modulo;
  stat->gr_posttl = postimeout;
  stat->gr_negttl = negtimeout;
}

void
set_grp_modulo (unsigned long mod)
{
  modulo = mod;
}

void
set_pos_grp_ttl (unsigned long ttl)
{
  postimeout = ttl;
}

void
set_neg_grp_ttl (unsigned long ttl)
{
  negtimeout = ttl;
}

int
cache_grpinit ()
{
  pthread_attr_t attr;
  pthread_t thread;

  grptbl = calloc (modulo, sizeof (grphash));
  if (grptbl == NULL)
    return -1;
  gidtbl = calloc (modulo, sizeof (grphash));
  if (gidtbl == NULL)
    return -1;
  negtbl = calloc (modulo, sizeof (neghash));
  if (negtbl == NULL)
    return -1;

  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  pthread_create (&thread, NULL, grptable_update, &attr);
  pthread_create (&thread, NULL, negtable_update, &attr);

  pthread_attr_destroy (&attr);

  return 0;
}

static struct group *
save_grp (struct group *src)
{
  struct group *dest;
  unsigned long int l;
  size_t tlen;
  size_t name_len = strlen (src->gr_name) + 1;
  size_t passwd_len = strlen (src->gr_passwd) + 1;
  char *cp;

  /* How many members does this group have?  */
  l = tlen = 0;
  while (src->gr_mem[l] != NULL)
    tlen += strlen (src->gr_mem[l++]) + 1;

  dest = malloc (sizeof (struct group) + (l + 1) * sizeof (char *)
		 + name_len + passwd_len + tlen);
  if (dest == NULL)
    return NULL;

  dest->gr_mem = (char **) (dest + 1);
  cp = (char *) (dest->gr_mem + l + 1);

  dest->gr_name = cp;
  cp = mempcpy (cp, src->gr_name, name_len);
  dest->gr_passwd = cp;
  cp = mempcpy (cp, src->gr_passwd, passwd_len);
  dest->gr_gid = src->gr_gid;

  l = 0;
  while (src->gr_mem[l] != NULL)
    {
      dest->gr_mem[l] = cp;
      cp = stpcpy (cp, src->gr_mem[l]) + 1;
      ++l;
    }
  dest->gr_mem[l] = NULL;

  return dest;
}

static void
free_grp (struct group *src)
{
  free (src);
}

static int
add_cache (struct group *grp)
{
  grphash *work;
  gidhash *gidwork;
  unsigned long int hash = __nis_hash (grp->gr_name,
				       strlen (grp->gr_name)) % modulo;

  if (debug_flag)
    dbg_log (_("grp_add_cache (%s)"), grp->gr_name);

  work = &grptbl[hash];

  if (grptbl[hash].grp == NULL)
    grptbl[hash].grp = save_grp (grp);
  else
    {
      while (work->next != NULL)
	work = work->next;

      work->next = calloc (1, sizeof (grphash));
      work->next->grp = save_grp (grp);
      work = work->next;
    }

  time (&work->create);
  gidwork = &gidtbl[grp->gr_gid % modulo];
  if (gidwork->grptr == NULL)
    gidwork->grptr = work->grp;
  else
    {
      while (gidwork->next != NULL)
	gidwork = gidwork->next;

      gidwork->next = calloc (1, sizeof (gidhash));
      gidwork->next->grptr = work->grp;
    }

  return 0;
}

static struct group *
cache_search_name (const char *name)
{
  grphash *work;
  unsigned long int hash = __nis_hash (name, strlen(name)) % modulo;

  work = &grptbl[hash];

  while (work->grp != NULL)
    {
      if (strcmp (work->grp->gr_name, name) == 0)
	return work->grp;
      if (work->next != NULL)
	work = work->next;
      else
	return NULL;
    }
  return NULL;
}

static struct group *
cache_search_gid (gid_t gid)
{
  gidhash *work;

  work = &gidtbl[gid % modulo];

  while (work->grptr != NULL)
    {
      if (work->grptr->gr_gid == gid)
	return work->grptr;
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
    dbg_log (_("grp_add_netgache (%s|%ld)"), key, hash);

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
    dbg_log (_("grp_cache_search_neg (%s|%ld)"), key, hash);

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
cache_getgrnam (void *v_param)
{
  param_t *param = (param_t *)v_param;
  struct group *grp;

  pthread_rwlock_rdlock (&grplock);
  grp = cache_search_name (param->key);

  /* I don't like it to hold the read only lock longer, but it is
     necessary to avoid to much malloc/free/strcpy.  */

  if (grp != NULL)
    {
      if (debug_flag)
	dbg_log (_("Found \"%s\" in cache !"), param->key);

      ++poshit;
      gr_send_answer (param->conn, grp);
      close_socket (param->conn);

      pthread_rwlock_unlock (&grplock);
    }
  else
    {
      int status;
      int buflen = 1024;
      char *buffer = calloc (1, buflen);
      struct group resultbuf;

      if (debug_flag)
	dbg_log (_("Doesn't found \"%s\" in cache !"), param->key);

      pthread_rwlock_unlock (&grplock);

      pthread_rwlock_rdlock (&neglock);
      status = cache_search_neg (param->key);
      pthread_rwlock_unlock (&neglock);

      if (status == 0)
	{
	  while (buffer != NULL
		 && (getgrnam_r (param->key, &resultbuf, buffer, buflen, &grp)
		     != 0)
		 && errno == ERANGE)
	    {
	      errno = 0;
	      buflen += 1024;
	      buffer = realloc (buffer, buflen);
	    }

	  if (buffer != NULL && grp != NULL)
	    {
	      struct group *tmp;

	      ++poshit;
	      pthread_rwlock_wrlock (&grplock);
	      /* While we are waiting on the lock, somebody else could
		 add this entry.  */
	      tmp = cache_search_name (param->key);
	      if (tmp == NULL)
		add_cache (grp);
	      pthread_rwlock_unlock (&grplock);
	    }
	  else
	    {
	      pthread_rwlock_wrlock (&neglock);
	      add_negcache (param->key);
	      ++negmiss;
	      pthread_rwlock_unlock (&neglock);
	    }
	}
      else
	++neghit;

      gr_send_answer (param->conn, grp);
      close_socket (param->conn);
      if (buffer != NULL)
	free (buffer);
    }
  free (param->key);
  free (param);
  return NULL;
}

void *
cache_gr_disabled (void *v_param)
{
  param_t *param = (param_t *)v_param;

  if (debug_flag)
    dbg_log (_("\tgroup cache is disabled\n"));

  gr_send_disabled (param->conn);
  return NULL;
}

void *
cache_getgrgid (void *v_param)
{
  param_t *param = (param_t *)v_param;
  struct group *grp, resultbuf;
  gid_t gid = strtol (param->key, NULL, 10);

  pthread_rwlock_rdlock (&grplock);
  grp = cache_search_gid (gid);

  /* I don't like it to hold the read only lock longer, but it is
     necessary to avoid to much malloc/free/strcpy.  */

  if (grp != NULL)
    {
      if (debug_flag)
	dbg_log (_("Found \"%d\" in cache !"), gid);

      ++poshit;
      gr_send_answer (param->conn, grp);
      close_socket (param->conn);

      pthread_rwlock_unlock (&grplock);
    }
  else
    {
      int buflen = 1024;
      char *buffer = malloc (buflen);
      int status;

      if (debug_flag)
	dbg_log (_("Doesn't found \"%d\" in cache !"), gid);

      pthread_rwlock_unlock (&grplock);

      pthread_rwlock_rdlock (&neglock);
      status = cache_search_neg (param->key);
      pthread_rwlock_unlock (&neglock);

      if (status == 0)
        {
	  while (buffer != NULL
		 && (getgrgid_r (gid, &resultbuf, buffer, buflen, &grp) != 0)
		 && errno == ERANGE)
	    {
	      errno = 0;
	      buflen += 1024;
	      buffer = realloc (buffer, buflen);
	    }

	  if (buffer != NULL && grp != NULL)
	    {
	      struct group *tmp;

	      ++posmiss;
	      pthread_rwlock_wrlock (&grplock);
	      /* While we are waiting on the lock, somebody else could
		 add this entry.  */
	      tmp = cache_search_gid (gid);
	      if (tmp == NULL)
		add_cache (grp);
	      pthread_rwlock_unlock (&grplock);
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

      gr_send_answer (param->conn, grp);
      close_socket (param->conn);
      if (buffer != NULL)
	free (buffer);
    }
  free (param->key);
  free (param);
  return NULL;
}

static void *
grptable_update (void *v)
{
  time_t now;
  int i;

  sleep (20);

  while (!do_shutdown)
    {
      if (debug_flag > 2)
	dbg_log (_("(grptable_update) Wait for write lock!"));

      pthread_rwlock_wrlock (&grplock);

      if (debug_flag > 2)
	dbg_log (_("(grptable_update) Have write lock"));

      time (&now);
      for (i = 0; i < modulo; ++i)
	{
	  grphash *work = &grptbl[i];

	  while (work && work->grp)
	    {
	      if ((now - work->create) >= postimeout)
		{
		  gidhash *uh = &gidtbl[work->grp->gr_gid % modulo];

		  if (debug_flag)
		    dbg_log (_("Give \"%s\" free"), work->grp->gr_name);

		  while (uh && uh->grptr)
		    {
		      if (uh->grptr->gr_gid == work->grp->gr_gid)
			{
			  if (debug_flag > 3)
			    dbg_log (_("Give gid for \"%s\" free"),
				     work->grp->gr_name);
			  if (uh->next != NULL)
			    {
			      gidhash *tmp = uh->next;
			      uh->grptr = tmp->grptr;
			      uh->next = tmp->next;
			      free (tmp);
			    }
			  else
			    uh->grptr = NULL;
			}
		      uh = uh->next;
		    }

		  free_grp (work->grp);
		  if (work->next != NULL)
		    {
		      grphash *tmp = work->next;
		      work->create = tmp->create;
		      work->next = tmp->next;
		      work->grp = tmp->grp;
		      free (tmp);
		    }
		  else
		    work->grp = NULL;
		}
	      work = work->next;
	    }
	}
      if (debug_flag > 2)
	dbg_log (_("(grptable_update) Release wait lock"));
      pthread_rwlock_unlock (&grplock);
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
	dbg_log (_("(neggrptable_update) Wait for write lock!"));

      pthread_rwlock_wrlock (&neglock);

      if (debug_flag > 2)
	dbg_log (_("(neggrptable_update) Have write lock"));

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
	dbg_log (_("(neggrptable_update) Release wait lock"));
      pthread_rwlock_unlock (&neglock);
      sleep (10);
    }
  return NULL;
}
