/* Copyright (C) 1998-2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <not-cancel.h>
#include <nis/rpcsvc/nis.h>

#include "nscd-client.h"


static int
open_socket (void)
{
  int sock = __socket (PF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    return -1;

  /* Make socket non-blocking.  */
  int fl = __fcntl (sock, F_GETFL);
  if (fl != -1)
    __fcntl (sock, F_SETFL, fl | O_NONBLOCK);

  struct sockaddr_un sun;
  sun.sun_family = AF_UNIX;
  strcpy (sun.sun_path, _PATH_NSCDSOCKET);
  if (__connect (sock, (struct sockaddr *) &sun, sizeof (sun)) < 0
      && errno != EINPROGRESS)
    goto out;

  struct pollfd fds[1];
  fds[0].fd = sock;
  fds[0].events = POLLOUT | POLLERR | POLLHUP;
  if (__poll (fds, 1, 5 * 1000) > 0)
    /* Success.  We do not check for success of the connect call here.
       If it failed, the following operations will fail.  */
    return sock;

 out:
  close_not_cancel_no_status (sock);

  return -1;
}


void
__nscd_unmap (struct mapped_database *mapped)
{
  assert (mapped->counter == 0);
  __munmap ((void *) mapped->head, mapped->mapsize);
  free (mapped);
}


/* Try to get a file descriptor for the shared meory segment
   containing the database.  */
static struct mapped_database *
get_mapping (request_type type, const char *key,
	     struct mapped_database **mappedp)
{
  struct mapped_database *result = NO_MAPPING;
#ifdef SCM_RIGHTS
  const size_t keylen = strlen (key) + 1;
  char resdata[keylen];
  int saved_errno = errno;

  int mapfd = -1;

  /* Send the request.  */
  struct iovec iov[2];
  request_header req;

  int sock = open_socket ();
  if (sock < 0)
    goto out;

  req.version = NSCD_VERSION;
  req.type = type;
  req.key_len = keylen;

  iov[0].iov_base = &req;
  iov[0].iov_len = sizeof (req);
  iov[1].iov_base = (void *) key;
  iov[1].iov_len = keylen;

  if (TEMP_FAILURE_RETRY (__writev (sock, iov, 2))
      != iov[0].iov_len + iov[1].iov_len)
    /* We cannot even write the request.  */
    goto out_close2;

  /* Room for the data sent along with the file descriptor.  We expect
     the key name back.  */
  iov[0].iov_base = resdata;
  iov[0].iov_len = keylen;

  char buf[CMSG_SPACE (sizeof (int))];
  struct msghdr msg = { .msg_iov = iov, .msg_iovlen = 1,
			.msg_control = buf, .msg_controllen = sizeof (buf) };
  struct cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);

  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN (sizeof (int));

  *(int *) CMSG_DATA (cmsg) = -1;

  msg.msg_controllen = cmsg->cmsg_len;

  struct pollfd fds[1];
  fds[0].fd = sock;
  fds[0].events = POLLIN | POLLERR | POLLHUP;
  if (__poll (fds, 1, 5 * 1000) <= 0)
    /* Failure or timeout.  */
    goto out_close2;

  if (TEMP_FAILURE_RETRY (__recvmsg (sock, &msg, 0)) != keylen)
    goto out_close2;

  mapfd = *(int *) CMSG_DATA (cmsg);

  if (CMSG_FIRSTHDR (&msg)->cmsg_len != CMSG_LEN (sizeof (int)))
    goto out_close;

  struct stat64 st;
  if (strcmp (resdata, key) != 0
      || fstat64 (mapfd, &st) != 0
      || st.st_size < sizeof (struct database_pers_head))
    goto out_close;

  struct database_pers_head head;
  if (TEMP_FAILURE_RETRY (__pread (mapfd, &head, sizeof (head), 0))
      != sizeof (head))
    goto out_close;

  if (head.version != DB_VERSION || head.header_size != sizeof (head)
      /* This really should not happen but who knows, maybe the update
	 thread got stuck.  */
      || head.timestamp + MAPPING_TIMEOUT < time (NULL))
    goto out_close;

  size_t size = (sizeof (head) + roundup (head.module * sizeof (ref_t), ALIGN)
		 + head.data_size);

  if (st.st_size < size)
    goto out_close;

  /* The file is large enough, map it now.  */
  void *mapping = __mmap (NULL, size, PROT_READ, MAP_SHARED, mapfd, 0);
  if (mapping != MAP_FAILED)
    {
      /* Allocate a record for the mapping.  */
      struct mapped_database *newp;

      newp = malloc (sizeof (*newp));
      if (newp == NULL)
	{
	  /* Ugh, after all we went through the memory allocation failed.  */
	  __munmap (result, size);
	  goto out_close;
	}

      newp->head = mapping;
      newp->data = ((char *) mapping + head.header_size
		    + roundup (head.module * sizeof (ref_t), ALIGN));
      newp->mapsize = size;
      /* Set counter to 1 to show it is usable.  */
      newp->counter = 1;

      result = newp;
    }

 out_close:
  __close (mapfd);
 out_close2:
  __close (sock);
 out:
  __set_errno (saved_errno);
#endif	/* SCM_RIGHTS */

  struct mapped_database *oldval = *mappedp;
  *mappedp = result;

  if (oldval != NULL && atomic_decrement_val (&oldval->counter) == 0)
    __nscd_unmap (oldval);

  return result;
}


struct mapped_database *
__nscd_get_map_ref (request_type type, const char *name,
		    struct locked_map_ptr *mapptr, volatile int *gc_cyclep)
{
  struct mapped_database *cur = mapptr->mapped;
  if (cur == NO_MAPPING)
    return cur;

  int cnt = 0;
  while (atomic_compare_and_exchange_val_acq (&mapptr->lock, 1, 0) != 0)
    {
      // XXX Best number of rounds?
      if (++cnt > 5)
	return NO_MAPPING;

      atomic_delay ();
    }

  cur = mapptr->mapped;

  if (__builtin_expect (cur != NO_MAPPING, 1))
    {
      /* If not mapped or timestamp not updated, request new map.  */
      if (cur == NULL
	  || (cur->head->nscd_certainly_running == 0
	      && cur->head->timestamp + MAPPING_TIMEOUT < time (NULL)))
	cur = get_mapping (type, name, &mapptr->mapped);

      if (__builtin_expect (cur != NO_MAPPING, 1))
	{
	  if (__builtin_expect (((*gc_cyclep = cur->head->gc_cycle) & 1) != 0,
				0))
	    cur = NO_MAPPING;
	  else
	    atomic_increment (&cur->counter);
	}
    }

  mapptr->lock = 0;

  return cur;
}


const struct datahead *
__nscd_cache_search (request_type type, const char *key, size_t keylen,
		     const struct mapped_database *mapped)
{
  unsigned long int hash = __nis_hash (key, keylen) % mapped->head->module;

  ref_t work = mapped->head->array[hash];
  while (work != ENDREF)
    {
      struct hashentry *here = (struct hashentry *) (mapped->data + work);

      if (type == here->type && keylen == here->len
	  && memcmp (key, mapped->data + here->key, keylen) == 0)
	{
	  /* We found the entry.  Increment the appropriate counter.  */
	  const struct datahead *dh
	    = (struct datahead *) (mapped->data + here->packet);

	  /* See whether we must ignore the entry or whether something
	     is wrong because garbage collection is in progress.  */
	  if (dh->usable && ((char *) dh + dh->allocsize
			     <= (char *) mapped->head + mapped->mapsize))
	    return dh;
	}

      work = here->next;
    }

  return NULL;
}


/* Create a socket connected to a name. */
int
__nscd_open_socket (const char *key, size_t keylen, request_type type,
		    void *response, size_t responselen)
{
  int saved_errno = errno;

  int sock = open_socket ();
  if (sock >= 0)
    {
      request_header req;
      req.version = NSCD_VERSION;
      req.type = type;
      req.key_len = keylen;

      struct iovec vec[2];
      vec[0].iov_base = &req;
      vec[0].iov_len = sizeof (request_header);
      vec[1].iov_base = (void *) key;
      vec[1].iov_len = keylen;

      ssize_t nbytes = TEMP_FAILURE_RETRY (__writev (sock, vec, 2));
      if (nbytes == (ssize_t) (sizeof (request_header) + keylen))
	{
	  /* Wait for data.  */
	  struct pollfd fds[1];
	  fds[0].fd = sock;
	  fds[0].events = POLLIN | POLLERR | POLLHUP;
	  if (__poll (fds, 1, 5 * 1000) > 0)
	    {
	      nbytes = TEMP_FAILURE_RETRY (__read (sock, response,
						   responselen));
	      if (nbytes == (ssize_t) responselen)
		return sock;
	    }
	}

      close_not_cancel_no_status (sock);
    }

  __set_errno (saved_errno);

  return -1;
}
