/* Hosts file parser in nss_files module.
   Copyright (C) 1996-2001, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>


/* Get implementation for some internal functions.  */
#include "../resolv/mapv4v6addr.h"
#include "../resolv/res_hconf.h"


#define ENTNAME		hostent
#define DATABASE	"hosts"
#define NEED_H_ERRNO

#define EXTRA_ARGS	 , af, flags
#define EXTRA_ARGS_DECL	 , int af, int flags

#define ENTDATA hostent_data
struct hostent_data
  {
    unsigned char host_addr[16]; /* IPv4 or IPv6 address.  */
    char *h_addr_ptrs[2];	/* Points to that and null terminator.  */
  };

#define TRAILING_LIST_MEMBER		h_aliases
#define TRAILING_LIST_SEPARATOR_P	isspace
#include "files-parse.c"
LINE_PARSER
("#",
 {
   char *addr;

   STRING_FIELD (addr, isspace, 1);

   /* Parse address.  */
   if (inet_pton (af, addr, entdata->host_addr) <= 0
       && (af != AF_INET6 || (flags & AI_V4MAPPED) == 0
	   || inet_pton (AF_INET, addr, entdata->host_addr) <= 0
	   || (map_v4v6_address ((char *) entdata->host_addr,
				 (char *) entdata->host_addr),
	       0)))
     /* Illegal address: ignore line.  */
     return 0;

   /* We always return entries of the requested form.  */
   result->h_addrtype = af;
   result->h_length = af == AF_INET ? INADDRSZ : IN6ADDRSZ;

   /* Store a pointer to the address in the expected form.  */
   entdata->h_addr_ptrs[0] = entdata->host_addr;
   entdata->h_addr_ptrs[1] = NULL;
   result->h_addr_list = entdata->h_addr_ptrs;

   STRING_FIELD (result->h_name, isspace, 1);
 })



#define HOST_DB_LOOKUP(name, keysize, keypattern, break_if_match, proto...) \
enum nss_status								      \
_nss_files_get##name##_r (proto,					      \
			  struct STRUCTURE *result, char *buffer,	      \
			  size_t buflen, int *errnop H_ERRNO_PROTO)	      \
{									      \
  enum nss_status status;						      \
									      \
  __libc_lock_lock (lock);						      \
									      \
  /* Reset file pointer to beginning or open file.  */			      \
  status = internal_setent (keep_stream);				      \
									      \
  if (status == NSS_STATUS_SUCCESS)					      \
    {									      \
      /* Tell getent function that we have repositioned the file pointer.  */ \
      last_use = getby;							      \
									      \
      while ((status = internal_getent (result, buffer, buflen, errnop	      \
					H_ERRNO_ARG EXTRA_ARGS_VALUE))	      \
	     == NSS_STATUS_SUCCESS)					      \
	{ break_if_match }						      \
									      \
      if (status == NSS_STATUS_SUCCESS					      \
	  && _res_hconf.flags & HCONF_FLAG_MULTI)			      \
	{								      \
	  /* We have to get all host entries from the file.  */		      \
	  const size_t tmp_buflen = MIN (buflen, 4096);			      \
	  char tmp_buffer[tmp_buflen];					      \
	  struct hostent tmp_result_buf;				      \
	  int naddrs = 1;						      \
	  int naliases = 0;						      \
	  char *bufferend;						      \
									      \
	  while (result->h_aliases[naliases] != NULL)			      \
	    ++naliases;							      \
									      \
	  bufferend = (char *) &result->h_aliases[naliases + 1];	      \
									      \
	  while ((status = internal_getent (&tmp_result_buf, tmp_buffer,      \
					    tmp_buflen, errnop H_ERRNO_ARG    \
					    EXTRA_ARGS_VALUE))		      \
		 == NSS_STATUS_SUCCESS)					      \
	    {								      \
	      int matches = 1;						      \
	      struct hostent *old_result = result;			      \
	      result = &tmp_result_buf;					      \
	      /* The following piece is a bit clumsy but we want to use the   \
		 `break_if_match' value.  The optimizer should do its	      \
		 job.  */						      \
	      do							      \
		{							      \
		  break_if_match					      \
		  result = old_result;					      \
		}							      \
	      while ((matches = 0));					      \
									      \
	      if (matches)						      \
		{							      \
		  /* We could be very clever and try to recycle a few bytes   \
		     in the buffer instead of generating new arrays.  But     \
		     we are not doing this here since it's more work than     \
		     it's worth.  Simply let the user provide a bit bigger    \
		     buffer.  */					      \
		  char **new_h_addr_list;				      \
		  char **new_h_aliases;					      \
		  int newaliases = 0;					      \
		  size_t newstrlen = 0;					      \
		  int cnt;						      \
									      \
		  /* Count the new aliases and the length of the strings.  */ \
		  while (tmp_result_buf.h_aliases[newaliases] != NULL)	      \
		    {							      \
		      char *cp = tmp_result_buf.h_aliases[newaliases];	      \
		      ++newaliases;					      \
		      newstrlen += strlen (cp) + 1;			      \
		    }							      \
		  /* If the real name is different add it also to the	      \
		     aliases.  This means that there is a duplication	      \
		     in the alias list but this is really the users	      \
		     problem.  */					      \
		  if (strcmp (old_result->h_name,			      \
			      tmp_result_buf.h_name) != 0)		      \
		    {							      \
		      ++newaliases;					      \
		      newstrlen += strlen (tmp_result_buf.h_name) + 1;	      \
		    }							      \
									      \
		  /* Make sure bufferend is aligned.  */		      \
		  assert ((bufferend - (char *) 0) % sizeof (char *) == 0);   \
									      \
		  /* Now we can check whether the buffer is large enough.     \
		     16 is the maximal size of the IP address.  */	      \
		  if (bufferend + 16 + (naddrs + 2) * sizeof (char *)	      \
		      + roundup (newstrlen, sizeof (char *))		      \
		      + (naliases + newaliases + 1) * sizeof (char *)	      \
		      >= buffer + buflen)				      \
		    {							      \
		      *errnop = ERANGE;					      \
		      *herrnop = NETDB_INTERNAL;			      \
		      status = NSS_STATUS_TRYAGAIN;			      \
		      break;						      \
		    }							      \
									      \
		  new_h_addr_list =					      \
		    (char **) (bufferend				      \
			       + roundup (newstrlen, sizeof (char *))	      \
			       + 16);					      \
		  new_h_aliases =					      \
		    (char **) ((char *) new_h_addr_list			      \
			       + (naddrs + 2) * sizeof (char *));	      \
									      \
		  /* Copy the old data in the new arrays.  */		      \
		  for (cnt = 0; cnt < naddrs; ++cnt)			      \
		    new_h_addr_list[cnt] = old_result->h_addr_list[cnt];      \
									      \
		  for (cnt = 0; cnt < naliases; ++cnt)			      \
		    new_h_aliases[cnt] = old_result->h_aliases[cnt];	      \
									      \
		  /* Store the new strings.  */				      \
		  cnt = 0;						      \
		  while (tmp_result_buf.h_aliases[cnt] != NULL)		      \
		    {							      \
		      new_h_aliases[naliases++] = bufferend;		      \
		      bufferend = (__stpcpy (bufferend,			      \
					     tmp_result_buf.h_aliases[cnt])   \
				   + 1);				      \
		      ++cnt;						      \
		    }							      \
									      \
		  if (cnt < newaliases)					      \
		    {							      \
		      new_h_aliases[naliases++] = bufferend;		      \
		      bufferend = __stpcpy (bufferend,			      \
					    tmp_result_buf.h_name) + 1;	      \
		    }							      \
									      \
		  /* Final NULL pointer.  */				      \
		  new_h_aliases[naliases] = NULL;			      \
									      \
		  /* Round up the buffer end address.  */		      \
		  bufferend += (sizeof (char *)				      \
				- ((bufferend - (char *) 0)		      \
				   % sizeof (char *))) % sizeof (char *);     \
									      \
		  /* Now the new address.  */				      \
		  new_h_addr_list[naddrs++] =				      \
		    memcpy (bufferend, tmp_result_buf.h_addr,		      \
			    tmp_result_buf.h_length);			      \
									      \
		  /* Also here a final NULL pointer.  */		      \
		  new_h_addr_list[naddrs] = NULL;			      \
									      \
		  /* Store the new array pointers.  */			      \
		  old_result->h_aliases = new_h_aliases;		      \
		  old_result->h_addr_list = new_h_addr_list;		      \
									      \
		  /* Compute the new buffer end.  */			      \
		  bufferend = (char *) &new_h_aliases[naliases + 1];	      \
		  assert (bufferend <= buffer + buflen);		      \
									      \
		  result = old_result;					      \
		}							      \
	    }								      \
									      \
	  if (status != NSS_STATUS_TRYAGAIN)				      \
	    status = NSS_STATUS_SUCCESS;				      \
	}								      \
									      \
									      \
      if (! keep_stream)						      \
	internal_endent ();						      \
    }									      \
									      \
  __libc_lock_unlock (lock);						      \
									      \
  return status;							      \
}


#define EXTRA_ARGS_VALUE \
  , ((_res.options & RES_USE_INET6) ? AF_INET6 : AF_INET),		      \
  ((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0)
#include "files-XXX.c"
HOST_DB_LOOKUP (hostbyname, ,,
		{
		  LOOKUP_NAME_CASE (h_name, h_aliases)
		}, const char *name)


#undef EXTRA_ARGS_VALUE
/* XXX Is using _res to determine whether we want to convert IPv4 addresses
   to IPv6 addresses really the right thing to do?  */
#define EXTRA_ARGS_VALUE \
  , af, ((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0)
HOST_DB_LOOKUP (hostbyname2, ,,
		{
		  LOOKUP_NAME_CASE (h_name, h_aliases)
		}, const char *name, int af)

#undef EXTRA_ARGS_VALUE
/* We only need to consider IPv4 mapped addresses if the input to the
   gethostbyaddr() function is an IPv6 address.  */
#define EXTRA_ARGS_VALUE \
  , af, (len == IN6ADDRSZ ? AI_V4MAPPED : 0)
DB_LOOKUP (hostbyaddr, ,,
	   {
	     if (result->h_length == (int) len
		 && ! memcmp (addr, result->h_addr_list[0], len))
	       break;
	   }, const void *addr, socklen_t len, int af)
