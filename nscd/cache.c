/* Copyright (c) 1998, 1999, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <atomic.h>
#include <errno.h>
#include <error.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <arpa/inet.h>
#include <rpcsvc/nis.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include "nscd.h"
#include "dbg_log.h"

/* Search the cache for a matching entry and return it when found.  If
   this fails search the negative cache and return (void *) -1 if this
   search was successful.  Otherwise return NULL.

   This function must be called with the read-lock held.  */
struct hashentry *
cache_search (request_type type, void *key, size_t len, struct database *table,
	      uid_t owner)
{
  unsigned long int hash = __nis_hash (key, len) % table->module;
  struct hashentry *work;
  unsigned long int nsearched = 0;

  work = table->array[hash];

  while (work != NULL)
    {
      ++nsearched;

      if (type == work->type && len == work->len
	  && memcmp (key, work->key, len) == 0 && work->owner == owner)
	{
	  /* We found the entry.  Increment the appropriate counter.  */
	  if (work->data == (void *) -1)
	    ++table->neghit;
	  else
	    ++table->poshit;

	  break;
	}

      work = work->next;
    }

  if (nsearched > table->maxnsearched)
    table->maxnsearched = nsearched;

  return work;
}

/* Add a new entry to the cache.  The return value is zero if the function
   call was successful.

   This function must be called with the read-lock held.

   We modify the table but we nevertheless only acquire a read-lock.
   This is ok since we use operations which would be safe even without
   locking, given that the `prune_cache' function never runs.  Using
   the readlock reduces the chance of conflicts.  */
void
cache_add (int type, void *key, size_t len, const void *packet, size_t total,
	   void *data, int last, time_t t, struct database *table, uid_t owner)
{
  unsigned long int hash = __nis_hash (key, len) % table->module;
  struct hashentry *newp;

  newp = malloc (sizeof (struct hashentry));
  if (newp == NULL)
    error (EXIT_FAILURE, errno, _("while allocating hash table entry"));

  newp->type = type;
  newp->len = len;
  newp->key = key;
  newp->owner = owner;
  newp->data = data;
  newp->timeout = t;
  newp->packet = packet;
  newp->total = total;

  newp->last = last;

  /* Put the new entry in the first position.  */
  do
    newp->next = table->array[hash];
  while (atomic_compare_and_exchange_bool_acq (&table->array[hash], newp,
					       newp->next));

  /* Update the statistics.  */
  if (data == (void *) -1)
    ++table->negmiss;
  else if (last)
    ++table->posmiss;

  /* Instead of slowing down the normal process for statistics
     collection we accept living with some incorrect data.  */
  unsigned long int nentries = ++table->nentries;
  if (nentries > table->maxnentries)
    table->maxnentries = nentries;
}

/* Walk through the table and remove all entries which lifetime ended.

   We have a problem here.  To actually remove the entries we must get
   the write-lock.  But since we want to keep the time we have the
   lock as short as possible we cannot simply acquire the lock when we
   start looking for timedout entries.

   Therefore we do it in two stages: first we look for entries which
   must be invalidated and remember them.  Then we get the lock and
   actually remove them.  This is complicated by the way we have to
   free the data structures since some hash table entries share the same
   data.  */
void
prune_cache (struct database *table, time_t now)
{
  size_t cnt = table->module;
  int mark[cnt];
  int anything = 0;
  size_t first = cnt + 1;
  size_t last = 0;

  /* If this table is not actually used don't do anything.  */
  if (cnt == 0)
    return;

  /* If we check for the modification of the underlying file we invalidate
     the entries also in this case.  */
  if (table->check_file)
    {
      struct stat st;

      if (stat (table->filename, &st) < 0)
	{
	  char buf[128];
	  /* We cannot stat() the file, disable file checking if the
             file does not exist.  */
	  dbg_log (_("cannot stat() file `%s': %s"),
		   table->filename, strerror_r (errno, buf, sizeof (buf)));
	  if (errno == ENOENT)
	    table->check_file = 0;
	}
      else
	{
	  if (st.st_mtime != table->file_mtime)
	    {
	      /* The file changed.  Invalidate all entries.  */
	      now = LONG_MAX;
	      table->file_mtime = st.st_mtime;
	    }
	}
    }

  /* We run through the table and find values which are not valid anymore.

     Note that for the initial step, finding the entries to be removed,
     we don't need to get any lock.  It is at all timed assured that the
     linked lists are set up correctly and that no second thread prunes
     the cache.  */
  do
    {
      struct hashentry *runp = table->array[--cnt];

      mark[cnt] = 0;

      while (runp != NULL)
	{
	  if (runp->timeout < now)
	    {
	      ++mark[cnt];
	      anything = 1;
	      first = MIN (first, cnt);
	      last = MAX (last, cnt);
	    }
	  runp = runp->next;
	}
    }
  while (cnt > 0);

  if (anything)
    {
      struct hashentry *head = NULL;

      /* Now we have to get the write lock since we are about to modify
	 the table.  */
      if (__builtin_expect (pthread_rwlock_trywrlock (&table->lock) != 0, 0))
	{
	  ++table->wrlockdelayed;
	  pthread_rwlock_wrlock (&table->lock);
	}

      while (first <= last)
	{
	  if (mark[first] > 0)
	    {
	      struct hashentry *runp;

	      while (table->array[first]->timeout < now)
		{
		  table->array[first]->dellist = head;
		  head = table->array[first];
		  table->array[first] = head->next;
		  --table->nentries;
		  if (--mark[first] == 0)
		    break;
		}

	      runp = table->array[first];
	      while (mark[first] > 0)
		{
		  if (runp->next->timeout < now)
		    {
		      runp->next->dellist = head;
		      head = runp->next;
		      runp->next = head->next;
		      --mark[first];
		      --table->nentries;
		    }
		  else
		    runp = runp->next;
		}
	    }
	  ++first;
	}

      /* It's all done.  */
      pthread_rwlock_unlock (&table->lock);

      /* One extra pass if we do debugging.  */
      if (__builtin_expect (debug_level > 0, 0))
	{
	  struct hashentry *runp = head;

	  while (runp != NULL)
	    {
	      char buf[INET6_ADDRSTRLEN];
	      const char *str;

	      if (runp->type == GETHOSTBYADDR || runp->type == GETHOSTBYADDRv6)
		{
		  inet_ntop (runp->type == GETHOSTBYADDR ? AF_INET : AF_INET6,
			     runp->key, buf, sizeof (buf));
		  str = buf;
		}
	      else
		str = runp->key;

	      dbg_log ("remove %s entry \"%s\"", serv2str[runp->type], str);

	      runp = runp->dellist;
	    }
	}

      /* And another run to free the data.  */
      do
	{
	  struct hashentry *old = head;

	  /* Free the data structures.  */
	  if (old->data == (void *) -1)
	    free (old->key);
	  else if (old->last)
	    free (old->data);

	  head = head->dellist;

	  free (old);
	}
      while (head != NULL);
    }
}
