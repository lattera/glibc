/* Copyright (c) 1997 Free Software Foundation, Inc.
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
   Boston, MA 02111-1307, USA. */

#include <string.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>
#include "nis_intern.h"

nis_error
nis_servstate (const nis_server *serv, const nis_tag *tags,
	       const int numtags, nis_tag **result)
{
  nis_taglist taglist;
  nis_taglist tagres;

  tagres.tags.tags_len = 0;
  tagres.tags.tags_val = NULL;
  *result = NULL;
  taglist.tags.tags_len = numtags;
  taglist.tags.tags_val = (nis_tag *)tags;

  if (serv == NULL)
    {
      if (__do_niscall (NULL, NIS_SERVSTATE, (xdrproc_t) xdr_nis_taglist,
			(caddr_t) &taglist, (xdrproc_t) xdr_nis_taglist,
			(caddr_t) &tagres, 0) != RPC_SUCCESS)
	return NIS_RPCERROR;
    }
  else
    {
      if (__do_niscall2 (serv, 1, NIS_SERVSTATE, (xdrproc_t) xdr_nis_taglist,
			 (caddr_t) &taglist, (xdrproc_t) xdr_nis_taglist,
			 (caddr_t) &tagres, 0) != RPC_SUCCESS)
	return NIS_RPCERROR;
    }
  if (tagres.tags.tags_len > 0)
    {
      u_long i;

      result = malloc (sizeof (nis_tag *) * tagres.tags.tags_len);
      if (result == NULL)
	return NIS_NOMEMORY;
      for (i = 0; i < tagres.tags.tags_len; ++i)
	{
	  result[i] = malloc (sizeof (nis_tag));
	  if (result[i] == NULL)
	    return NIS_NOMEMORY;
	  result[i]->tag_val = strdup (tagres.tags.tags_val[i].tag_val);
	  result[i]->tag_type = tagres.tags.tags_val[i].tag_type;
	}
    }

  return NIS_SUCCESS;
}

nis_error
nis_stats (const nis_server *serv, const nis_tag *tags,
	   const int numtags, nis_tag **result)
{
  nis_taglist taglist;
  nis_taglist tagres;

  tagres.tags.tags_len = 0;
  tagres.tags.tags_val = NULL;
  *result = NULL;
  taglist.tags.tags_len = numtags;
  taglist.tags.tags_val = (nis_tag *)tags;

  if (serv == NULL)
    {
      if (__do_niscall (NULL, NIS_STATUS, (xdrproc_t) xdr_nis_taglist,
			(caddr_t) &taglist, (xdrproc_t) xdr_nis_taglist,
			(caddr_t) &tagres, 0) != RPC_SUCCESS)
	return NIS_RPCERROR;
    }
  else
    {
      if (__do_niscall2 (serv, 1, NIS_STATUS, (xdrproc_t) xdr_nis_taglist,
			 (caddr_t) &taglist, (xdrproc_t) xdr_nis_taglist,
			 (caddr_t) &tagres, 0) != RPC_SUCCESS)
	return NIS_RPCERROR;
    }
  if (tagres.tags.tags_len > 0)
    {
      u_long i;

      result = malloc (sizeof (nis_tag *) * tagres.tags.tags_len);
      if (result == NULL)
	return NIS_NOMEMORY;
      for (i = 0; i < tagres.tags.tags_len; ++i)
	{
	  result[i] = malloc (sizeof (nis_tag));
	  if (result[i] == NULL)
	    return NIS_NOMEMORY;
	  result[i]->tag_val = strdup (tagres.tags.tags_val[i].tag_val);
	  result[i]->tag_type = tagres.tags.tags_val[i].tag_type;
	}
    }

  return NIS_SUCCESS;
}

void
nis_freetags (nis_tag *tags, const int numtags)
{
  int i;

  for (i = 0; i < numtags; ++i)
    free (tags->tag_val);
  free (tags);
}
