/* Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2004.

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

#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <arpa/nameser.h>
#include <nsswitch.h>


#if PACKETSZ > 65536
# define MAXPACKET	PACKETSZ
#else
# define MAXPACKET	65536
#endif


/* We need this time later.  */
typedef union querybuf
{
  HEADER hdr;
  unsigned char buf[MAXPACKET];
} querybuf;


enum nss_status
_nss_dns_getcanonname_r (const char *name, char *buffer, size_t buflen,
			 char **result,int *errnop, int *h_errnop)
{
  /* Just an alibi buffer, res_nquery will allocate a real buffer for
     us.  */
  unsigned char buf[20];
  union
  {
    querybuf *buf;
    unsigned char *ptr;
  } ansp = { .ptr = buf };
  enum nss_status status = NSS_STATUS_UNAVAIL;
  int qtypes[] = { ns_t_a, ns_t_aaaa };
#define nqtypes (sizeof (qtypes) / sizeof (qtypes[0]))

  for (int i = 0; i < nqtypes; ++i)
    {
      int r = __libc_res_nquery (&_res, name, ns_c_in, qtypes[i],
				 buf, sizeof (buf), &ansp.ptr);
      if (r > 0)
	{
	  /* We need to decode the response.  Just one question record.
	     And if we got no answers we bail out, too.  */
	  if (ansp.buf->hdr.qdcount != htons (1))
	    continue;

	  /* Number of answers.   */
	  unsigned int ancount = ntohs (ansp.buf->hdr.ancount);

	  /* Beginning and end of the buffer with query, answer, and the
	     rest.  */
	  unsigned char *ptr = &ansp.buf->buf[sizeof (HEADER)];
	  unsigned char *endptr = ansp.ptr + r;

	  /* Skip over the query.  This is the name, type, and class.  */
	  int s = __dn_skipname (ptr, endptr);
	  if (s < 0)
	    {
	    unavail:
	      status = NSS_STATUS_UNAVAIL;
	      break;
	    }

	  /* Skip over the name and the two 16-bit values containing type
	     and class.  */
	  ptr += s + 2 * sizeof (uint16_t);

	  while (ancount-- > 0)
	    {
	      /* Now the reply.  First again the name from the query,
		 then type, class, TTL, and the length of the RDATA.
		 We remember the name start.  */
	      unsigned char *namestart = ptr;
	      s = __dn_skipname (ptr, endptr);
	      if (s < 0)
		goto unavail;

	      ptr += s;

	      /* Check whether type and class match.  */
	      unsigned int type = ntohs (*(uint16_t *) ptr);
	      if (type == qtypes[i])
		{
		  /* We found the record.  */
		  s = __dn_expand (ansp.buf->buf, endptr, namestart,
				   buffer, buflen);
		  if (s < 0)
		    {
		      if (errno != EMSGSIZE)
			goto unavail;

		      /* The buffer is too small.  */
		      *errnop = ERANGE;
		      status = NSS_STATUS_TRYAGAIN;
		      h_errno = NETDB_INTERNAL;
		    }
		  else
		    {
		      /* Success.  */
		      *result = buffer;
		      status = NSS_STATUS_SUCCESS;
		    }

		  goto out;
		}

	      if (type != ns_t_cname)
		goto unavail;

	      ptr += sizeof (uint16_t);
	      if (*(uint16_t *) ptr != htons (ns_c_in))
		goto unavail;

	      /* Also skip over the TTL.  */
	      ptr += sizeof (uint16_t) + sizeof (uint32_t);

	      /* Skip over the data length and data.  */
	      ptr += sizeof (uint16_t) + ntohs (*(uint16_t *) ptr);
	    }
	}
    }

 out:
  *h_errnop = h_errno;

  if (ansp.ptr != buf)
    free (ansp.ptr);

  return status;
}
