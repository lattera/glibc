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

#include <assert.h>
#include <atomic.h>
#include <errno.h>
#include <error.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <arpa/inet.h>
#include <rpcsvc/nis.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include "nscd.h"
#include "dbg_log.h"


/* Number of times a value is reloaded without being used.  UINT_MAX
   means unlimited.  */
unsigned int reload_count = DEFAULT_RELOAD_LIMIT;


/* Search the cache for a matching entry and return it when found.  If
   this fails search the negative cache and return (void *) -1 if this
   search was successful.  Otherwise return NULL.

   This function must be called with the read-lock held.  */
struct datahead *
cache_search (request_type type, void *key, size_t len,
	      struct database_dyn *table, uid_t owner)
{
  unsigned long int hash = __nis_hash (key, len) % table->head->module;

  unsigned long int nsearched = 0;
  struct datahead *result = NULL;

  ref_t work = table->head->array[hash];
  while (work != ENDREF)
    {
      ++nsearched;

      struct hashentry *here = (struct hashentry *) (table->data + work);

      if (type == here->type && len == here->len
	  && memcmp (key, table->data + here->key, len) == 0
	  && here->owner == owner)
	{
	  /* We found the entry.  Increment the appropriate counter.  */
	  struct datahead *dh
	    = (struct datahead *) (table->data + here->packet);

	  /* See whether we must ignore the entry.  */
	  if (dh->usable)
	    {
	      /* We do not synchronize the memory here.  The statistics
		 data is not crucial, we synchronize only once in a while
		 in the cleanup threads.  */
	      if (dh->notfound)
		++table->head->neghit;
	      else
		{
		  ++table->head->poshit;

		  if (dh->nreloads != 0)
		    dh->nreloads = 0;
		}

	      result = dh;
	      break;
	    }
	}

      work = here->next;
    }

  if (nsearched > table->head->maxnsearched)
    table->head->maxnsearched = nsearched;

  return result;
}

/* Add a new entry to the cache.  The return value is zero if the function
   call was successful.

   This function must be called with the read-lock held.

   We modify the table but we nevertheless only acquire a read-lock.
   This is ok since we use operations which would be safe even without
   locking, given that the `prune_cache' function never runs.  Using
   the readlock reduces the chance of conflicts.  */
int
cache_add (int type, const void *key, size_t len, struct datahead *packet,
	   bool first, struct database_dyn *table,
	   uid_t owner)
{
  if (__builtin_expect (debug_level >= 2, 0))
    {
      const char *str;
      char buf[INET6_ADDRSTRLEN + 1];
      if (type == GETHOSTBYADDR || type == GETHOSTBYADDRv6)
	str = inet_ntop (type == GETHOSTBYADDR ? AF_INET : AF_INET6,
			 key, buf, sizeof (buf));
      else
	str = key;

      dbg_log (_("add new entry \"%s\" of type %s for %s to cache%s"),
	       str, serv2str[type], dbnames[table - dbs],
	       first ? " (first)" : "");
    }

  unsigned long int hash = __nis_hash (key, len) % table->head->module;
  struct hashentry *newp;

  newp = mempool_alloc (table, sizeof (struct hashentry));
  /* If we cannot allocate memory, just do not do anything.  */
  if (newp == NULL)
    return -1;

  newp->type = type;
  newp->first = first;
  newp->len = len;
  newp->key = (char *) key - table->data;
  assert (newp->key + newp->len <= table->head->first_free);
  newp->owner = owner;
  newp->packet = (char *) packet - table->data;

  /* Put the new entry in the first position.  */
  do
    newp->next = table->head->array[hash];
  while (atomic_compare_and_exchange_bool_acq (&table->head->array[hash],
					       (ref_t) ((char *) newp
							- table->data),
					       (ref_t) newp->next));

  /* Update the statistics.  */
  if (packet->notfound)
    ++table->head->negmiss;
  else if (first)
    ++table->head->posmiss;

  /* We depend on this value being correct and at least as high as the
     real number of entries.  */
  atomic_increment (&table->head->nentries);

  /* It does not matter that we are not loading the just increment
     value, this is just for statistics.  */
  unsigned long int nentries = table->head->nentries;
  if (nentries > table->head->maxnentries)
    table->head->maxnentries = nentries;

  return 0;
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
prune_cache (struct database_dyn *table, time_t now)
{
  size_t cnt = table->head->module;

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
  bool mark[cnt];
  size_t first = cnt + 1;
  size_t last = 0;
  char *const data = table->data;
  bool any = false;

  do
    {
      ref_t run = table->head->array[--cnt];

      while (run != ENDREF)
	{
	  struct hashentry *runp = (struct hashentry *) (data + run);
	  struct datahead *dh = (struct datahead *) (data + runp->packet);

	  /* Check whether the entry timed out.  */
	  if (dh->timeout < now)
	    {
	      /* This hash bucket could contain entries which need to
		 be looked at.  */
	      mark[cnt] = true;

	      first = MIN (first, cnt);
	      last = MAX (last, cnt);

	      /* We only have to look at the data of the first entries
		 since the count information is kept in the data part
		 which is shared.  */
	      if (runp->first)
		{

		  /* At this point there are two choices: we reload the
		     value or we discard it.  Do not change NRELOADS if
		     we never not reload the record.  */
		  if ((reload_count != UINT_MAX
		       && __builtin_expect (dh->nreloads >= reload_count, 0))
		      /* We always remove negative entries.  */
		      || dh->notfound
		      /* Discard everything if the user explicitly
			 requests it.  */
		      || now == LONG_MAX)
		    {
		      /* Remove the value.  */
		      dh->usable = false;

		      /* We definitely have some garbage entries now.  */
		      any = true;
		    }
		  else
		    {
		      /* Reload the value.  We do this only for the
			 initially used key, not the additionally
			 added derived value.  */
		      switch (runp->type)
			{
			case GETPWBYNAME:
			  readdpwbyname (table, runp, dh);
			  break;

			case GETPWBYUID:
			  readdpwbyuid (table, runp, dh);
			  break;

			case GETGRBYNAME:
			  readdgrbyname (table, runp, dh);
			  break;

			case GETGRBYGID:
			  readdgrbygid (table, runp, dh);
			  break;

			case GETHOSTBYNAME:
			  readdhstbyname (table, runp, dh);
			  break;

			case GETHOSTBYNAMEv6:
			  readdhstbynamev6 (table, runp, dh);
			  break;

			case GETHOSTBYADDR:
			  readdhstbyaddr (table, runp, dh);
			  break;

			case GETHOSTBYADDRv6:
			  readdhstbyaddrv6 (table, runp, dh);
			  break;

			case GETAI:
			  readdhstai (table, runp, dh);
			  break;

			case INITGROUPS:
			  readdinitgroups (table, runp, dh);
			  break;

			default:
			  assert (! "should never happen");
			}

		      /* If the entry has been replaced, we might need
			 cleanup.  */
		      any |= !dh->usable;
		    }
		}
	    }
	  else
	    assert (dh->usable);

	  run = runp->next;
	}
    }
  while (cnt > 0);

  if (first <= last)
    {
      struct hashentry *head = NULL;

      /* Now we have to get the write lock since we are about to modify
	 the table.  */
      if (__builtin_expect (pthread_rwlock_trywrlock (&table->lock) != 0, 0))
	{
	  ++table->head->wrlockdelayed;
	  pthread_rwlock_wrlock (&table->lock);
	}

      while (first <= last)
	{
	  if (mark[first])
	    {
	      ref_t *old = &table->head->array[first];
	      ref_t run = table->head->array[first];

	      while (run != ENDREF)
		{
		  struct hashentry *runp = (struct hashentry *) (data + run);
		  struct datahead *dh
		    = (struct datahead *) (data + runp->packet);

		  if (! dh->usable)
		    {
		      /* We need the list only for debugging but it is
			 more costly to avoid creating the list than
			 doing it.  */
		      runp->dellist = head;
		      head = runp;

		      /* No need for an atomic operation, we have the
			 write lock.  */
		      --table->head->nentries;

		      run = *old = runp->next;
		    }
		  else
		    {
		      old = &runp->next;
		      run = runp->next;
		    }
		}
	    }

	  ++first;
	}

      /* It's all done.  */
      pthread_rwlock_unlock (&table->lock);

      /* Make sure the data is saved to disk.  */
      if (table->persistent)
	msync (table->head,
	       table->data + table->head->first_free - (char *) table->head,
	       MS_ASYNC);

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
			     table->data + runp->key, buf, sizeof (buf));
		  str = buf;
		}
	      else
		str = table->data + runp->key;

	      dbg_log ("remove %s entry \"%s\"", serv2str[runp->type], str);

	      runp = runp->dellist;
	    }
	}
    }

  /* Run garbage collection if any entry has been removed or replaced.  */
  if (any)
    gc (table);
}
