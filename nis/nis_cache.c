/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nis_cache.h>
#include <bits/libc-lock.h>

#include "nis_intern.h"

static struct timeval TIMEOUT = {10, 0};

#define HEADER_MAGIC  0x07021971
#define SPACER_MAGIC  0x07654321

#define CACHE_VERSION 0x00000001

struct cache_header
{
  u_long magic;               /* Magic number */
  u_long vers;                /* Cache file format version */
  u_short tcp_port;           /* tcp port of nis_cachemgr */
  u_short udp_port;           /* udp port of nis_cachemgr */
  u_long entries;             /* Number of cached objs. */
  off_t used;                 /* How many space are used ? */
};
typedef struct cache_header cache_header;

struct cache_spacer
{
  u_long magic;                /* Magic number */
  u_long hashval;
  time_t ctime;                /* time we have created this object */
  time_t ttl;                  /* time to life of this object */
  off_t next_offset;
};
typedef struct cache_spacer cache_spacer;

static int cache_fd = -1;
static int clnt_sock;
static caddr_t maddr = NULL;
static size_t msize;
static CLIENT *cache_clnt = NULL;

/* If there is no cachemgr, we shouldn't use NIS_SHARED_DIRCACHE, if
   there is no NIS_SHARED_DIRCACHE, we couldn't use nis_cachemgr.
   So, if the clnt_call to nis_cachemgr fails, we also close the cache file.
   But another thread could read the cache => lock the cache_fd and cache_clnt
   variables with the same lock */
__libc_lock_define_initialized (static, mgrlock)

/* close file handles and nis_cachemgr connection */
static void
__cache_close (void)
{
  if (cache_fd != -1)
    {
      close (cache_fd);
      cache_fd = -1;
    }
  if (cache_clnt != NULL)
    {
      clnt_destroy (cache_clnt);
      close (clnt_sock);
      cache_clnt = NULL;
    }
}

/* open the cache file and connect to nis_cachemgr */
static bool_t
__cache_open (void)
{
  struct sockaddr_in sin;
  cache_header hptr;

  if ((cache_fd = open (CACHEFILE, O_RDONLY)) == -1)
    return FALSE;

  if (read (cache_fd, &hptr, sizeof (cache_header)) == -1
      || lseek (cache_fd, 0, SEEK_SET) < 0)
    {
      close (cache_fd);
      cache_fd = -1;
      return FALSE;
    }
  if (hptr.magic != HEADER_MAGIC)
    {
      close (cache_fd);
      cache_fd = -1;
      syslog (LOG_ERR, _("NIS+: cache file is corrupt!"));
      return FALSE;
    }

  memset (&sin, '\0', sizeof (sin));
  sin.sin_family = AF_INET;
  clnt_sock = RPC_ANYSOCK;
  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  sin.sin_port = htons (hptr.tcp_port);
  cache_clnt = clnttcp_create (&sin, CACHEPROG, CACHE_VER_1, &clnt_sock, 0, 0);
  if (cache_clnt == NULL)
    {
      close (cache_fd);
      cache_fd = -1;
      return FALSE;
    }
  /* If the program exists, close the socket */
  if (fcntl (clnt_sock, F_SETFD, FD_CLOEXEC) == -1)
    perror (_("fcntl: F_SETFD"));
  return TRUE;
}

/* Ask the cache manager to update directory 'name'
   for us (because the ttl has expired). */
static nis_error
__cache_refresh (nis_name name)
{
  char clnt_res = 0;
  nis_error result = NIS_SUCCESS;

  __libc_lock_lock (mgrlock);

  if (cache_clnt == NULL)
    result = NIS_FAIL;
  else if (clnt_call (cache_clnt, NIS_CACHE_REFRESH_ENTRY,
		      (xdrproc_t) xdr_wrapstring, (caddr_t) &name,
		      (xdrproc_t) xdr_void, &clnt_res, TIMEOUT)
	   != RPC_SUCCESS)
    {
      __cache_close ();
      result = NIS_FAIL;
    }

  __libc_lock_unlock (mgrlock);

  return result;
}

static nis_error
__cache_find (const_nis_name name, directory_obj **obj)
{
  unsigned long hash;
  struct cache_header *hptr;
  struct cache_spacer *cs;
  struct directory_obj *dir;
  XDR xdrs;
  caddr_t addr, ptr;
  time_t now = time (NULL);

  if (maddr == NULL)
    return NIS_FAIL;

  hash = __nis_hash (name, strlen(name));
  hptr = (cache_header *)maddr;
  if ((hptr->magic != HEADER_MAGIC) || (hptr->vers != CACHE_VERSION))
    {
      syslog (LOG_ERR, _("NIS+: cache file is corrupt!"));
      return NIS_SYSTEMERROR;
    }
  cs = (cache_spacer *)(maddr + sizeof (cache_header));
  while (cs->next_offset)
    {
      if (cs->magic != SPACER_MAGIC)
	{
	  syslog (LOG_ERR, _("NIS+: cache file is corrupt!"));
	  return NIS_SYSTEMERROR;
	}
      if (cs->hashval == hash)
	{
	  if ((now - cs->ctime) > cs->ttl)
	    return NIS_CACHEEXPIRED;
	  dir = calloc (1, sizeof (directory_obj));
	  addr = (caddr_t)cs + sizeof (cache_spacer);
	  xdrmem_create (&xdrs, addr, cs->next_offset, XDR_DECODE);
	  xdr_directory_obj (&xdrs, dir);
	  xdr_destroy (&xdrs);
	  *obj = dir;
	  return NIS_SUCCESS;
	}
      ptr = (caddr_t)cs;
      ptr += cs->next_offset + sizeof (struct cache_spacer);
      cs = (struct cache_spacer *)ptr;
    }
  return NIS_NOTFOUND;
}

static directory_obj *
internal_cache_search (const_nis_name name)
{
  directory_obj *dir;
  nis_error res;
  int second_refresh = 0;
  struct stat s;

  if (cache_fd == -1)
    if (__cache_open () == FALSE)
      return NULL;

 again:
  /* This lock is for nis_cachemgr, so it couldn't write a new cache
     file if we reading it */
  if (__nis_lock_cache () == -1)
    return NULL;

  if (maddr != NULL)
    munmap (maddr, msize);
  if (fstat (cache_fd, &s) < 0)
    maddr = MAP_FAILED;
  else
    {
      msize = s.st_size;
      maddr = mmap (0, msize, PROT_READ, MAP_SHARED, cache_fd, 0);
    }
  if (maddr == MAP_FAILED)
    {
      __nis_unlock_cache ();
      return NULL;
    }

  res = __cache_find (name, &dir);

  munmap (maddr, msize);
  maddr = NULL;
  /* Allow nis_cachemgr to write a new cachefile */
  __nis_unlock_cache ();

  switch(res)
    {
    case NIS_CACHEEXPIRED:
      if (second_refresh)
	{
	  __cache_close ();
	  syslog (LOG_WARNING,
		  _("NIS+: nis_cachemgr failed to refresh object for us"));
	  return NULL;
	}
      ++second_refresh;
      if (__cache_refresh ((char *) name) != NIS_SUCCESS)
	return NULL;
      goto again;
      break;
    case NIS_SUCCESS:
      return dir;
    default:
      return NULL;
    }
}

directory_obj *
__cache_search (const_nis_name name)
{
  directory_obj *dir;

  __libc_lock_lock (mgrlock);

  dir = internal_cache_search (name);

  __libc_lock_unlock (mgrlock);

  return dir;
}

nis_error
__cache_add (fd_result *fd)
{
  char clnt_res = 0;
  nis_error result = NIS_SUCCESS;

  __libc_lock_lock (mgrlock);

  if (cache_clnt == NULL)
    if (__cache_open () == FALSE)
      result = NIS_FAIL;

  if (cache_clnt != NULL &&
      (clnt_call (cache_clnt, NIS_CACHE_ADD_ENTRY, (xdrproc_t) xdr_fd_result,
		  (caddr_t)fd, (xdrproc_t) xdr_void, &clnt_res, TIMEOUT)
       != RPC_SUCCESS))
    {
      __cache_close ();
      result = NIS_RPCERROR;
    }

  __libc_lock_unlock (mgrlock);

  return result;
}
